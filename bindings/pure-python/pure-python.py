import json
import time
import zmq

class MessagePart:
    def __init__(self):
        self.content_type = ''
        self.compression_algorithm = ''
        self.compression_level = 0
        self.payload = ''

class MessageEnvelope:
    def __init__(self):
        self.parts = []
        self.headers = {}
    def parse(self, s):
        headers_txt, payload = s.split('\n\n', 1)
        for line in headers_txt.splitlines():
            key, value = line.split(': ', 1)
            self.headers[key] = value
        part_count = int(self.headers['Part-count'])
        last_start = 0
        for i in range(part_count):
            part = MessagePart()
            k = 'Content-length-%d' % i
            part.content_length = int(self.headers[k])
            del self.headers[k]
            k = 'Content-type-%d' % i
            if k in self.headers:
                part.content_type = self.headers[k]
                del self.headers[k]
            k = 'Compression-algorithm-%d' % i
            if k in self.headers:
                raise RuntimeError('compression not supported')
                part.compression_algorithm = self.headers.get('Compression-algorithm-%d' % i, '')
                del self.headers[k]
            part.payload = payload[last_start:last_start+part.content_length]
            last_start += part.content_length
            self.parts.append(part)
    def serialize(self):
        s = ''
        if 'Header' in self.headers:
            s += 'Header: %s\n' % self.headers["Header"]
        s += 'Part-count: %d\n' % len(self.parts)
        content_length = 0
        for i, part in enumerate(self.parts):
            s += 'Content-length-%d: %d\n' % (i, len(part.payload))
            if part.content_type:
                s += 'Content-type-%d: %s\n' % (i, part.content_type)
            if part.compression_algorithm:
                raise RuntimeError('compression not supported')
            content_length += len(part.payload)
        s += 'Content-length: %d\n' % content_length
        for key, value in self.headers.items():
            s += '%s: %s\n' % (key, value)
        s += '\n'
        for part in self.parts:
            s += part.payload
        return s

class Socket:
    def __init__(self, node, sock_type, name, managed=True, notify_graph=False):
        self.node = node
        self.name = name
        self.notify_graph = notify_graph
        self.has_header = False
        self.remote_addr = ''
        if managed: self.node.add_socket(self)
        ctx = zmq.Context.instance()
        self.socket = ctx.socket(sock_type)
    def init(self):
        pass
    def spin_once(self):
        pass
    def cleanup():
        pass
    def poll(self):
        poller = zmq.Poller()
        poller.register(self.socket, zmq.POLLIN)
        socks = dict(poller.poll())
        return socks[self.socket] == zmq.POLLIN
    def read_envelope(self):
        data = self.socket.recv_string()
        env = MessageEnvelope()
        env.parse(data)
        return env
    def read_msg(self):
        env = self.read_envelope()
        if not env.parts:
            raise RuntimeError('received message with no parts')
        try:
            msg = env.parts[0].payload
            msg = json.loads(msg)
        except: pass
        return msg, env.parts[0].content_type
    def write_envelope(self, env):
        if self.has_header: env.headers['Header'] = self.name
        data = env.serialize()
        return self.socket.send_string(data)
    def write_msg(self, msg, msgtype):
        env = MessageEnvelope()
        env.parts = [MessagePart()]
        env.parts[0].payload = json.dumps(msg) if isinstance(msg, dict) else msg
        env.parts[0].content_type = msgtype
        self.write_envelope(env)
    def notify_topic(self, reverse, active):
        self._send_notify('topic', reverse, active)
    def notify_service(self, reverse, active):
        self._send_notify('service', reverse, active)
    def _send_notify(self, what, reverse, active):
        self.node.resolv_cli.call({
            'node_name': self.node.name,
            '%s_name' % what: self.name,
            'reverse': reverse,
            'active': active
        }, {'topic': 'NodeTopicRequest', 'service': 'NodeServiceRequest'}[what])

class Publisher(Socket):
    def __init__(self, node, name, managed=True, notify_graph=True):
        super().__init__(node, zmq.PUB, name, managed, notify_graph)
        self.has_header = True
    def init(self):
        if not self.remote_addr:
            self.remote_addr = self.node.xsub_addr
        self.socket.connect(self.remote_addr)
        if self.notify_graph: self.notify_topic(False, True)
    def publish(self, msg, msgtype):
        self.write_msg(msg, msgtype)
    def cleanup(self):
        if self.notify_graph: self.notify_topic(False, False)
        self.socket.disconnect()

class Subscriber(Socket):
    def __init__(self, node, name, callback, managed=True, notify_graph=True):
        super().__init__(node, zmq.SUB, name, managed, notify_graph)
        if not callable(callback):
            raise TypeError('callback is not callable')
        self.callback = callback
    def init(self):
        if not self.remote_addr:
            self.remote_addr = self.node.xpub_addr
        self.socket.connect(self.remote_addr)
        self.socket.setsockopt_string(zmq.SUBSCRIBE, 'Header: %s' % self.name)
        if self.notify_graph: self.notify_topic(True, True)
    def spin_once(self):
        while self.poll():
            msg, msgtype = self.read_msg()
            self.callback(msg, msgtype)
    def cleanup(self):
        if self.notify_graph: self.notify_topic(True, False)
        self.socket.disconnect()

