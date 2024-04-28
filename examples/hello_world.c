int printf(char *format, ...);

int
main()
{
  printf("hello world\n");
  int x;
  x = 2;
  int y = 3;
  int z;
  // printf("%d\n", z);
  int i = 1;
  do {
    printf("%d\n", i);
    i = i + 1;
  } while (i < 10);

  return 0;
}