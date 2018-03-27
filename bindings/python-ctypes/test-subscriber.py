import b0

def callback(msg):
    print('Received message "%s"' % msg)
node = b0.Node('python-subscriber')
sub = b0.Subscriber(node, 'A', callback)
node.init()
print('Subscribed to topic "%s"...' % sub.get_topic_name())
node.spin()
node.cleanup()
