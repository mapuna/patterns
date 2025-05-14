# CMakeLists.txt for the singleton pattern example

# Find the concurrentqueue package
find_package(concurrentqueue CONFIG REQUIRED)

# Define the logger library target
add_library(logger INTERFACE)
target_include_directories(logger INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Define the singleton example executable target
add_executable(singleton_example
    main.cpp
)

# Link the executable with the logger library and concurrentqueue
target_link_libraries(singleton_example PRIVATE 
    logger 
    concurrentqueue::concurrentqueue
)

# Set target properties
set_target_properties(singleton_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Install targets
install(TARGETS singleton_example logger
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
