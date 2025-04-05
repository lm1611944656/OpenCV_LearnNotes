# 指定头文件所在目录，方便func1目录下的源文件使用
include_directories(${PROJECT_SOURCE_DIR}/src/func2
                    ${PROJECT_SOURCE_DIR}/src/utils)

# 遍历当前文件下的所有源文件，放入FUNC1_SOURCE变量中
aux_source_directory(${PROJECT_SOURCE_DIR}/src/func1 FUNC1_SOURCE)
message("func1的源码: ${FUNC1_SOURCE}")

# 设置动态库输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/library)

# 将func1目录下的所有源文件，封装成动态库文件FUNC1.so
add_library(FUNC1 SHARED ${FUNC1_SOURCE})

