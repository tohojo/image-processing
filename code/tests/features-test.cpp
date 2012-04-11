/**
 * features-test.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-10
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cv.h>
#include <highgui.h>
#include "fast_hessian.h"

using namespace std;
using namespace cv;

float dist(Point a, Point b)
{
  return sqrt(pow(a.x-b.x,2)+pow(a.y-b.y,2));
}

int main(int argc, char** argv)
{
  if(argc < 3) {
    cerr << "Usage: " << argv[0] << " <imagefile> <pointsfile>" << endl;
    return 1;
  }
  Mat img = imread(argv[1], 0);

  string line;
  ifstream in(argv[2], ios::in);
  list<Point> points;

  int in_x, in_y;
  char sep;
  while(in) {
    if(getline(in, line)) {
      stringstream stream(line);
      stream >> in_x >> ws;
      sep = stream.peek();
      if(sep == ',')
        stream.get(sep);
      stream >> ws >> in_y;
      points.push_back(Point(in_x, in_y));
    }
  }

  cerr << img.cols << "x" << img.rows << " " << points.size() << endl;
  for(list<Point>::iterator i = points.begin(); i != points.end(); ++i) {
    cerr << "(" << i->x << "," << i->y << ")" << endl;
  }

  float threshold = 1;
  float step = 5;
  float max = 200;
  float dist_fuzz = 15;
  list<KeyPoint> kpts;

  int matches, not_matched;

  cout << "threshold,matches,not-matched,false-pos" << endl;


  for(;threshold <= max; threshold += step) {
    matches = 0;
    not_matched = 0;
    FastHessian fh(img, 4, 4, 2, threshold);
    fh.compute();
    kpts = fh.interestPoints().toStdList();
    for(list<Point>::iterator j = points.begin(); j != points.end(); ++j) {
      bool nomatch = true;
      for(list<KeyPoint>::iterator i = kpts.begin(); i != kpts.end(); ++i) {
        //cout << "Keypoint: (" << i->pt.x << "," << i->pt.y << ") Point: (" << j->x << "," << j->y << ") Dist: " << dist(i->pt, *j) << endl;
        if(dist(i->pt, *j) <= dist_fuzz) { matches++; nomatch = false; }
      }
      if(nomatch) not_matched++;
    }
    cout << threshold << "," << matches << "," << not_matched << "," << kpts.size()-matches << endl;
  }

}

