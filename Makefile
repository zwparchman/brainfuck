OFLAGS = -O3
CC=g++
STD=-std=c++14
CFLAGS= -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				$(OFLAGS)
LFLAGS= -g $(STD) $(OFLAGS) 

.PHONY:clean 

all : bfc bf

bfc : bf.cpp
	$(CC) $(OFLAGS) -o bfc bf.cpp -g -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow -lpthread

bf : bf.nim
	nim c bf.nim

dbg: bfc
	gdb bfc

run: bfc
	./bfc

time: bfc
	time ./bfc

cache: bfc
	rm c*grind* -f
	valgrind --tool=cachegrind ./bfc

call: bfc
	rm c*grind* -f
	valgrind --tool=callgrind ./bfc

inspect: 
	kcachegrind c*grind\.out\.*

clean:
	rm -f *o 
	rm -f bfc
	rm -f c*grind\.out\.*
