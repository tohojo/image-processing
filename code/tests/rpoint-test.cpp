/**
 * rpoint-test.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-29
 */

#include "rpoint.h"
#include <stdio.h>

using namespace ImageProcessing;

int main(int argc, char *argv[])
{
  printf("(0,0) < (0,0): %s\n",
         RPoint(0,0) < RPoint(0,0) ? "true" : "false");
  printf("(0,0) < (1,0): %s\n",
         RPoint(0,0) < RPoint(1,0) ? "true" : "false");
  printf("(0,0) < (0,1): %s\n",
         RPoint(0,0) < RPoint(0,1) ? "true" : "false");
  printf("(1,0) < (0,0): %s\n",
         RPoint(1,0) < RPoint(0,0) ? "true" : "false");
  printf("(0,1) < (0,0): %s\n",
         RPoint(0,1) < RPoint(0,0) ? "true" : "false");
  printf("(1,0) < (0,5): %s\n",
         RPoint(1,0) < RPoint(0,5) ? "true" : "false");
  printf("(0,5) < (1,0): %s\n",
         RPoint(0,5) < RPoint(1,0) ? "true" : "false");
  printf("\n");
  printf("(0,0) == (0,0): %s\n",
         RPoint(0,0) == RPoint(0,0) ? "true" : "false");
  printf("(0,0) == (1,0): %s\n",
         RPoint(0,0) == RPoint(1,0) ? "true" : "false");
  printf("(0,0) == (0,1): %s\n",
         RPoint(0,0) == RPoint(0,1) ? "true" : "false");
  printf("(1,0) == (0,0): %s\n",
         RPoint(1,0) == RPoint(0,0) ? "true" : "false");
  printf("(0,1) == (0,0): %s\n",
         RPoint(0,1) == RPoint(0,0) ? "true" : "false");
  printf("(1,0) == (0,5): %s\n",
         RPoint(1,0) == RPoint(0,5) ? "true" : "false");
  printf("(0,5) == (1,0): %s\n",
         RPoint(0,5) == RPoint(1,0) ? "true" : "false");
  printf("(5,5) == (5,5): %s\n",
         RPoint(5,5) == RPoint(5,5) ? "true" : "false");
  printf("\n");

  printf("(0,1)+(1,0): "); (RPoint(0,1)+RPoint(1,0)).print(); printf("\n");
  printf("(0,1)+(-1,0): "); (RPoint(0,1)+RPoint(-1,0)).print(); printf("\n");
  return 0;
}

