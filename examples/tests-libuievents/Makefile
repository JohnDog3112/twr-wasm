CC := clang
TWRCFLAGS := --target=wasm32 -nostdinc -nostdlib -isystem  ../../include
CPPLIB := ../twr-cpp
CFLAGS := -c -Wall -O3 $(TWRCFLAGS) -I $(CPPLIB)
CFLAGS_DEBUG := -c -Wall -g -O0  $(TWRCFLAGS) -I $(CPPLIB)



.PHONY: default

default: tests-libuievents.wasm eventTester.js

eventTester.js: index.html eventTester.ts
	tsc

tests-libuievents.o: tests-libuievents.c
	$(CC) $(CFLAGS)  $< -o $@

tests-libuievents.wasm: tests-libuievents.o
	wasm-ld tests-libuievents.o ../../lib-c/twr.a -o tests-libuievents.wasm \
		--no-entry --initial-memory=4063232 --max-memory=4063232 \

clean:
	rm -f *.o
	rm -f *.wasm
	rm -f *.js