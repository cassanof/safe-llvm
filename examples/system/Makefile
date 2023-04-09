all: build build-safe

build:
	gcc rop.c -O0 -no-pie -fno-stack-protector -o rop.out

build-safe:
	../../build/bin/clang -O0 rop.c -fno-stack-protector -no-pie -o rop-safe.out

