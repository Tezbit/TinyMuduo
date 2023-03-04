cmake_minimum_required(VERSION 3.16)
project(TinyMuduo)

set(CMAKE_CXX_STANDARD 17)
# 设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")

# 添加搜索路径
include_directories(
        ${PROJECT_SOURCE_DIR}/base
        ${PROJECT_SOURCE_DIR}/net
        ${PROJECT_SOURCE_DIR}/http
)

# 包含这些目录下的.cc文件
aux_source_directory(${PROJECT_SOURCE_DIR}/base SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/http SRC_HTTP)

#add_executable(TinyMuduo main.cpp ${SRC_BASE} ${SRC_NET} ${SRC_HTTP})
add_library(TinyMuduo SHARED ${SRC_BASE} ${SRC_NET} ${SRC_HTTP})

# 设置生成动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)