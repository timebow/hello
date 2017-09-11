#include <stdio.h>

extern void f1(void);
extern void f2(void);

int main(void)
{
   printf(">> main start!\n");

   f1();
   f2();

   printf(">> main end!\n");
   
   return 0;
}
