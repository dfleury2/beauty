<div align="center">
  <img alt="A Rose" src="https://github.com/dfleury2/beauty/raw/master/docs/rose.png" height="180" />
  <br>
  A simple Http server and client above <a href="https://github.com/boostorg/beast">Boost.Beast</a>
</div>
<br>

Beauty is a layer above <a href="https://github.com/boostorg/beast">Boost.Beast</a> which provide facilities to create Http server or client. Beauty allows the creation of synchronous or asynchronous server and client, and adds some signals and timer management based on <a href="https://github.com/boostorg/asio">Boost.Asio</a>

## Features
- Http or Http/s server or client side
- Websocket (no TLS yet) for server and client (still experimental)
- Synchronous or Asynchronous API
- Timeout support
- Postponed response from server support
- Easy routing server with placeholders
- Timers and signals support included
- Startable and stoppable application event loop
- Customizable thread pool size
- Work-in-progress: Swagger description API

## Examples

- a server

A more complete example is available in examples/server.cpp

```cpp
#include <beauty/beauty.hpp>

int main()
{
    // Create a server
    beauty::server server;

    // Add a default '/' route
    server.add_route("/")
        .get([](const auto& req, auto& res) {
            res.body() = "It's work ;) ... it works! :)";
        });

    // Add a '/person/:id' route
    server.add_route("/person/:id")
        .get([](const auto& req, auto& res) {
            auto id = req.a("id").as_string();
            res.body() = "You asked for the person id: " + id;
        });

    // Open the listening port
    server.listen(8085);
        // Listen will automatically start the loop in a separate thread

    // Wait for the server to stop
    server.wait();
}

```

- a synchronous client

```cpp
#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
    // Create a client
    beauty::client client;

    // Request an URL
    auto[ec, response] = client.get("http://127.0.0.1:8085");

    // Check the result
    if (!ec) {
        if (response.is_status_ok()) {
            // Display the body received
            std::cout << response.body() << std::endl;
        } else {
            std::cout << response.status() << std::endl;
        }
    } else {
        // An error occurred
        std::cout << ec << ": " << ec.message() << std::endl;
    }
}
```
- an asynchronous client

```cpp
#include <beauty/beauty.hpp>

#include <iostream>
#include <chrono>

int main()
{
    // Create a client
    beauty::client client;

    // Request an URL
    client.get("http://127.0.0.1:8085",
               [](auto ec, auto&& response) {
                   // Check the result
                   if (!ec) {
                       if (response.is_status_ok()) {
                           // Display the body received
                           std::cout << response.body() << std::endl;
                       } else {
                           std::cout << response.status() << std::endl;
                       }
                   } else {
                       // An error occurred
                       std::cout << ec << ": " << ec.message() << std::endl;
                   }
               });

    // Need to wait a little bit to received the response
    for (int i = 0; i < 10; ++i) {
        std::cout << '.'; std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
}

```

- timers

```cpp
#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
    // Launch a repeatable timer each 250ms
    int timer_count = 4;
    beauty::repeat(0.250, [&timer_count]() {
            std::cout << "Tick..." << std::endl;
            if (--timer_count == 0) {
                std::cout << "Dring !" << std::endl;
                beauty::stop();
            }
        });

    // Launch a one shot timer after 600ms
    beauty::after(0.600, [] {
            std::cout << "Snooze !" << std::endl;
    });

    // Wait for the end
    beauty::wait();
}
```

- signals

```cpp
#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
    // Catch the small signals
    beauty::signal({SIGUSR1, SIGUSR2}, [](int s) {
        std::cout << "Shot miss..." << std::endl;
    });

    // Catch the big one
    beauty::signal(SIGINT, [](int s) {
        std::cout << "Head shot !" << std::endl;
        beauty::stop();
    });

    // Wait for the end
    beauty::wait();
}
```

- Websocket server

Here an example of a simple chat server using websocket, use `ws://127.0.0.1:8085/chat/MyRoom` to connect
to a room named MyRoom.

```cpp
#include <beauty/beauty.hpp>

#include <string>
#include <unordered_map>

using Sessions = std::unordered_map<std::string /* UUID */, std::weak_ptr<beauty::websocket_session>>;
using Rooms = std::unordered_map<std::string /* ROOM */,  Sessions>;

Rooms rooms;

int
main(int argc, char* argv[])
{
    beauty::server server;

    server.add_route("/chat/:room")
        .ws(beauty::ws_handler{
            .on_connect = [](const beauty::ws_context& ctx) {
                rooms[ctx.attributes["room"].as_string()][ctx.uuid] = ctx.ws_session;
            },
            .on_receive = [](const beauty::ws_context& ctx, const char* data, std::size_t size, bool is_text) {
                for (auto& [uuid, session] : rooms[ctx.attributes["room"].as_string()]) {
                    if (auto s = session.lock(); s) {
                        s->send(std::string(data, size));
                    }
                }
            },
            .on_disconnect = [](const beauty::ws_context& ctx) {
                for (auto& [name, room] : rooms) {
                    room.erase(ctx.uuid);
                }
            }
        });

    server.listen(8085);

    beauty::wait();
}

```


