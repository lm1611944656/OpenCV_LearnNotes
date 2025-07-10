# 遍历当前文件下的所有源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ccurl CCURL_LIST)

# 打印变量
message("ccurl目录下的源码为：${CCURL_LIST}")

# 提升变量CCURL_LIST的范围， 并且给这个变量赋予新的变量名。如果不提升CCURL_LIST的作用域，那么该变量的作用域只能在子目录
set(NEW_CCURL_LIST ${CCURL_LIST} PARENT_SCOPE)
# message("22222 src 目录下的源码为：${NEW_SRC_LIST}")

# 指定静态库的输出目录
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libarys)

# 指定动态库的输出目录
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libarys)

# 生成静态库
# add_library(SRC_LIB STATIC ${SRC_LIST})

# 生成动态库
#add_library(slib SHARED ${TASKS_LIST})

# 为目标指定头文件
#target_include_directories(slib PUBLIC
#    ${PROJECT_SOURCE_DIR}/tasks
#    "/usr/local/include/opencv4")

# 输出动态库的输出目录
#message("Dynamic library output directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")