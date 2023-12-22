# aux_source_directory(. TEST_LIST)

add_executable(Test test_base64.cpp ../src/sec/rsacrypto.cpp)

#指定可执行文件链接时需要的库文件
target_link_libraries(Test muduo_net muduo_base mysqlclient hiredis pthread crypto)
