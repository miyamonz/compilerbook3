CFLAGS=-std=c11 -g -static

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): main.h

test/main.exe: main test/main.c
# ASSERTのプリプロセスのみを行う
	$(CC) -o- -E -P -C test/main.c | ./main - > test/main.s
	echo 'int char_fn() { return 257; }' | $(CC) -xc -c -o test/tmp2.o -
	$(CC) -static -o $@ test/main.s test/tmp2.o -xc test/common.c

test: test/main.exe
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

nqueen.s: examples/nqueen.C main
	./main examples/nqueen.C > $@

nqueen: nqueen.s
	$(CC) -static -o $@ $^

clean:
	rm -rf chibicc tmp* test/*.s test/*.exe
	find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

.PHONY: test clean