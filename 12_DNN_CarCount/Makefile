builddir := ./build
app := application
ku1 := library


.PYTHON:run clean

run:
	@cmake -S . -B build && cmake --build build && cd $(builddir) && make -j4 && cd - && $(builddir)/$(app)

clean:
	@rm -rf $(builddir) $(ku1) 