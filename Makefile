all: build
	./main.bin

build: clean
	gcc -g -Wall -Wextra -pedantic -std=c89 main.c -o main.bin

build_release: clean
	gcc -O3 -march=native -flto -fno-exceptions -fomit-frame-pointer -DNDEBUG -std=c89 main.c -o main.bin

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.out

clean_data:
	rm -rf model_data
	mkdir model_data
	
