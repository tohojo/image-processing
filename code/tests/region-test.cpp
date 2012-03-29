/**
 * region-test.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-03-29
 */

#include "region.h"
#include "rpoint.h"
#include <stdio.h>
#include <cv.h>

using namespace ImageProcessing;

void printMatrix(Mat m) {
  Size s = m.size();
  for(int i = s.height-1; i >= 0; i--) {
    for(int j = 0; j < s.width; j++) {
      printf("%03d,", m.at<uchar>(j,i));
    }
    printf("\n");
  }
}


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

  printf("Adding additional points\n");
  one.add(RPoint(0,2));
  one.add(RPoint(1,2));
  one.add(RPoint(2,0));
  one.add(RPoint(2,1));
  one.add(RPoint(2,2));
  one.print();
  printf("contains(1,1): %s\n", one.contains(RPoint(1,1)) ? "true" : "false");

  Mat m = Mat::ones(5,5,CV_8U);
  printMatrix(m);
  Region(m).print();

  Rect r(3,0,2,2);
  Mat m2 = Mat(m,r);
  printMatrix(m2);
  two = Region(m2,true);
  two.print();
  m2.at<uchar>(1,1)=0;
  Region(m2,true).print();

  one.add(two);
  one.print();

  Mat m3 = one.toMask(m);
  printMatrix(m3);
}

