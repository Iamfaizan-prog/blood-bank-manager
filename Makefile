CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Iinclude
CORE = src/core/storage.c src/core/validation.c src/core/donor.c src/core/inventory.c

.PHONY: all tui cli run web clean
all: tui cli

tui: bin/blood_bank
cli: bin/bb_cli

bin/blood_bank: $(CORE) src/tui/tui.c src/main.c
	mkdir -p bin data
	$(CC) $(CFLAGS) $(CORE) src/tui/tui.c src/main.c -o bin/blood_bank

bin/bb_cli: $(CORE) src/cli/main.c
	mkdir -p bin data
	$(CC) $(CFLAGS) $(CORE) src/cli/main.c -o bin/bb_cli

run: tui
	./bin/blood_bank

web: cli
	python3 bridge/server.py

clean:
	rm -rf bin
