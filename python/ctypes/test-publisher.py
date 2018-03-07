import b0
import time

node = b0.Node('python-publisher')
pub = b0.Publisher(node, 'A')
node.init()
i = 0
while not node.shutdown_requested():
    msg = 'msg-%d' % i
    i += 1
    print('Sending message "%s"...' % msg)
    pub.publish(msg)
    node.spin_once()
    time.sleep(1)
node.cleanup()
