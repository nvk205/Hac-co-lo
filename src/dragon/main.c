#include <stdio.h>

int main(void){

    int fails = dragon_selftest(1);
    return fails != 0;
}