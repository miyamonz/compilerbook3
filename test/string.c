#include "test.h"

int main()
{
    ASSERT(0, ""[0]);
    assert(97, "abc"[0]);
    assert(98, "abc"[1]);
    assert(99, "abc"[2]);
    assert(0, "abc"[3]);
    assert(4, sizeof("abc"));

    assert(7, "\a"[0]);
    assert(8, "\b"[0]);
    assert(9, "\t"[0]);
    assert(10, "\n"[0]);
    assert(11, "\v"[0]);
    assert(12, "\f"[0]);
    assert(13, "\r"[0]);
    assert(27, "\e"[0]);
    assert(0, "\0"[0]);

    assert(106, "\j"[0]);
    assert(107, "\k"[0]);
    assert(108, "\l"[0]);

    printf("OK");
    return 0;
}