builddir := ./build
app := application
ku1 := library


.PYTHON:run clean install_run

run:
	@cmake -S . -B build && cmake --build build && cd $(builddir) && make -j4 && cd - && $(builddir)/$(app)

clean:
	@rm -rf $(builddir) $(ku1) 

install_run:
	@echo "配置、构建并安装项目..."
	@cmake -S . -B $(builddir) && \
	cmake --build $(builddir) --parallel 4 && \
	sudo cmake --install $(builddir)