<div align="center">
  <img src="https://github.com/dfleury2/beauty/raw/master/docs/rose.png" height="180" />
  <br>
  <i>A simple Http server and client above <a href="https://github.com/boostorg/beast">Boost.Beast</a>
</div>
<br>

Beauty is a layer above <a href="https://github.com/boostorg/beast">Boost.Beast</a> which provide facilities to create Http server or client. Beauty allows the creation of synchronous or asynchronous server and client, and adds some signals and timer management based on <a href="https://github.com/boostorg/asio">Boost.Asio</a>

Here some examples:

- a synchronous server

```cpp
#include <beauty/beauty.hpp>

int main()
{
    // Create a server
    beauty::server server;

    // Add a default '/' route
    server.get("/", [](const auto& req, auto& res) {
        res.body() = "It's work ;) ... it works! :)";
    })

    // Open the listening port
    server.listen(8085)

    // Run the event loop - no additional thread
    server.run();
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
        // Display the body received
        std::cout << response.body() << std::endl;
    } else {
        // An error occured
        std::cout << ec << ": " << ec.message() << std::endl;
    }
}
```

- timers

```cpp
#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
    // Launch a repeatable timer each 250ms
    beauty::repeat(0.250, [timer_count = 4]() mutable {
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
    beauty::run();
}
```

- signals

```cpp
#include <beauty/beauty.hpp>

#include <iostream>

int main()
{
    // Catch the small signal
    beauty::signal(SIGUSR1, [](int s) {
        std::cout << "Shot miss..." << std::endl;
    });

    // Catch the big one
    beauty::signal(SIGINT, [](int s) {
        std::cout << "Head shot !" << std::endl;
        beauty::stop();
    });

    // Wait for the end
    beauty::run();
}
```

Further examples can be found into the binaries directory at the root of the project.

## Build

Beauty depends Boost.Beast and OpenSsl.

```bash
git clone https://github.com/dfleury2/beauty.git
cd beauty
mkdir build && cd build
cmake ../sources
cmake --build . -j4
```
