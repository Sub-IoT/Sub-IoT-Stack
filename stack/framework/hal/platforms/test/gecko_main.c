#include <stdio.h>
#include <efm32gg.h>

void bootstrap();

int main()
{
    printf("config string '%s'\n", config);
    bootstrap();
    printf("bootstrap finished\n");
    return 0;
}