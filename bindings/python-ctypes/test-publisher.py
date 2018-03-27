import b0
import time

node = b0.Node('python-publisher')
pub = b0.Publisher(node, 'A')
node.init()
print('Publishing to topic "%s"...' % pub.get_topic_name())
i = 0
while not node.shutdown_requested():
    msg = 'msg-%d' % i
    i += 1
    print('Sending message "%s"...' % msg)
    pub.publish(msg.encode('utf-8'))
    node.spin_once()
    time.sleep(1)
node.cleanup()
