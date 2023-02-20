CFLAGS=-std=c11 -g -static

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): main.h


# ASSERTのプリプロセスのみを行い、テストコードを自前のコンパイラでコンパイルする
test/main.s: main test/main.c
	$(CC) -o- -E -P -C test/main.c | ./main - > test/main.s

test/tmp2.o:
	echo 'int char_fn() { return 257; }' | $(CC) -xc -c -o test/tmp2.o -

test/main.exe: test/main.s test/tmp2.o test/common.c
	$(CC) -static -o $@ test/main.s test/tmp2.o -xc test/common.c

test: test/main.exe
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done


examples/nqueen.s: examples/nqueen.C main
	./main examples/nqueen.C > $@

nqueen.exe: examples/nqueen.s
	$(CC) -static -o $@ $^

nqueen: nqueen.exe
	./nqueen.exe


clean:
	rm -rf chibicc tmp* test/*.s test/*.o test/*.exe main examples/nqueen.s
	find * -type f '(' -name '*~' -o -name '*.o' -o -name '*.exe' ')' -exec rm {} ';'

.PHONY: test clean nqueen