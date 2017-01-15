#include "stdio.h"
#include "sds.h"

int main(int argc, char const *argv[])
{
    sds s = sdsnewlen("", 0);
    printf("%s\n", s);
    return 0;
}