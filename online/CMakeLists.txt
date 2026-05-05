cmake_minimum_required(VERSION 3.22)
project(online)

# # 设置可执行文件输出路径
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

# 添加调试信息
message(STATUS "Configuring online")

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/net)
include_directories(${PROJECT_SOURCE_DIR}/include/threadpool)
include_directories(${CMAKE_SOURCE_DIR}/Utils/Configuration)
include_directories(${CMAKE_SOURCE_DIR}/Utils/split)
include_directories(${CMAKE_SOURCE_DIR}/Utils/Timer)
include_directories(${CMAKE_SOURCE_DIR}/Utils/webPage)
include_directories(/usr/include)
include_directories(/usr/local/include)


# 添加源文件
file(GLOB SOURCES 
"${PROJECT_SOURCE_DIR}/src/*.cpp" 
"${PROJECT_SOURCE_DIR}/src/net/*.cpp" 
"${PROJECT_SOURCE_DIR}/src/threadpool/*.cpp" 
"${CMAKE_SOURCE_DIR}/Utils/Configuration/*.cpp" 
"${CMAKE_SOURCE_DIR}/Utils/webPage/*.cpp")

# 添加可执行文件或库
add_executable(online ${SOURCES})

# 链接需要的库（如果有的话）
target_link_libraries(online boost_regex tinyxml2)


# 设置可执行文件输出路径
set_target_properties(online PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../online)

# 添加调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
