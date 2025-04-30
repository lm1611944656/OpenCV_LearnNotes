.PYTHON:run clean mldd install

run:
	cmake -S . -B build && cmake --build build && cd build && make && cd - && ./build/application

clean:
	@rm -rf ./build ./libarys

# 查看可执行文件所依赖的动态库
mldd:
	ldd ./build/application

# 打包安装
install:
	cmake --install ./build