#include <stdio.h>

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

void test(int n)
{
    void (*fptr)();
    fptr = fizz;
    /*
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
    */
    asm("syscall");
    asm("syscall");
    asm("syscall");
    asm("movl $2, %eax;\n\tsyscall;

}

