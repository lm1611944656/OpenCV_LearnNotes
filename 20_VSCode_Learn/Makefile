BUILDDIR := build

# 指定编译器路径为系统默认的 gcc/g++
CC  := /usr/bin/gcc
CXX := /usr/bin/g++

.PHONY: all clean run


# 方式2，通过命令行添加编译参数
all:
	cmake -S . -B $(BUILDDIR) \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-G "Unix Makefiles"
	cmake --build $(BUILDDIR)

run: all
	./$(BUILDDIR)/application

clean:
	rm -rf $(BUILDDIR)/*

rebuild: clean all