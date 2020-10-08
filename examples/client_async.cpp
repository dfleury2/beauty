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
                   } else if (!ec) {
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
