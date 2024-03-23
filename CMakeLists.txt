# 设置cmake的最低版本
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(mprpc-jianglong)

# 生成debuf版本 可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

# 设置项目可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# 设置项目库文件搜索路径 -l
link_libraries(${PROJECT_SOURCE_DIR}/lib)

# 添加子目录
# 包含了mprpc框架所有的相关代码
add_subdirectory(src)
# 包含了mprpc框架使用的示例代码
add_subdirectory(example)