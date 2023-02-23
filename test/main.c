#define ASSERT(x, y) assert(x, y, #y)

int gg2;
int *gg2_ptr()
{
    gg2 = 3;
    return &gg2;
}

char char_fn();

int count()
{
    static cnt;
    cnt = cnt + 1;
    return cnt;
}
int param_decay(int x[]) { return x[0]; }

int main()
{
    test_arith();
    test_control();
    test_function();
    test_pointer();
    test_string();
    test_variable();
    test_struct();
    test_type();

    ASSERT(24, ({ int *x[3]; sizeof(x); }));
    ASSERT(8, ({ int (*x)[3]; sizeof(x); }));
    ASSERT(3, ({ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }));
    ASSERT(4, ({ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }));

    ASSERT(3, *gg2_ptr());

    {
        void *x;
    }

    ASSERT(0, ({ _Bool x=0; x; }));
    ASSERT(1, ({ _Bool x=1; x; }));
    ASSERT(1, ({ _Bool x=2; x; }));

    ASSERT(1, ({ char x; sizeof(x); }));
    ASSERT(2, ({ short int x; sizeof(x); }));
    ASSERT(2, ({ int short x; sizeof(x); }));
    ASSERT(4, ({ int x; sizeof(x); }));
    ASSERT(4, ({ typedef t; t x; sizeof(x); }));
    ASSERT(8, ({ long int x; sizeof(x); }));
    ASSERT(8, ({ int long x; sizeof(x); }));

    ASSERT(1, sizeof(char));
    ASSERT(2, sizeof(short));
    ASSERT(2, sizeof(short int));
    ASSERT(2, sizeof(int short));
    ASSERT(4, sizeof(int));
    ASSERT(8, sizeof(long));
    ASSERT(8, sizeof(long int));
    ASSERT(8, sizeof(long int));
    ASSERT(8, sizeof(char *));
    ASSERT(8, sizeof(int *));
    ASSERT(8, sizeof(long *));
    ASSERT(8, sizeof(int **));
    ASSERT(8, sizeof(int(*)[4]));
    ASSERT(32, sizeof(int *[4]));
    ASSERT(16, sizeof(int[4]));
    ASSERT(48, sizeof(int[3][4]));

    ASSERT(4, sizeof(0));
    ASSERT(4294967297, 4294967297); // 修正前では1になるが、コンパイラがどっちも1としてpassしちゃうという細かい話がある
    ASSERT(8, sizeof(4294967297));

    // 8590066177 = 0x200020201
    // 513          0x      201
    // 131585     = 0x    20201
    ASSERT(131585, (int)8590066177);
    ASSERT(513, (short)8590066177);
    ASSERT(1, (char)8590066177);
    ASSERT(1, (_Bool)1);
    ASSERT(1, (_Bool)2);
    ASSERT(0, (_Bool)(char)256);
    ASSERT(1, (long)1);
    ASSERT(0, (long)&*(int *)0);
    ASSERT(5, ({ int x=5; long y=(long)&x; *(int*)y; }));

    ASSERT(97, 'a');
    ASSERT(10, '\n');

    ASSERT(1, char_fn());

    ASSERT(0, ({ enum { zero, one, two }; zero; }));
    ASSERT(1, ({ enum { zero, one, two }; one; }));
    ASSERT(2, ({ enum { zero, one, two }; two; }));
    ASSERT(5, ({ enum { five=5, six, seven }; five; }));
    ASSERT(6, ({ enum { five=5, six, seven }; six; }));
    ASSERT(0, ({ enum { zero, five=5, three=3, four }; zero; }));
    ASSERT(5, ({ enum { zero, five=5, three=3, four }; five; }));
    ASSERT(3, ({ enum { zero, five=5, three=3, four }; three; }));
    ASSERT(4, ({ enum { zero, five=5, three=3, four }; four; }));
    ASSERT(4, ({ enum { zero, one, two } x; sizeof(x); }));
    ASSERT(4, ({ enum t { zero, one, two }; enum t y; sizeof(y); }));

    ASSERT(1, count());
    ASSERT(2, count());
    ASSERT(3, count());

    ASSERT(55, ({ int j=0; for (int i=0; i<=10; i=i+1) j=j+i; j; }));
    ASSERT(3, ({ int i=3; int j=0; for (int i=0; i<=10; i=i+1) j=j+i; i; }));

    ASSERT(3, (1, 2, 3));

    ASSERT(3, ({ int i=2; ++i; }));
    ASSERT(1, ({ int i=2; --i; }));
    ASSERT(2, ({ int i=2; i++; }));
    ASSERT(2, ({ int i=2; i--; }));
    ASSERT(3, ({ int i=2; i++; i; }));
    ASSERT(1, ({ int i=2; i--; i; }));
    ASSERT(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p++; }));
    ASSERT(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; ++*p; }));
    ASSERT(1, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; *p--; }));
    ASSERT(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; --*p; }));

    ASSERT(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[0]; }));
    ASSERT(0, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[1]; }));
    ASSERT(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; a[2]; }));
    ASSERT(2, ({ int a[3]; a[0]=0; a[1]=1; a[2]=2; int *p=a+1; (*p++)--; *p; }));

    ASSERT(7, ({ int i=2; i+=5; i; }));
    ASSERT(7, ({ int i=2; i+=5; }));
    ASSERT(3, ({ int i=5; i-=2; i; }));
    ASSERT(3, ({ int i=5; i-=2; }));
    ASSERT(6, ({ int i=3; i*=2; i; }));
    ASSERT(6, ({ int i=3; i*=2; }));
    ASSERT(3, ({ int i=6; i/=2; i; }));
    ASSERT(3, ({ int i=6; i/=2; }));

    ASSERT(0, !1);
    ASSERT(0, !2);
    ASSERT(1, !0);

    ASSERT(-1, ~0);
    ASSERT(0, ~-1);

    ASSERT(0, 0 & 1);
    ASSERT(1, 3 & 1); // 0b11 & 0b01 = 0b01
    ASSERT(3, 7 & 3); // 0b111 & 0b011 = 0b011
    ASSERT(10, -1 & 10);

    ASSERT(1, 0 | 1);
    ASSERT(3, 2 | 1);
    ASSERT(3, 1 | 3);

    ASSERT(0, 0 ^ 0);
    ASSERT(0, 8 ^ 8);
    ASSERT(4, 7 ^ 3); // 0b111 ^ 0b011 = 0b100
    ASSERT(2, 7 ^ 5); // 0b111 ^ 0b101 = 0b010

    ASSERT(1, 0 || 1);
    ASSERT(1, 0 || (2 - 2) || 5);
    ASSERT(0, 0 || 0);
    ASSERT(0, 0 || (2 - 2));

    ASSERT(0, 0 && 1);
    ASSERT(0, (2 - 2) && 5);
    ASSERT(1, 1 && 5);

    ASSERT(3, ({ int x[2]; x[0]=3; param_decay(x); }));

    ASSERT(8, ({ struct *foo; sizeof(foo); }));
    ASSERT(4, ({ struct T *foo; struct T {int x;}; sizeof(struct T); }));
    ASSERT(1, ({ struct T { struct T *next; int x; } a; struct T b; b.x=1; a.next=&b; a.next->x; }));

    ASSERT(3, ({ int i=0; for(;i<10;i++) { if (i == 3) break; } i; }));
    ASSERT(4, ({ int i=0; while (1) { if (i++ == 3) break; } i; }));
    ASSERT(3, ({ int i=0; for(;i<10;i++) { for (;;) break; if (i == 3) break; } i; }));
    ASSERT(4, ({ int i=0; while (1) { while(1) break; if (i++ == 3) break; } i; }));

    ASSERT(10, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } i; }));
    ASSERT(6, ({ int i=0; int j=0; for (;i<10;i++) { if (i>5) continue; j++; } j; }));
    ASSERT(10, ({ int i=0; int j=0; for(;!i;) { for (;j!=10;j++) continue; break; } j; }));
    ASSERT(11, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } i; }));
    ASSERT(5, ({ int i=0; int j=0; while (i++<10) { if (i>5) continue; j++; } j; }));
    ASSERT(11, ({ int i = 0; int j = 0; while (!i) {while (j++ != 10) continue; break; } j; }));

    printf("OK\n");
    return 0;
}

