import b0

node = b0.Node('python-service-client')
cli = b0.ServiceClient(node, 'control')
node.init()
print('Using service "%s"...' % cli.get_service_name())
req = 'hello'
print('Sending "%s"...' % req)
rep = cli.call(req.encode('utf-8'))
print('Received "%s"' % rep)
node.cleanup()
