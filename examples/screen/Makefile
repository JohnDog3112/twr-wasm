CC := clang
TWRCFLAGS := --target=wasm32 -nostdinc -nostdlib -isystem  ../../include
CPPLIB := ../twr-cpp
CFLAGS := -c -Wall -O3 $(TWRCFLAGS) -I $(CPPLIB)
CFLAGS_DEBUG := -c -Wall -g -O0  $(TWRCFLAGS) -I $(CPPLIB)

$(info $(shell mkdir -p out))

.PHONY: default

default: screen.wasm screen-a.wasm pong window out/appSpawner.js

pong:
	(cd ../pong && make)

window:
	(cd ../window && make)

screen.o: screen.c
	$(CC) $(CFLAGS)  $< -o $@

screen-a.o: screen.c
	$(CC) $(CFLAGS)  $< -o $@ -DASYNC

screen.wasm: screen.o
	wasm-ld  screen.o ../../lib-c/twr.a -o screen.wasm \
		--no-entry --initial-memory=4063232 --max-memory=4063232 \

screen-a.wasm: screen-a.o
	wasm-ld screen-a.o ../../lib-c/twr.a -o screen-a.wasm \
		--no-entry --shared-memory --no-check-features --initial-memory=4063232 --max-memory=4063232 \

out/appSpawner.js: appSpawner.ts
	tsc

clean:
	rm -f *.o
	rm -f *.wasm
	rm -f out/*
	(cd ../pong && make clean)
	(cd ../window && make clean)