int test_arith()
{
    ASSERT(0, 0);
    ASSERT(42, 42);
    ASSERT(21, 5 + 20 - 4);
    ASSERT(21, 1 + 2 + 3 + 4 + 5 + 6);
    ASSERT(41, 12 + 34 - 5);
    ASSERT(47, 5 + 6 * 7);
    ASSERT(15, 5 * (9 - 6));
    ASSERT(4, (3 + 5) / 2);
    ASSERT(10, -10 + 20);
    ASSERT(10, - -10);
    ASSERT(10, - -+10);

    ASSERT(0, 0 == 1);
    ASSERT(1, 42 == 42);
    ASSERT(1, 0 != 1);
    ASSERT(0, 42 != 42);

    ASSERT(1, 0 < 1);
    ASSERT(0, 1 < 1);
    ASSERT(0, 2 < 1);
    ASSERT(1, 0 <= 1);
    ASSERT(1, 1 <= 1);
    ASSERT(0, 2 <= 1);

    ASSERT(1, 1 > 0);
    ASSERT(0, 1 > 1);
    ASSERT(0, 1 > 2);
    ASSERT(1, 1 >= 0);
    ASSERT(1, 1 >= 1);
    ASSERT(0, 1 >= 2);

    return 0;
}

int test_control()
{
    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; }));
    ASSERT(3, ({ int x; if (1-1) x=2; else x=3; x; }));
    ASSERT(2, ({ int x; if (1) x=2; else x=3; x; }));
    ASSERT(2, ({ int x; if (2-1) x=2; else x=3; x; }));

    ASSERT(3, ({ 1; {2;} 3; }));

    ASSERT(10, ({ int i=0; while(i<10) i=i+1; i; }));
    ASSERT(55, ({ int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} j; }));
    ASSERT(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }));

    return 0;
}

