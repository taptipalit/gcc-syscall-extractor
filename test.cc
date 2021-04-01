void fizz();
void buzz();
void fizzbuzz();
void print_number(int n);

void test(int n)
{
    void (*fptr)();
    fptr = fizz;
    for (int i = 0; i < n; i++)
    {
        bool div_3 = i % 3 == 0;
        bool div_5 = i % 5 == 0;
        if (div_3 && div_5)
            fizzbuzz();
        else if (div_3)
            fizz();
        else if (div_5)
            buzz();
        else
            print_number(i);
    }
    asm("mov $1, %rax");
    asm("mov $0, %rax \n\t syscall\n\t");
}

