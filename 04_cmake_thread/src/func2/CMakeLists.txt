# 指定头文件所在目录，方便func2目录下的源文件使用
include_directories(${PROJECT_SOURCE_DIR}/src/utils
                    ${PROJECT_SOURCE_DIR}/src/func2)

# 遍历当前文件下的所有源文件，放入FUNC2_SOURCE变量中
aux_source_directory(${PROJECT_SOURCE_DIR}/src/func2 FUNC2_SOURCE)
message(STATUS "func2的源码: ${FUNC2_SOURCE}")


# 指定动态库生成的位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/library)

# 将func2目录下的所有源文件，封装成动态库文件FUNC2.so
add_library(FUNC2 SHARED ${FUNC2_SOURCE})