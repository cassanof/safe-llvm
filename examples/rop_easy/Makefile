all: build build-safe

build:
	gcc rop.c -static -no-pie -fno-stack-protector -o rop.out

build-safe:
	../../build/bin/clang -static -O0 rop.c -fno-stack-protector -fno-pie -o rop-safe.out

