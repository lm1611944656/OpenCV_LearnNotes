# 遍历当前文件下的所有源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/tasks2 TASK2_LIST)

# 打印变量
message("2222 src 目录下的源码为：${TASK2_LIST}")

# 提升变量SRC_LIST的范围， 并且给这个变量赋予新的变量名。如果不提升SRC_LIST的作用域，那么该变量的作用域只能在子目录
#set(NEW_SRC_LIST ${SRC_LIST} PARENT_SCOPE)
# message("22222 src 目录下的源码为：${NEW_SRC_LIST}")

# 指定静态库的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libarys)

# 指定动态库的输出目录
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libarys)

# 生成静态库
add_library(alib STATIC ${TASK2_LIST})

# 生成动态库
# add_library(SRC_LIB SHARED ${SRC_LIST})

# 为目标指定头文件
target_include_directories(alib PUBLIC
    ${PROJECT_SOURCE_DIR}/tasks2
    "/usr/local/include/opencv4")