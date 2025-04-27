# 指定头文件所在目录，方便utils目录下的源文件使用
include_directories(${PROJECT_SOURCE_DIR}/src/utils)

# 遍历当前文件下的所有源文件，放入UTILS_SOURCE变量中
aux_source_directory(${PROJECT_SOURCE_DIR}/src/utils UTILS_SOURCE)

# 指定静态库生成的位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/library)

# 将utils目录下的所有源文件，封装成静态库文件UTILS.a
add_library(UTILS STATIC ${UTILS_SOURCE})