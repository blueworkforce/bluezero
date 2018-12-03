# Porting to BlueZero v2

## protobuf

Built-in protobuf support has been dropped. Just use protobuf's `SerializeToString(string*)` method to serialize to a `std::string` payload, or use the code in https://github.com/blueworkforce/bluezero-protobuf to bring back protobuf support in your project (replace `b0::Publisher` with `b0::protobuf::Publisher`, and similarly for the other socket classes).

## b0::init

Now it is mandatory to call `b0::init(int, char**)` prior to node constructions, in order to process command line arguments.

## error: ambiguous call to XXX

Where XXX can be:

 - b0::Publisher::Publisher(...)
 - b0::Subscriber::Subscriber(...)
 - b0::ServiceClient::ServiceClient(...)
 - b0::ServiceServer::ServiceServer(...)

Solution: cast the `callback` argument to the correct type, choosing one from:

 - CallbackRaw
 - CallbackRawType
 - CallbackParts
 - CallbackMsg<T>
 - CallbackMsgParts<T>

Example: `static_cast<b0::Subscriber::CallbackRaw>(callback)`.

## Other API changes

`b0::Node::spin()` now accepts a callback method as first argument.
