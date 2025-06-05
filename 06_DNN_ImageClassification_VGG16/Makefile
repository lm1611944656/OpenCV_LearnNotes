.PYTHON:run clean

run:
	cmake -S . -B build && cmake --build build && cd build && make && cd - && ./build/application

clean:
	@rm -rf ./build