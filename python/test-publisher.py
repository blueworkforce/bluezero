import pyb0
from time import sleep
node = pyb0.Node('python-publisher')
pub = pyb0.Publisher(node, 'A')
node.init()
for i in range(1000000):
    msg = 'msg-%d' % i
    print('Sending message "%s"...' % msg)
    pub.publish(msg)
    sleep(1)
node.cleanup()
