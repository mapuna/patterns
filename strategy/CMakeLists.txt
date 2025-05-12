# CMakeLists.txt for the strategy pattern example

# Define the tree library target
add_library(tree STATIC
    tree.cpp
)

# Set output name to libtree.a
set_target_properties(tree PROPERTIES
    OUTPUT_NAME tree
    PREFIX "lib"
)

# Define the strategy example executable target
add_executable(strategy_example
    main.cpp
)

# Link the executable with the tree library
target_link_libraries(strategy_example PRIVATE tree)

# Set target properties
set_target_properties(strategy_example PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Install targets
install(TARGETS strategy_example tree
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