- Websocket client

Here an example of a simple client that connect to the previous chat server,
use `ws://127.0.0.1:8085/chat/MyRoom` to connect to a room named MyRoom.

```cpp
int
main(int argc, char* argv[])
{
    beauty::client client;

    client.ws(argv[1], beauty::ws_handler{
        .on_connect = [](const beauty::ws_context& ctx) {
            std::cout << "--- Connected --- to: " << ctx.remote_endpoint << std::endl;
        },
        .on_receive = [](const beauty::ws_context& ctx, const char* data, std::size_t size, bool is_text) {
            std::cout << "--- Received:\n";
            std::cout.write(data, size);
            std::cout << "\n---" << std::endl;

        },
        .on_disconnect = [&client](const beauty::ws_context& ctx) {
            std::cout << "--- Disconnected ---" << std::endl;
        },
        .on_error = [&client](boost::system::error_code ec, const char* what) {
            std::cout << "--- Error: " << ec << ", " << ec.message() << ": " << what << std::endl;

            std::cout << "Retrying connection on error in 1s..." << std::endl;
            beauty::after(1.0, [&client] {
                std::cout << "Trying connection..." << std::endl;
                client.ws_connect();
            });
        }
    });

    std::cout << "> ";
    std::cout.flush();
    for(std::string line; getline(std::cin, line);) {
        if (line == "q") break;
        std::cout << "> ";
        std::cout.flush();
        client.ws_send(std::move(line));
    }
}
```


Further examples can be found into the binaries directory at the root of the project.

## Build

Beauty depends Boost.Beast and OpenSsl. You can rely on Conan to get the package or
only the FindPackage from CMake.

### Linux

For Conan, you need to provide a profile, here, using default as profile.

```shell
git clone https://github.com/dfleury2/beauty.git
cd beauty/
conan install . -pr default -pr:b default -b missing -of build
cmake -S . -B build --preset conan-release
cmake --build build --preset conan-release
```

If you do not want to use Conan, you can try:

```shell
git clone https://github.com/dfleury2/beauty.git
cd beauty/
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Hope your the dependencies are found on your Linux.

If you want to disable openssl:
```shell
conan install . -o "&:openssl=False" -pr default -pr:b default -b missing -of build
```

In case you want to build and run unit tests, you can use the following command:

```shell
git clone https://github.com/dfleury2/beauty.git
cd beauty/
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
cd build/tests/
ctest
```

To build the examples, you can use the following command:

```shell
git clone https://github.com/dfleury2/beauty.git
cd beauty/
cmake -S . -B build -DBEAUTY_BUILD_EXAMPLES=ON
cmake --build build
cd build/examples/
./beauty_application # or any other example
```

### Linux Makefile

For those who want just a simple Makefile without bothering with dependency management,
in the `docs` directory, there is an example of a simple one-shot Makefile to create a
library archive to be used in another project. This Makefile must used (and moved) at the
at the project root.

```makefile
LIB  = libeauty.a
OBJS = src/acceptor.o \
       src/application.o \
       src/attributes.o \
       src/client.o \
       src/exception.o \
       src/route.o \
       src/router.o \
       src/server.o \
       src/sha1.o \
       src/signal.o \
       src/swagger.o \
       src/timer.o \
       src/url.o \
       src/utils.o

.cpp.o:
	g++ -std=c++17 -Wall -O2 -c -o $@ $< -I./include -I./build/include

$(LIB): version.hpp $(OBJS)
	ar -r $@ $(OBJS)

version.hpp:
	VERSION=1.0.4 envsubst < src/version.hpp.in > ./include/beauty/version.hpp

clean:
	rm -f libeauty.a ./src/*.o ./include/beauty/version.hpp
```

### Windows

You must have a valid profile for VS Build. At this time conan center provide only pre-built packages
for VS2019 (compiler.version = 16) and x86_64 mode (arch=x86_64). Boost and OpenSSL can be compiled
automatically for VS2022.

```shell
git clone https://github.com/dfleury2/beauty.git
cd beauty
conan install . -o "&:openssl=False" -pr vs2019 -pr:b vs2019 -b missing -of build
cmake -S . -B build --preset conan-release
cmake --build build --config Release
```

The binaries will be created in the `build\examples\Release` directory.cd ..

or for Visual Studio 2022. Unfortunately at this time, I did not succeed to compile
openssl with compiler.version = 17...


To be improved...
