include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories("/usr/local/include/opencv4")

# 遍历当前文件下的所有源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

# 打印变量
message("src 目录下的源码为：${SRC_LIST}")

# 提升变量SRC_LIST的范围， 并且给这个变量赋予新的变量名。如果不提升SRC_LIST的作用域，那么该变量的作用域只能在子目录
set(NEW_SRC_LIST ${SRC_LIST} PARENT_SCOPE)

# 指定静态库的输出目录
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libarys)

# 指定动态库的输出目录
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libarys)

# 生成动态库
# add_library(SRC_LIB SHARED ${SRC_LIST})