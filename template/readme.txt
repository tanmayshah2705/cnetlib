# cnetlib(Network Layer) Client-Server Template

## Overview

This template uses only the Network Layer APIs:

* Client side: `csnl.h` and `libcsnl`
* Server side: `ssnl.h` and `libssnl`

The template demonstrates:

* starting a TCP server
* connecting a TCP client
* fetching remote IP/port details on both sides
* sending a hello message from the client
* sending a hello message from the server
* checking whether the client is still connected
* sending a stop-server request from the client
* stopping the server, releasing client/server resources, and exiting the server app

## Structure

template/
  client/
    client_template.c
  server/
    server_template.c
  readme.txt

## Compile

Adjust the include and library paths according to where you copy this template.

Client:

gcc client/client_template.c -I path_to_header_file_folder -L path_to_library_file_folder -lcsnl -o client-app

Server:

gcc server/server_template.c -I path_to_header_file_folder -L path_to_library_file_folder -lssnl -lpthread -o server-app

## Run

Start the server:

./server-app 5046

Run the client. It sends a hello message, receives a server hello message, and then sends a stop-server request:

./client-app localhost 5046
