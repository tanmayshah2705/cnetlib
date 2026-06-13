# cnetlib (Network Layer Library)

A lightweight TCP networking library for C to create client-server applications using POSIX sockets.

It simplify common networking tasks such as establishing connections, exchanging data, managing clients, and handling connection-related errors.

The library is split into two components:

* **CSNL** (Client Side Network Layer)
* **SSNL** (Server Side Network Layer)



---

## Documentation

Complete API documentation is available online:

**https://tanmayshah2705.github.io/cnetlib/**

The documentation is generated using Doxygen and includes:

- API reference
- Function documentation
- Data structures
- Usage information
- Source code navigation

Generate documentation using:

```bash
doxygen doc/Doxyfile
```

Documentation sources are located in:

```text
doc/
```

---

## Features

### Client Side Network Layer (CSNL)

* Connect to TCP servers
* Send and receive data
* Retrieve local and remote endpoint information
* Detect connection failures
* Access error information
* Disconnect and release resources

### Server Side Network Layer (SSNL)

* Create and manage TCP servers
* Start and stop listening services
* Handle incoming client connections through callbacks
* Send and receive data from connected clients
* Retrieve endpoint information
* Detect failures and access error information
* Release allocated resources

---

## Project Structure

```text
cnetlib/
├── include/
│   ├── csnl.h
│   └── ssnl.h
│
├── src/
│   ├── client/
│   │   └── csnl.c
│   └── server/
│       └── ssnl.c
│
├── tutorial/
│   ├── app-client.c
│   ├── app-server.c
│   └── shutdown-server.c
│
├── doc/
│   ├── Doxyfile
│   ├── DoxygenLayout.xml
│   └── main.md
│
├── Makefile
├── deploy_docs.sh
├── LICENSE
└── README.md
```

---

## Requirements

* Linux
* GCC
* GNU Make
* POSIX sockets
* pthread library

---

## Building

Build the library:

```bash
make
```

Generated libraries:

```text
lib/libcsnl.a
lib/libssnl.a
```

Clean build artifacts:

```bash
make clean
```

---

## Installation

Build the project:

```bash
make
```

Include the appropriate header and link against the required static library.

### Client Applications

```c
#include <csnl.h>
```

```bash
gcc client.c  -Iinclude  -Llib  -lcsnl  -o client
```

### Server Applications

```c
#include <ssnl.h>
```

```bash
gcc server.c  -Iinclude  -Llib  -lssnl  -lpthread  -o server
```

---

## Quick Example

### Client

```c
tcp_connection *conn = tcp_connect("localhost", 5406);

if (!tcp_connection_failed(conn))
{
    tcp_connection_send(
        conn,
        "Hello",
        5
    );
}

disconnect_tcp_connection(conn);
release_tcp_connection(conn);
```

### Server

```c
tcp_server *server = allocate_tcp_server(5406);

tcp_start_server(server);

/* Server runs until stopped */

release_tcp_server(server);
```

---

## Examples

The `tutorial/` directory contains example programs demonstrating library usage.

| File                | Description             |
| ------------------- | ----------------------- |
| `app-client.c`      | Example TCP client      |
| `app-server.c`      | Example TCP server      |
| `shutdown-server.c` | Example shutdown client |


---

## API Overview

### Client Library

Main functionality includes:

* Establish TCP connection
* Send and receive data
* Retrieve local and remote endpoint information
* Detect connection failures
* Report connection errors

Header:

```text
include/csnl.h
```

### Server Library

Main functionality includes:

* TCP server creation and management
* Client connection handling through callbacks
* Send and receive data from connected clients
* Retrieve Local and remote endpoint information
* Detect connection failures
* Report connection errors
  

Header:

```text
include/ssnl.h
```

---

## Error Handling

Both the client and server libraries provide functions for detecting failures and retrieving error messages.

Applications should check the result of operations and handle errors appropriately.

---

## License

This project is licensed under the MIT License.

See the [LICENSE](LICENSE) file for details.
