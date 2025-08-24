CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11 -Iinclude
LDFLAGS=-static

SRC=src/main.c src/sandbox.c src/compile.c src/util.c
BIN=build/securecode

all: $(BIN)

$(BIN): $(SRC) | build logs workspace sandbox_root
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

build:
	mkdir -p build
logs:
	mkdir -p logs
workspace:
	mkdir -p workspace
sandbox_root:
	mkdir -p sandbox_root

clean:
	rm -rf build logs/*.txt

run: $(BIN)
	./build/securecode

