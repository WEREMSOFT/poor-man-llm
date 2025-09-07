ifdef ENABLE_MULTI
    CFLAGS += -DMULTI
endif

all: build
	./main.bin

run_multi: clean
	gcc -DMULTI -g -Wunused -Wunreachable-code -Wall -Wextra -pedantic -std=c89 -lpthread main.c -o main.bin
	./main.bin

build_vscode: clean
	gcc -DMULTI -g -Wunused -Wunreachable-code -Wall -Wextra -pedantic -std=c89 -lpthread main.c -o main.bin

build: clean
	gcc $(CFLAGS) -g -Wunused -Wunreachable-code -Wall -Wextra -pedantic -std=c89 -lpthread main.c -o main.bin

build_multi: clean
	gcc -DMULTI -O0 -g -Wunused -Wunreachable-code -Wall -Wextra -pedantic -std=c89 -lpthread main.c -o main.bin

build_release: clean
	gcc -DMULTI -O3 -march=native -flto -fno-exceptions -fomit-frame-pointer -DNDEBUG -std=c89 -lpthread main.c -o main.bin

run_release: clean clean_data build_release
	./main.bin the man

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.out

clean_data:
	rm -rf model_data
	mkdir model_data
	touch model_data/.gitkeep

valgrind: build
	valgrind --tool=memcheck ./main.bin
	
