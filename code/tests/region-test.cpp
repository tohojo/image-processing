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

  one.add(RPoint(0,0));
  one.add(RPoint(0,1));
  one.add(RPoint(1,0));
  one.add(RPoint(1,1));

  one.print();
}
