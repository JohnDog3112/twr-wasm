CC := clang
TWRCFLAGS := --target=wasm32 -nostdinc -nostdlib -isystem  ../../include
CPPLIB := ../twr-cpp
CFLAGS := -c -Wall -O3 $(TWRCFLAGS) -I $(CPPLIB)
CFLAGS_DEBUG := -c -Wall -g -O0  $(TWRCFLAGS) -I $(CPPLIB)



.PHONY: default

default: tests-uievents.wasm eventTester.js

eventTester.js: index.html eventTester.ts
	tsc

out/tests-uievents.o: tests-uievents.c
	$(CC) $(CFLAGS)  $< -o $@

tests-uievents.wasm: out/tests-uievents.o
	wasm-ld out/tests-uievents.o ../../lib-c/twr.a -o tests-uievents.wasm \
		--no-entry --initial-memory=4063232 --max-memory=4063232 \

clean:
	rm -f *.o
	rm -f *.wasm
	rm -f *.js
	rm -f out/*