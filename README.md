# Ping-Pong TCP

Simple C++ client-server application. Cient sends a "PING", server responds with a "PONG".

## Build

```bash
make
```

### Manual

```bash
g++ -std=c++20 -Wall -Wextra -o server server.cpp
g++ -std=c++20 -Wall -Wextra -o client client.cpp
```

## Run

```bash
make run-server
make run-client
```

### Manual

```bash
./server
./client
```

## Test

```bash
make test
```
