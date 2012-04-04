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
      printf("%3d", m.at<uchar>(Point(j,i)));
      if(j!=s.width-1) printf(",");
    }
    printf("\n");
  }
}


int main(int argc, char *argv[])
{
  Mat mat = Mat::zeros(Size(4,4), CV_8U);
  Mat btmleft(mat, Rect(0,0,2,2));
  Mat btmright(mat, Rect(2,0,2,2));
  Mat topleft(mat, Rect(0,2,2,2));
  Mat topright(mat, Rect(2,2,2,2));

  btmright.setTo(80);
  topleft.setTo(160);
  topright.setTo(255);

  printMatrix(mat);

  Region rbl(btmleft);
  Region rbr(btmright);
  Region rtl(topleft);
  Region rtr(topright);
  rbl.print();
  printMatrix(rbl.toMask(mat));
  rbr.print();
  printMatrix(rbr.toMask(mat));
  rtl.print();
  printMatrix(rtl.toMask(mat));
  rtr.print();
  printMatrix(rtr.toMask(mat));

  printf("\nBottom / top:\n\n");

  Region btm = rbl;
  btm.add(rbr);
  btm.print();
  printMatrix(btm.toMask(mat));
  Region top = rtl;
  top.add(rtr);
  top.print();
  printMatrix(top.toMask(mat));

  printf("\nWhole:\n\n");

  Region whole = top;
  whole.add(btm);
  whole.print();
  printMatrix(whole.toMask(mat));


  printf("\nBottom line:\n\n");

  Mat m00(btmleft, Rect(0,0,1,1));
  Mat m10(btmleft, Rect(1,0,1,1));
  Mat m20(btmright, Rect(0,0,1,1));
  Mat m30(btmright, Rect(1,0,1,1));

  Region btmline(m00);
  btmline.add(Region(m10));
  btmline.add(Region(m20));
  btmline.add(Region(m30));

  btmline.print();

  printMatrix(btmline.toMask(mat));

  printf("\nOutline:\n\n");

  Mat m31(mat, Rect(3,1,1,1));
  Mat m32(mat, Rect(3,2,1,1));
  Mat m33(mat, Rect(3,3,1,1));
  Mat m23(mat, Rect(2,3,1,1));
  Mat m13(mat, Rect(1,3,1,1));
  Mat m03(mat, Rect(0,3,1,1));
  Mat m02(mat, Rect(0,2,1,1));
  Mat m01(mat, Rect(0,1,1,1));


  Region outline = btmline;
  outline.add(Region(m31));
  outline.add(Region(m32));
  outline.add(Region(m33));
  outline.add(Region(m23));
  outline.add(Region(m13));
  outline.add(Region(m03));
  outline.add(Region(m02));
  outline.add(Region(m01));

  outline.print();
  printMatrix(outline.toMask(mat));
}

