# 设置变量 SRC_LIST 等于所有要编译的.cc文件
# set(SRC_LIST userservice.cc ../user.pb.cc)

set(SRC_LIST friendservice.cc ../friend.pb.cc)

# 将所有源文件编译成可执行文件provider
add_executable(provider ${SRC_LIST})

# 将可执行文件连接到所需要的库(动态库)
target_link_libraries(provider mprpc protobuf)