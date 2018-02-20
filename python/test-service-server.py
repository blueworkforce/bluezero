import pyb0
def callback(req):
    print('Received request "%s"' % req)
    rep = 'hi'
    print('Sending reply "%s"...' % rep)
    return rep
node = pyb0.Node('python-service-server')
srv = pyb0.ServiceServer(node, 'control', callback)
node.init()
node.spin()
node.cleanup()
