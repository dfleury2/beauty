#include <beauty/beauty.hpp>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>

// Health check response structure
struct health_status {
    bool healthy{true};
    std::string status{"OK"};
    std::chrono::system_clock::time_point start_time;
    
    [[nodiscard]] nlohmann::json to_json() const {
        const auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - start_time
        ).count();
        
        return {
            {"status", status},
            {"healthy", healthy},
            {"uptime_seconds", uptime},
            {"version", "1.0.0"}  // You might want to get this from your build system
        };
    }
};

// Global health status (in production, consider a more sophisticated state management)
health_status g_health_status{};

// Health check endpoint handler
void handle_health_check(const beauty::request&, beauty::response& res) {
    res.set(beauty::content_type::json);
    res.body() = g_health_status.to_json().dump(2);  // Pretty print with indent=2
}

// Detailed health check with components status
void handle_health_check_detail(const beauty::request&, beauty::response& res) {
    nlohmann::json detailed_status = g_health_status.to_json();
    detailed_status["components"] = {
        {"database", {
            {"status", "OK"},
            {"latency_ms", 42}
        }},
        {"cache", {
            {"status", "OK"},
            {"used_memory_mb", 128}
        }}
    };

    res.set(beauty::content_type::json);
    res.body() = detailed_status.dump(2);
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 3) {
        std::cerr <<
            "Usage: " << argv[0] << " <address> <port>\n" <<
            "Example:\n" <<
            "    " << argv[0] << " 0.0.0.0 8085\n";
        return EXIT_FAILURE;
    }
    auto address = argv[1];
    auto port = std::atoi(argv[2]);

    // Initialize health status with server start time
    g_health_status.start_time = std::chrono::system_clock::now();

    // Http Server
    beauty::server s;

    // Health check endpoints
    s.get("/health", handle_health_check)
     .get("/health/detail", handle_health_check_detail)
     // Example endpoint to demonstrate server functionality
     .get("/hello", [](const beauty::request&, beauty::response& res) {
            res.set(beauty::content_type::text_plain);
            res.body() = "Hello, Beauty!";
        })
     .listen(port, address);

    std::cout << "Health check server running on http://" << address << ":" << port << "\n";
    std::cout << "Try:\n";
    std::cout << "  curl http://" << address << ":" << port << "/health\n";
    std::cout << "  curl http://" << address << ":" << port << "/health/detail\n";

    s.run();
}
