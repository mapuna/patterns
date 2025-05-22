# Builder pattern example

# Set the target name
set(TARGET_NAME builder_pattern)

# Source files
set(SOURCES
    main.cpp
    request_builder.cpp
)

# Header files
set(HEADERS
    request_builder.h
)

# Create executable
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install target
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin)
