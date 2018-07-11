# BlueZero git repository

BlueZero (in short, "B0") is a cross-platform middleware which provides tools for interconnecting pieces of software running in multiple threads, processes or machines.

It has some similarities with ROS, although it only focuses on providing communication paradigms (client/server and publisher/subscriber) and message transport (based on ZeroMQ), while being agnostic to message serialization format or common protocols and data structures.

## Documentation

See the generated [`doc/html/index.html`](https://blueworkforce.github.io/bluezero/) in the `build` dir for the documentation.

## License

See [LICENSE.md](LICENSE.md).

## Dependencies

### Required Libraries

 - ZeroMQ v4.1+
 - Protocol Buffers v2.6+
 - Boost v1.54+

### Optional Libraries
 - zlib (for compressing payloads)
 - lz4 (for compressing payloads)
 - Doxygen (for generating documentation)
 - Graphviz (for generating documentation)
 - mscgen (for generating documentation)

### Ubuntu 16

Install the following apt packages:

 - libprotobuf-dev
 - protobuf-compiler
 - libzmq-dev
 - libboost-all-dev

### Ubuntu 14

Install the following apt packages:

 - libboost-all-dev

The apt packages for Protobuf and ZeroMQ from apt repository are too old. Compile and install them from git repositories:

 - https://github.com/google/protobuf
 - https://github.com/zeromq/libzmq

### macOS (homebrew)

Install the following:

 - zeromq
 - protobuf
 - boost
 - lz4

## Compiling

```bash
git clone https://github.com/BlueWorkforce/bluezero
cd bluezero
mkdir build
cd build
cmake ..
make
```

Optionally, generate the documentation (requires [Doxygen](http://www.doxygen.org) and [Graphviz](http://www.graphviz.org) installed):
```
make doc
```

Optionally, run unit tests:
```
make test
```

