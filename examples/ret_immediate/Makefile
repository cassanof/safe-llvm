CC = ../../build/bin/clang

all: $(patsubst %.c,%,$(wildcard *.c))

%: %.c
	$(CC) -o $@.out $<

clean:
	rm -f $(patsubst %.c,%,$(wildcard *.c))
