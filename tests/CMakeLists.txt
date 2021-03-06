
cmake_minimum_required(VERSION 3.1)
project(C_MAKE_DEMO)

if(APPLE)
    # no warning
    # https://stackoverflow.com/questions/31561309/cmake-warnings-under-os-x-macosx-rpath-is-not-specified-for-the-following-targe
    set(CMAKE_MACOSX_RPATH 0)
endif()


set(self_binary_name testhash)

set(Source_files 
	${CMAKE_CURRENT_LIST_DIR}/main.c
	${CMAKE_CURRENT_LIST_DIR}/threadaffinity.h
	${CMAKE_CURRENT_LIST_DIR}/../xhash/xhash.h
	${CMAKE_CURRENT_LIST_DIR}/../xhash/xhash.c
	${CMAKE_CURRENT_LIST_DIR}/../xhash/murmur3.c
	)

# more files, you can use if()
#list(APPEND Source_files )

add_executable(${self_binary_name} ${Source_files})

# include other directories
#target_include_directories(${self_binary_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

## defines options link-libraries
#target_compile_definitions(${self_binary_name} PRIVATE "_USE_DATA2")
if (WIN32)
  # visual studio file encoding
  target_compile_options(${self_binary_name} PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
#elseif(APPLE)
else()
  # fPIC for other library link
  target_compile_options(${self_binary_name} PRIVATE -fPIC)
  target_compile_options(${self_binary_name} PRIVATE -fvisibility=hidden)
  target_compile_options(${self_binary_name} PRIVATE -fno-common )
  target_link_libraries(${self_binary_name} pthread) # dlopen  
endif()
if(APPLE)
endif()


# visual studio project tree
source_group(files FILES ${Source_files})
