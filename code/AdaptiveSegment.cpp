#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace cv;
using namespace std;

void adaptThreshold(Mat I, int* threshold);

int main( int argc, char** argv)
{
  Mat img, grey, hist;
  img = imread(argv[1],1);
  int darkbg = 0;

  if(argc < 2 || !img.data) {
    printf("No image data\n");
    return -1;
  }

  if(argc > 2 && strcmp(argv[2], "darkbg") == 0)
    darkbg = 1;

  cvtColor(img, grey, CV_RGB2GRAY);

  Scalar m = mean(grey);
  int threshold = cvRound(m[0]);

  printf("Threshold before: %d\n", threshold);
  adaptThreshold(grey, &threshold);
  printf("Threshold after: %d\n", threshold);
  

  Mat dst;
  if(darkbg)
    dst = grey < threshold;
  else
    dst = grey >= threshold;

  namedWindow("Display image", CV_WINDOW_AUTOSIZE);
  imshow("Display image", dst);

  waitKey(0);

  return 0;

}

void adaptThreshold(Mat I, int* threshold)
{
  int old_threshold;
  do {
    old_threshold = *threshold;
    Mat above = I >= *threshold;
    Mat below = I < *threshold;
    Scalar mean_above = mean(I, above);
    Scalar mean_below = mean(I, below);
    *threshold = cvRound(((mean_above[0]+mean_below[0])/2));
    
  } while(old_threshold != *threshold);
}
    
