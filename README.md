BlueZero git repository
=======================

Middleware for the BlueWorkforce projects

See the generated [`doc/html/index.html`](https://blueworkforce.github.io/bluezero/) in the `build` dir for the documentation.

Required Libraries
==================

 - ZeroMQ v4.1+ (ubuntu package libzmq3-dev)
 - Protocol Buffers v2.6+
 - Boost v1.54+

Ubuntu 16
---------

Install the following apt packages:

 - libprotobuf-dev
 - protobuf-compiler
 - libzmq-dev
 - libboost-all-dev

Ubuntu 14
---------

Install the following apt packages:

 - libboost-all-dev

The apt packages for Protobuf and ZeroMQ from apt repository are too old. Compile and install them from git repositories:

 - https://github.com/google/protobuf
 - https://github.com/zeromq/libzmq
 - https://github.com/zeromq/cppzmq

macOS (homebrew)
----------------

Install the following:

 - zeromq
 - protobuf
 - boost

Compiling
=========

```bash
git clone https://github.com/BlueWorkforce/bluezero
cd bluezero
mkdir build
cd build
cmake ..
make
make doc
```