int ret3()
{
    return 3;
    return 5;
}

int ret5()
{
    return 5;
}

int add2(int x, int y)
{
    return x + y;
}

int sub2(int x, int y)
{
    return x - y;
}

int add6(int a, int b, int c, int d, int e, int f)
{
    return a + b + c + d + e + f;
}

int sub_char(char a, char b, char c)
{
    return a - b - c;
}

int fib(int x)
{
    if (x <= 1)
        return 1;
    return fib(x - 1) + fib(x - 2);
}

int test_function()
{
    ASSERT(3, ret3());
    ASSERT(5, ret5());
    ASSERT(8, add2(3, 5));
    ASSERT(2, sub2(5, 3));
    ASSERT(21, add6(1, 2, 3, 4, 5, 6));

    ASSERT(7, add2(3, 4));
    ASSERT(1, sub2(4, 3));
    ASSERT(55, fib(9));

    ASSERT(1, ({ sub_char(7, 3, 3); }));

    return 0;
}

int foo(int *x, int y) { return *x + y; }

int test_pointer()
{
    ASSERT(3, ({ int x=3; *&x; }));
    ASSERT(3, ({ int x=3; int *y=&x; int **z=&y; **z; }));
    ASSERT(5, ({ int x=3; int y=5; *(&x+1); }));
    ASSERT(5, ({ int x=3; int y=5; *(1+&x); }));
    ASSERT(3, ({ int x=3; int y=5; *(&y-1); }));
    ASSERT(5, ({ int x=3; int y=5; *(&x-(-1)); }));

    ASSERT(5, ({ int x=3; int y=5; int *z=&x; *(z+1); }));
    ASSERT(3, ({ int x=3; int y=5; int *z=&y;  *(z-1); }));
    ASSERT(5, ({ int x=3; int *y=&x; *y=5;  x; }));
    ASSERT(7, ({ int x=3; int y=5; *(&x+1)=7;  y; }));
    ASSERT(7, ({ int x=3; int y=5; *(&y-1)=7;  x; }));
    ASSERT(8, ({ int x=3; int y=5;  foo(&x, y); }));

    ASSERT(3, ({ int x[2]; int *y=&x; *y=3; *x; }));

    ASSERT(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }));
    ASSERT(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }));
    ASSERT(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }));

    ASSERT(0, ({ int x[2][3]; int *y=x; *y=0; **x; }));
    ASSERT(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }));
    ASSERT(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }));
    ASSERT(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }));
    ASSERT(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }));
    ASSERT(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }));
    ASSERT(6, ({ int x[2][3]; int *y=x; *(y+6)=6; **(x+2); }));

    ASSERT(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }));
    ASSERT(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }));
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }));
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }));
    ASSERT(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }));

    ASSERT(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }));
    ASSERT(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }));
    ASSERT(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }));
    ASSERT(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }));
    ASSERT(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }));
    ASSERT(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }));
    ASSERT(6, ({ int x[2][3]; int *y=x; y[6]=6; x[2][0]; }));

    return 0;
}

