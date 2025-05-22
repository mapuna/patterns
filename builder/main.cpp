#include <exception>
#include <iostream>

#include "request_builder.h"

void builder_pattern_demo( ) {
    std::cout << "=== HTTP Request Builder Pattern Demo ===\n";

    try {
        std::cout << "1. Simple GET Request:\n";
        auto simple_reuqest =
            HttpRequestBuilder("https://reqbin.com/echo/get/json",
                               HttpMethod::GET)
                .add_header("Accept", "application/json")
                .build( );
        std::cout << simple_reuqest->to_string( ) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what( ) << "\n";
    }
}

int main( ) {
    builder_pattern_demo( );
    return 0;
}
