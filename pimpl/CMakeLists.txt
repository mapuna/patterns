# CMakeLists.txt for the pimpl pattern example

# Find the curl package
find_package(CURL REQUIRED)

# Define the HTTP client library target
add_library(httpclient STATIC
    http_client.cpp
)

# Set output name to libhttpclient.a
set_target_properties(httpclient PROPERTIES
    OUTPUT_NAME httpclient
    PREFIX "lib"
)

# Link httpclient with curl
target_link_libraries(httpclient PRIVATE CURL::libcurl)

# Define the HTTP client example executable target
add_executable(httpclient_example
    main.cpp
)

# Link the executables with their respective libraries
target_link_libraries(httpclient_example PRIVATE httpclient)

# Set target properties
set_target_properties(httpclient_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Install targets
install(TARGETS httpclient_example httpclient
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
