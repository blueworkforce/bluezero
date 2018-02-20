import pyb0
node = pyb0.Node('python-service-client')
cli = pyb0.ServiceClient(node, 'control')
node.init()
req = 'hello'
print('Sending "%s"...' % req)
rep = cli.call(req)
print('Received "%s"' % rep)
node.cleanup()
