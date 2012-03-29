/**
 * region-test.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-29
 */

#include "region.h"
#include "rpoint.h"
#include <stdio.h>

using namespace ImageProcessing;

int main(int argc, char *argv[])
{
  Region one, two;

  printf("Adding point (0,0)\n");
  one.add(RPoint(0,0));
  one.print();
  printf("Adding point (0,1)\n");
  one.add(RPoint(0,1));
  one.print();
  printf("Inboundary(0,0): %s\n", one.inBoundary(RPoint(0,0)) ? "true" : "false");
  printf("contains(1,0): %s\n", one.contains(RPoint(1,0)) ? "true" : "false");
  printf("adjacentPoint(1,0): %s\n", one.adjacentPoint(RPoint(1,0)) ? "true" : "false");
  printf("inBoundary(1,0): %s\n", one.inBoundary(RPoint(1,0)) ? "true" : "false");
  printf("interior(1,0): %s\n", one.interior(RPoint(1,0)) ? "true" : "false");
  printf("Adding point (1,0)\n");
  one.add(RPoint(1,0));
  one.print();
  printf("Adding point (1,1)\n");
  one.add(RPoint(1,1));
  one.print();

}
