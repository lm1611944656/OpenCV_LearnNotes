# 1. 指定cmake的最低版本要求
cmake_minimum_required(VERSION 3.20)

# 2. 指定项目名称，项目版本，项目描述，项目使用的语言
project(application
        VERSION 1.1.1
        DESCRIPTION "QQ程序"
        LANGUAGES CXX)   

message("当前系统名称：${CMAKE_SYSTEM_NAME}")    # 获取系统名称
message("是否是WIN32: ${WIN32}")                # 判断是否是windows
message("是否是UNIX: ${UNIX}")                 # 判断是否是UNIX

# 设置 libcurl 的根目录
set(CURL_ROOT "/usr/local/curl8_14_1")

# 设置 libevent 根目录
set(LIBEVENT_ROOT "/usr/local/libevent2_1_12")

# 查找 OpenCV
find_package(OpenCV 4.10 REQUIRED)
message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries to link: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# 查找 libcurl
find_package(CURL REQUIRED)
message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
message(STATUS "CURL include directories: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL libraries to link: ${CURL_LIBRARIES}")

# 去指定的目录下执行cmake文件
add_subdirectory(${PROJECT_SOURCE_DIR}/src/ccurl)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/ctimer)

# 生成可执行文件
add_executable(${PROJECT_NAME} main.cpp 
    ${NEW_CCURL_LIST}
    ${NEW_TIMER_LIST})

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CURL_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/ccurl
    ${PROJECT_SOURCE_DIR}/src/ctimer
    ${LIBEVENT_ROOT}/include
)

# 添加库文件搜索路径
target_link_directories(${PROJECT_NAME} PUBLIC
    ${LIBEVENT_ROOT}/lib
)

# 为目标添加库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pthread                 # 使用多线程需要包含这个库
    ${OpenCV_LIBS}
    ${CURL_LIBRARIES}
    event
    event_core
    event_extra
)