# 定义了一个SEC_LIST变量，包含了该目录下所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(../sec SEC_LIST)
#aux_source_directory(./redis REDIS_LIST)
# 指定生成可执行文件
add_executable(SecClient ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${SEC_LIST})

#指定可执行文件链接时需要的库文件
target_link_libraries(SecClient muduo_net muduo_base mysqlclient hiredis pthread crypto)