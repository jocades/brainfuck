build/main: main.cpp
	mkdir -p build
	clang++ -o build/main -O2 -flto main.cpp

run: build/main
	./build/main

clean:
	rm -rf build

.PHONY: run clean
