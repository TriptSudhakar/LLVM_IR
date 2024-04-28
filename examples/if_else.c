int printf(char* format, ...);

int
main()
{
    int x = 2;
    int y = 3;
    int z = x + y;

    float b = 8/3.0;

    if(1.4 && 0.0)
    {
        printf("hello world\n");
        printf("b = %f\n", b);
    }

    if(1 || 2) printf("1 || 2\n");
    
    if(1/2 == 2/1) printf("1/2 == 2/1\n");
    else printf("1/2 != 2/1\n");

    printf("z = %d\n", z);
    return 0;
}