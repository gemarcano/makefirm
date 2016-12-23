CC      ?= gcc
CFLAGS  += -Ofast -Wall -Wextra

all:
	@mkdir -pv "build"
	$(CC) $(CFLAGS) makefirm.c -c -o build/makefirm.o
	$(CC) $(CFLAGS) sha256.c -c -o build/sha256.o
	$(CC) $(LDFLAGS) build/makefirm.o build/sha256.o -o makefirm

clean:
	rm -rf "build"
	rm -rf makefirm makefirm.exe
