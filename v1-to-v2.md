# Porting to BlueZero v2

## protobuf

Built-in protobuf support has been dropped. Just use protobuf's `SerializeToString(string*)` method to serialize to a `std::string` payload, or use the code in https://github.com/blueworkforce/bluezero-protobuf to bring back protobuf support in your project (replace `b0::Publisher` with `b0::protobuf::Publisher`, and similarly for the other socket classes).

## b0::init

Now it is mandatory to call `b0::init(int, char**)` prior to node constructions, in order to process command line arguments.

## Other API changes

`b0::Node::spin()` now accepts a callback method as first argument.
