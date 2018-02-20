import pyb0
def callback(msg):
    print('Received message: "%s"' % msg)
node = pyb0.Node('python-subscriber')
sub = pyb0.Subscriber(node, 'A', callback)
node.init()
node.spin()
node.cleanup()
