# Changelog

## v1.4.6 (2018-09-13)

 - Added `-o` option to resolver to disable heartbeat monitoring.

## v1.4.5 (2018-08-21)

 - Fix some compile errors in previous release.

## v1.4.4 (2018-08-21)

 - Fixed an uncaught exception happening during resolver quit (in `HBsweeper` thread).
 - Backported some tests from the `devel` branch.

## v1.4.3 (2018-08-03)

 - Added more APIs to the C interface (`Node.sleep_usec` and `Socket.set_option`).
 - Fix some build errors due to careless refactoring.
 - Drop `inproc://` sockets, so to allow the use of multiple resolvers in the same process.
 - Allow to specify resolver address via the C++ API.
 - Allow to specify resolver port via the C++ API.

## v1.4.2 (2018-07-25)

 - Minor changes in docs and code.

## v1.4.1 (2018-07-16)

 - Added more bindings.
 - Fixed several bugs with python bindings.
 - Added LocalLogger output level control via `B0_CONSOLE_LOGLEVEL` env varable.
 - Added protocol documentation.
 - Fix a division by zero occurring in rare conditions.

## v1.4.0 (2018-05-02)

 - Add API for changing some low level ZMQ socket options.
 - Add UserData field to nodes and sockets.
 - Added C API.
 - Dropped protobuf from tests, examples and public API.
 - Added `ctype` Python bindings.
 - [!] For using protobuf, need to include the corresponding header `<b0/protobuf/...>`.
 - [!] Node's `State` refactored to `NodeState` class.
 - Fixed a bug where disconnected nodes are not detected, do not disappear from the graph, and prevent new node with the same name to initialize.

## v1.3.0 (2018-01-11)

 - [!] Dropped `topic` argument from `Publisher/Subscriber` `read`/`write` methods.
 - [!] Dropped `topic` argument from `Publisher/Subscriber` callbacks.
 - [!] Executables prefixed with `b0_*`.
 - [!] Removed log() method at default level. Now log level must be always specified.
 - Variables prefixes renamed to `B0_*` (old names still working but deprecated).
 - Some fixes for building on Windows with Visual Studio 2015.
 - Introduced specific exception classes.
 - Introduced `Socket` class as a base class for `Publisher`, `Subscriber`, `ServiceClient` and `ServiceServer`.
 - Dropped `zhelpers.hpp` library.
 - Dropped `protobufhelpers.hpp` library.
 - Message type is now sent in the message envelope.
 - Removed references to ZeroMQ from public headers.
 - Added some GUI tools (log console and graph console).
 - Allow to set a timeout for the announce node phase (`setAnnounceTimeout()`).

## v1.2.0 (2017-11-07)

 - Added payload compression (multiple compression algorithms supported).
 - Refactored headers.

## v1.1.0

 - Added time synchronization.

## v1.0.0

 - First release: common sockets (topics, services).
