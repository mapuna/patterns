# CMakeLists.txt for the factory pattern example

# Define the factory example executable target
add_executable(factory_example
    main.cpp
)

# Include the current directory for header files
target_include_directories(factory_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set target properties
set_target_properties(factory_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Install targets
install(TARGETS factory_example
    RUNTIME DESTINATION bin
)