class ServiceClient(Socket):
    def __init__(self, node, name, managed=True, notify_graph=True):
        super().__init__(node, zmq.REQ, name, managed, notify_graph)
    def init(self):
        if not self.remote_addr:
            self.remote_addr = self.resolve_service(self.name)
        self.socket.connect(self.remote_addr)
        if self.notify_graph: self.notify_service(True, True)
    def resolve_service(self, name):
        rep, reptype = self.node.resolv_cli.call({
            'service_name': name
        }, 'ResolveServiceRequest')
        if not rep['ok']: raise RuntimeError('failed resolve service')
        return rep['sock_addr']
    def call(self, req, reqtype):
        self.write_msg(req, reqtype)
        rep, reptype = self.read_msg()
        return rep, reptype
    def cleanup(self):
        if self.notify_graph: self.notify_service(True, False)

class ServiceServer(Socket):
    def __init__(self, node, name, callback, managed=True, notify_graph=True):
        super().__init__(node, zmq.REP, name, managed, notify_graph)
        if not callable(callback):
            raise TypeError('callback is not callable')
        self.callback = callback
    def init(self):
        import socket, os
        hostname = os.environ.get('B0_HOST_ID', 'localhost')
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 0))
        free_port = s.getsockname()[1]
        s.close()
        self.socket.bind('tcp://*:%d' % free_port)
        self.announce_service('tcp://%s:%d' % (hostname, free_port))
        if self.notify_graph: self.notify_service(False, True)
    def announce_service(self, addr):
        rep, reptype = self.node.resolv_cli.call({
            'node_name': self.node.name,
            'service_name': self.name,
            'sock_addr': addr
        }, 'AnnounceServiceRequest')
        if not rep['ok']: raise TypeError('announce service failed')
    def spin_once(self):
        while self.poll():
            req, reqtype = self.read_msg()
            rep, reptype = self.callback(req, reqtype)
            self.write_msg(rep, reptype)
    def cleanup(self):
        if self.notify_graph: self.notify_service(False, False)

class Node:
    def __init__(self, name):
        self.name = name
        self.sockets = []
        self.shutdown_flag = False
        self.state = 'created'
        self.resolv_cli = ServiceClient(self, 'resolv', False, False)
    def add_socket(self, socket):
        if self.state != 'created':
            raise RuntimeError('cannot add socket to an already initialized node')
        self.sockets.append(socket)
    def init(self):
        if self.state != 'created': raise RuntimeError('invalid state')
        self.resolv_cli.remote_addr = 'tcp://127.0.0.1:22000'
        self.resolv_cli.init() # this socket is not managed
        self.announce_node()
        for socket in self.sockets:
            socket.init()
        self.state = 'ready'
    def announce_node(self):
        rep, reptype = self.resolv_cli.call({
            'node_name': self.name
        }, 'AnnounceNodeRequest')
        if not rep['ok']: raise RuntimeError('announce node failed')
        self.xpub_addr = rep['xpub_sock_addr']
        self.xsub_addr = rep['xsub_sock_addr']
        self.name = rep['node_name']
    def send_heartbeat(self):
        rep, reptype = self.resolv_cli.call({
            'node_name': self.name
        })
    def spin_once(self):
        if self.state != 'ready': raise RuntimeError('invalid state')
        for socket in self.sockets:
            socket.spin_once()
    def spin(self, rate = 10):
        while not self.shutdown_flag:
            self.spin_once()
            self.send_heartbeat()
            time.sleep(1. / rate)
    def cleanup(self):
        if self.state != 'ready': raise RuntimeError('invalid state')
        self.state = 'terminated'
        self.notify_shutdown()
        for socket in self.sockets:
            socket.cleanup()
    def notify_shutdown(self):
        rep, reptype = self.resolv_cli.call({
            'node_name': self.name
        }, 'ShutdownRequest')

def run_pub():
    node = Node('python-publisher-node')
    pub = Publisher(node, 'A')
    node.init()
    while 1:
        pub.publish({'a': 1, 'b': 2}, 'foo')
        time.sleep(0.1)
    node.cleanup()

def run_sub():
    def cb(msg, msgtype):
        print(msg, msgtype)
    node = Node('python-subscriber-node')
    sub = Subscriber(node, 'A', cb)
    node.init()
    node.spin()
    node.cleanup()

def run_cli():
    node = Node('python-service-client')
    cli = ServiceClient(node, 'control')
    node.init()
    print(cli.call({'msg': 'hello', 'number': 23}, 'foo'))
    node.cleanup()

def run_srv():
    def cb(msg, msgtype):
        return {'answer': 42}, 'TheAnswer'
    node = Node('python-service-server')
    srv = ServiceServer(node, 'control', cb)
    node.init()
    node.spin()
    node.cleanup()

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print('usage: %s <task_name>' % sys.argv[0])
        sys.exit(1)
    globals()['run_%s' % sys.argv[1]](*sys.argv[2:])

