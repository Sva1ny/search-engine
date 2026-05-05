cmake_minimum_required(VERSION 3.10)
project(Project)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)

    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})

    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})

endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加子目录
add_subdirectory(offline_keyword)
add_subdirectory(offline_page)
add_subdirectory(online)



