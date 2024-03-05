#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ASTNode.hpp"
#include "c.tab.hpp"


extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;

static void usage()
{
  printf("Usage: cc <prog.c>\n");
}

int
main(int argc, char **argv)
{
  if (argc != 2) {
    usage();
    exit(1);
  }
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  extern ASTNode* root;
  int ret = yyparse();

  root->print();

  printf("retv = %d\n", ret);
  exit(0);
}