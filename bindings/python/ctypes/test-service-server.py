import b0

def callback(req):
    print('Received request "%s"...' % req)
    rep = 'hi'
    print('Sending reply "%s"...' % rep)
    return rep.encode('utf-8')
node = b0.Node('python-service-server')
srv = b0.ServiceServer(node, 'control', callback)
node.init()
print('Offering service "%s"...' % srv.get_service_name())
node.spin()
node.cleanup()