int test_string()
{
    ASSERT(0, ""[0]);
    ASSERT(97, "abc"[0]);
    ASSERT(98, "abc"[1]);
    ASSERT(99, "abc"[2]);
    ASSERT(0, "abc"[3]);
    ASSERT(4, sizeof("abc"));

    ASSERT(7, "\a"[0]);
    ASSERT(8, "\b"[0]);
    ASSERT(9, "\t"[0]);
    ASSERT(10, "\n"[0]);
    ASSERT(11, "\v"[0]);
    ASSERT(12, "\f"[0]);
    ASSERT(13, "\r"[0]);
    ASSERT(27, "\e"[0]);
    ASSERT(0, "\0"[0]);

    ASSERT(106, "\j"[0]);
    ASSERT(107, "\k"[0]);
    ASSERT(108, "\l"[0]);

    return 0;
}

int g1;
int g2[4];

int test_variable()
{
    ASSERT(3, ({ int a; a=3; a; }));
    ASSERT(8, ({ int a=3; int z=5; a+z; }));
    ASSERT(3, ({ int a=3; a; }));
    ASSERT(8, ({ int a=3; int z=5; a+z; }));
    ASSERT(6, ({ int a; int b; a=b=3; a+b; }));

    ASSERT(3, ({ int foo=3; foo; }));
    ASSERT(8, ({ int foo123=3; int bar=5; foo123+bar; }));

    ASSERT(4, ({ int x; sizeof(x); }));
    ASSERT(4, ({ int x; sizeof x; }));
    ASSERT(8, ({ int *x; sizeof(x); }));

    ASSERT(16, ({ int x[4]; sizeof(x); }));
    ASSERT(48, ({ int x[3][4]; sizeof(x); }));
    ASSERT(16, ({ int x[3][4]; sizeof(*x); }));
    ASSERT(4, ({ int x[3][4]; sizeof(**x); }));
    ASSERT(5, ({ int x[3][4]; sizeof(**x) + 1; }));
    ASSERT(5, ({ int x[3][4]; sizeof **x + 1; }));
    ASSERT(4, ({ int x[3][4]; sizeof(**x + 1); }));

    ASSERT(4, ({ int x=1; sizeof(x=2); }));
    ASSERT(1, ({ int x=1; sizeof(x=2); x; }));

    ASSERT(0, g1);
    ASSERT(3, ({ g1=3; g1; }));
    ASSERT(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; }));
    ASSERT(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; }));
    ASSERT(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; }));
    ASSERT(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; }));

    ASSERT(4, sizeof(g1));
    ASSERT(16, sizeof(g2));

    ASSERT(1, ({ char x=1; x; }));
    ASSERT(1, ({ char x=1; char y=2; x; }));
    ASSERT(2, ({ char x=1; char y=2; y; }));

    ASSERT(1, ({ char x; sizeof(x); }));
    ASSERT(10, ({ char x[10]; sizeof(x); }));

    ASSERT(2, ({ int x=2; { int x=3; } x; }));
    ASSERT(2, ({ int x=2; { int x=3; } int y=4; x; }));
    ASSERT(3, ({ int x=2; { x=3; } x; }));

    return 0;
}

