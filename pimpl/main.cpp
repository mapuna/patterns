#include "http_client.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // Create an instance of HTTPClient
        HTTPClient client;
        
        // Perform a GET request
        std::cout << "Performing GET request to httpbin.org...\n";
        std::string getResponse = client.get("https://httpbin.org/get");
        std::cout << "GET Response:\n" << getResponse << "\n\n";
        
        // Perform a POST request
        std::cout << "Performing POST request to httpbin.org...\n";
        std::string postData = "{\"name\": \"test\", \"value\": 42}";
        std::string postResponse = client.post("https://httpbin.org/post", postData);
        std::cout << "POST Response:\n" << postResponse << "\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
