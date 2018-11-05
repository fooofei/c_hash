
cmake_minimum_required(VERSION 3.1)

project(hashdemo)

set(self_name hashdemo)
add_executable(${self_name} 
  ${CMAKE_CURRENT_LIST_DIR}/test_hash.c
  ${CMAKE_CURRENT_LIST_DIR}/hash.c
)
