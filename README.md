# cnetlib

`cnetlib` is a lightweight C network layer for creating TCP client/server applications.
It provides socket connection helpers, remote/local IP and port inspection, and a small server event API.

## Included packages

- `include/` — public headers for client/server APIs (`csnl.h`, `ssnl.h`)
- `lib/` — library binaries for client/server (`libcsnl`, `libssnl`)
- `template/` — copy-paste starter client/server templates for quick application building
- `tutorial/` — example applications that demonstrate how to use the APIs
- `doc/` — Doxygen configuration and documentation source files

## Features

- TCP client connect and request/response handling
- TCP server start/stop and client connection callbacks
- Remote/local IP and port lookup helpers
- Error reporting helpers with message strings
- Example apps for basic client/server usage and shutdown handling

## Quick start

Build the client and server using the provided headers and libraries.

### Client

```sh
gcc tutorial/app-client.c \
    -Iinclude -Llib -lcsnl \
    -o app-client
```

### Server

```sh
gcc tutorial/app-server.c \
    -Iinclude -Llib -lssnl -lpthread \
    -o app-server
```

> Adjust include/library paths as needed for your local layout.

## Running the examples

1. Start the server:

```sh
./app-server 5046
```

2. Run the client:

```sh
./app-client localhost 5046
```

3. The client sends a hello message, the server replies, then the client sends a stop request.

## Documentation

The `doc/` folder contains Doxygen sources and configuration.
Run `doxygen doc/Doxyfile` to generate HTML documentation.

The generated docs include:
- Main project overview
- Client API reference
- Server API reference
- Examples/tutorials
- Templates

## Notes

- The tutorial examples are the best place to learn `cnetlib` usage.
- `tcp_connection_get_*_ip()` / `tcp_client_get_*_ip()` and `*_error()` return malloc'd strings that must be freed.

## License

This project is released under the MIT License — see the [LICENSE](LICENSE) file included in the repository.

Copyright (c) 2026 Tanmay Shah