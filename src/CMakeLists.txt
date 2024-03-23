# 搜集所有指定路径下的源文件的文件名 并将输出结果列表存储在指定变量中
# aux_source_directory(. SRC_LIST)

set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    mprpcheader.pb.cc 
    mprpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc 
    logger.cc
    zookeeperutil.cc)
# 将当前目录下的所有源文件生成静态库
add_library(mprpc  ${SRC_LIST})
# 依赖muduo库
target_link_libraries(mprpc muduo_net muduo_base  pthread zookeeper_mt) #在连接muduo库时  net在base前面 顺序要写清楚