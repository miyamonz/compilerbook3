#define ASSERT(x, y) assert(x, y, #y)

int main()
{
    test_arith();
    test_control();
    test_function();
    test_pointer();
    test_string();
    test_variable();
    test_struct();
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

    printf("OK");
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

    printf("OK");
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

    printf("OK");
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

    printf("OK");
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

    printf("OK");
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

    ASSERT(8, ({ int x; sizeof(x); }));
    ASSERT(8, ({ int x; sizeof x; }));
    ASSERT(8, ({ int *x; sizeof(x); }));

    ASSERT(32, ({ int x[4]; sizeof(x); }));
    ASSERT(96, ({ int x[3][4]; sizeof(x); }));
    ASSERT(32, ({ int x[3][4]; sizeof(*x); }));
    ASSERT(8, ({ int x[3][4]; sizeof(**x); }));
    ASSERT(9, ({ int x[3][4]; sizeof(**x) + 1; }));
    ASSERT(9, ({ int x[3][4]; sizeof **x + 1; }));
    ASSERT(8, ({ int x[3][4]; sizeof(**x + 1); }));

    ASSERT(8, ({ int x=1; sizeof(x=2); }));
    ASSERT(1, ({ int x=1; sizeof(x=2); x; }));

    ASSERT(0, g1);
    ASSERT(3, ({ g1=3; g1; }));
    ASSERT(0, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[0]; }));
    ASSERT(1, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[1]; }));
    ASSERT(2, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[2]; }));
    ASSERT(3, ({ g2[0]=0; g2[1]=1; g2[2]=2; g2[3]=3; g2[3]; }));

    ASSERT(8, sizeof(g1));
    ASSERT(32, sizeof(g2));

    ASSERT(1, ({ char x=1; x; }));
    ASSERT(1, ({ char x=1; char y=2; x; }));
    ASSERT(2, ({ char x=1; char y=2; y; }));

    ASSERT(1, ({ char x; sizeof(x); }));
    ASSERT(10, ({ char x[10]; sizeof(x); }));

    ASSERT(2, ({ int x=2; { int x=3; } x; }));
    ASSERT(2, ({ int x=2; { int x=3; } int y=4; x; }));
    ASSERT(3, ({ int x=2; { x=3; } x; }));

    printf("OK");
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

    ASSERT(8, ({ struct {int a;} x; sizeof(x); }));
    ASSERT(16, ({ struct {int a; int b;} x; sizeof(x); }));
    ASSERT(24, ({ struct {int a[3];} x; sizeof(x); }));
    ASSERT(32, ({ struct {int a;} x[4]; sizeof(x); }));
    ASSERT(48, ({ struct {int a[3];} x[2]; sizeof(x); }));
    ASSERT(2, ({ struct {char a; char b;} x; sizeof(x); }));
    // align struct members
    ASSERT(16, ({ struct {char a; int b;} x; sizeof(x); }));
    ASSERT(16, ({ struct {int a; char b;} x; sizeof(x); }));

    // int x; char y;
    // |------|
    // xxxx,,,y
    ASSERT(15, ({ int x; char y; int a=&x; int b=&y; b-a; }));
    // char x; int y;
    //    ||
    // ,,,xyyyy
    ASSERT(1, ({ char x; int y; int a=&x; int b=&y; b-a; }));

    ASSERT(16, ({ struct t {int a; int b;} x; struct t y; sizeof(y); }));
    ASSERT(16, ({ struct t {int a; int b;}; struct t y; sizeof(y); }));
    ASSERT(2, ({ struct t {char a[2];}; { struct t {char a[4];}; } struct t y; sizeof(y); }));
    ASSERT(3, ({ struct t {int x;}; int t=1; struct t y; y.x=2; t+y.x; }));

    printf("OK");
    return 0;
}