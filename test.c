#include <stdio.h>

#include <asm/unistd.h>      // compile without -m32 for 64 bit call numbers
void fizz();
void buzz();
void fizzbuzz();
void print_number(int n);


/*
void dothis(int n) {
    void (*fptr)();
    fptr = fizz;
    for (int i = 0; i < n; i++)
    {
        int div_3 = i % 3 == 0;
        int div_5 = i % 5 == 0;
        if (div_3 && div_5)
            fizzbuzz();
        else if (div_3)
            fizz();
        else if (div_5)
            buzz();
        else
            print_number(i);
    }

}
*/

 #define __NR_write 1
ssize_t my_write(int fd, const void *buf, size_t size)
{
    ssize_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        //                 EDI      RSI       RDX
        : "0"(__NR_write), "D"(fd), "S"(buf), "d"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void test(int n)
{
    void (*fptr)();
    fptr = fizz;
    printf("hello\n");
	char *buf = malloc(100);
	my_write(0, buf, 10);
    for (int i = 0; i < n; i++)
    {
        int div_3 = i % 3 == 0;
        int div_5 = i % 5 == 0;
        if (div_3 && div_5)
            fizzbuzz();
        else if (div_3)
            fizz();
        else if (div_5)
            buzz();
        else
            print_number(i);
    }
    DefaultHandler();
    /*
    asm("syscall");
    asm("syscall");
    asm("syscall");
    asm("movl $2, %eax;\n\tsyscall;
    */

}

#define weak_alias(old, new) \
            extern __typeof(old) new __attribute__((weak, alias(#old)))

void inline __attribute__((always_inline)) DefaultHandler() {
            puts("Default Handler");
}
weak_alias( DefaultHandler, Feature1);
compat_symbol (libm, DefaultHandler, fegetenv, GLIBC_2_1);

