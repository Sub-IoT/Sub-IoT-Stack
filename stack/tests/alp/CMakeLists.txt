project(test_alp)
cmake_minimum_required(VERSION 2.8)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries (${PROJECT_NAME} alp d7ap_fs framework)
