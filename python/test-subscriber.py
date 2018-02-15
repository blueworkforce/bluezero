import pyb0
def callback(payload):
    print('received payload: %s' % payload)
node = pyb0.Node('python-subscriber')
sub = pyb0.Subscriber(node, 'A', callback)
node.init()
node.spin()
node.cleanup()
