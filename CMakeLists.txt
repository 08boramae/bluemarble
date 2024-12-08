cmake_minimum_required(VERSION 3.10)
project(bluemarble C)

set(CMAKE_C_STANDARD 11)

# Server executable
add_executable(server
        server.c
)

# Client executable
add_executable(client
        client.c
)

add_executable(game
        game.c
)

# Link with required libraries
if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
    target_link_libraries(game ws2_32)
endif()