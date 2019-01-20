#include <stdio.h>
#include <string.h>

struct A {
    char a[100];
};

int main(int argc, const char * argv[]) {
   
    struct A a;
    memset(&a, 'c', sizeof(struct A));
    
    struct A b;
    b = a;
    
    for (int i = 0; i < 100; i++) {
        if (i != 0 && i % 10 == 0)
            printf("\n");
        printf("%c", b.a[i]);
    }
    printf("\n");
    
    
    return 0;
}