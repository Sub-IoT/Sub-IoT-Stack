project(test_aes)
cmake_minimum_required(VERSION 2.8)

add_executable(${PROJECT_NAME} main.c)

#link with the framework library that includes the AES library
target_link_libraries (${PROJECT_NAME} framework)