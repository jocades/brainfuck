build/bf: main.cpp
	mkdir -p build
	clang++ -o build/bf -O2 -flto main.cpp

run: build/bf
	./build/bf

clean:
	rm -rf build

.PHONY: run clean