int test_struct()
{
    ASSERT(1, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.a; }));
    ASSERT(2, ({ struct {int a; int b;} x; x.a=1; x.b=2; x.b; }));
    ASSERT(1, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.a; }));
    ASSERT(2, ({ struct {char a; int b; char c;} x; x.b=1; x.b=2; x.c=3; x.b; }));
    ASSERT(3, ({ struct {char a; int b; char c;} x; x.a=1; x.b=2; x.c=3; x.c; }));

    ASSERT(0, ({ struct {int a; int b;} x[3]; int *p=x; p[0]=0; x[0].a; }));
    ASSERT(1, ({ struct {int a; int b;} x[3]; int *p=x; p[1]=1; x[0].b; }));
    ASSERT(2, ({ struct {int a; int b;} x[3]; int *p=x; p[2]=2; x[1].a; }));
    ASSERT(3, ({ struct {int a; int b;} x[3]; int *p=x; p[3]=3; x[1].b; }));

    ASSERT(6, ({ struct {int a[3]; int b[5];} x; int *p=&x; x.a[0]=6; p[0]; }));
    ASSERT(7, ({ struct {int a[3]; int b[5];} x; int *p=&x; x.b[0]=7; p[3]; }));

    ASSERT(6, ({ struct { struct { int b; } a; } x; x.a.b=6; x.a.b; }));

    ASSERT(4, ({ struct {int a;} x; sizeof(x); }));
    ASSERT(8, ({ struct {int a; int b;} x; sizeof(x); }));
    ASSERT(12, ({ struct {int a[3];} x; sizeof(x); }));
    ASSERT(16, ({ struct {int a;} x[4]; sizeof(x); }));
    ASSERT(24, ({ struct {int a[3];} x[2]; sizeof(x); }));
    ASSERT(2, ({ struct {char a; char b;} x; sizeof(x); }));
    // align struct members
    ASSERT(8, ({ struct {char a; int b;} x; sizeof(x); }));
    ASSERT(8, ({ struct {int a; char b;} x; sizeof(x); }));

    // int x; char y;
    // |------|
    // xxxx,,,y
    ASSERT(7, ({ int x; char y; int a=&x; int b=&y; b-a; }));
    // char x; int y;
    //    ||
    // ,,,xyyyy
    ASSERT(1, ({ char x; int y; int a=&x; int b=&y; b-a; }));

    ASSERT(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }));
    ASSERT(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }));

    // ->
    ASSERT(3, ({ struct t {char a;} x; struct t *y = &x; x.a=3; y->a; }));
    ASSERT(3, ({ struct t {char a;} x; struct t *y = &x; y->a=3; x.a; }));

    ASSERT(1, ({ typedef int t; t x=1; x; }));
    ASSERT(1, ({ typedef struct {int a;} t; t x; x.a=1; x.a; }));
    ASSERT(1, ({ typedef int t; t t=1; t; }));
    ASSERT(2, ({ typedef struct {int a;} t; { typedef int t; } t x; x.a=2; x.a; }));

    ASSERT(8, ({ struct t {int a; int b;} x; struct t y; sizeof(y); }));
    ASSERT(8, ({ struct t {int a; int b;}; struct t y; sizeof(y); }));

    return 0;
}

int sub_short(short a, short b, short c)
{
    return a - b - c;
}
int sub_long(long a, long b, long c)
{
    return a - b - c;
}
int test_type()
{
    ASSERT(2, ({ short x; sizeof(x); }));
    ASSERT(4, ({ struct {char a; short b;} x; sizeof(x); }));

    ASSERT(8, ({ long x; sizeof(x); }));
    ASSERT(16, ({ struct {char a; long b;} x; sizeof(x); }));

    ASSERT(1, sub_short(7, 3, 3));
    ASSERT(1, sub_long(7, 3, 3));

    return 0;
}