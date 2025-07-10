# 遍历当前文件下的所有源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ctimer TIMER_LIST)

# 打印变量
message("目录下的源码为：${TIMER_LIST}")

# 提升变量SRC_LIST的范围， 并且给这个变量赋予新的变量名。如果不提升SRC_LIST的作用域，那么该变量的作用域只能在子目录
set(NEW_TIMER_LIST ${TIMER_LIST} PARENT_SCOPE)