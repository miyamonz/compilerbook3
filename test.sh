#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) { 
    return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./main <(echo "$input") > tmp.s
  cc $( [[ -v STATIC ]] && echo '-static') -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 2 'int main() { int x=2; { int x=3; } return x; }'
assert 2 'int main() { int x=2; { int x=3; } { int y=4; return x; }}'
assert 3 'int main() { int x=2; { x=3; } return x; }'

assert 3 'int main() { 1; 2; return 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 1 'int main() { return 1; 2; 3; }'


STATIC=1 assert 97 'int main() { return "abc"[0]; }'
STATIC=1 assert 98 'int main() { return "abc"[1]; }'
STATIC=1 assert 99 'int main() { return "abc"[2]; }'
STATIC=1 assert 0 'int main() { return "abc"[3]; }'
STATIC=1 assert 4 'int main() { return sizeof("abc"); }'

assert 0 'int main() { return ({ 0; }); }'
assert 2 'int main() { return ({ 0; 1; 2; }); }'
assert 1 'int main() { ({ 0; return 1; 2; }); return 3; }'
assert 3 'int main() { return ({ int x=3; x; }); }'

assert 2 'int main() { /* return 1; */ return 2; }'
assert 2 'int main() { // return 1;
return 2; }'

echo OK