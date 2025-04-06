.PYTHON:run clean

run:
	cmake -S . -B build && cmake --build build && cd build && make && cd - && ./build/ImageShow

clean:
	@rm -rf ./build