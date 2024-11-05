CC := clang
TWRCFLAGS := --target=wasm32 -nostdinc -nostdlib -isystem  ../../include
CPPLIB := ../twr-cpp
CFLAGS := -c -Wall -O3 $(TWRCFLAGS) -I $(CPPLIB)
CFLAGS_DEBUG := -c -Wall -g -O0  $(TWRCFLAGS) -I $(CPPLIB)

OBJOUTDIR := out
$(info $(shell mkdir -p $(OBJOUTDIR)))

HEADERS := widgets.h

# put all objects into a single directory; assumes unqiue filenames
OBJECTS_RAW := widgets.o widget-test.o
OBJECTS := $(patsubst %, $(OBJOUTDIR)/%, $(OBJECTS_RAW))
OBJECTS_ASYNC := $(patsubst %, $(OBJOUTDIR)/async-%, $(OBJECTS_RAW))

.PHONY: default

default: widget-test.wasm widget-test.wasm


$(OBJOUTDIR)/%.o: %.c $(HEADERS)
	clang $(CFLAGS)  $< -o $@


$(OBJOUTDIR)/async-%.o: %.c $(HEADERS)
	clang $(CFLAGS)  $< -o $@ -DASYNC

# jsEventsLib.js: jsEventsLib.ts
# 	tsc

widget-test.wasm: $(OBJECTS)
	wasm-ld $(OBJECTS) ../../lib-c/twr.a -o widget-test.wasm \
		--no-entry --initial-memory=2031616 --max-memory=2031616

widget-test-a.wasm: $(OBJECTS_ASYNC)
	wasm-ld $(OBJECTS_ASYNC) ../../lib-c/twr.a -o widget-test-a.wasm \
		--no-entry --shared-memory --no-check-features --initial-memory=2031616 --max-memory=2031616
	
run: default
	cd ..
	./server.py

clean:
	rm -f *.wasm
	rm -f out/*