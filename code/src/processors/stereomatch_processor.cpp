#include "stereomatch_processor.h"

StereoProcessor::StereoProcessor(QObject *parent)
  : TwoImageProcessor(parent)
{
}

StereoProcessor::~StereoProcessor()
{
}

void StereoProcessor::run()
{
  forever {
    if(abort) return;
	//
	dynamicProgramming();
	//
    mutex.lock();
    output_image = right_image;
    emit progress(100);
    emit updated();
    if(once) return;

    if(!restart)
      condition.wait(&mutex);
    restart = false;
    mutex.unlock();
  }
}

/*
For every pixel in the right image, we extract the 7-by-7-pixel block around it
and search along the same row in the left image for the block that best matches it.
Here we search in a range of +/- 15 pixels (the disparity range) around the
pixel's location in the first image, and we use the sum of absolute differences (SAD)
to compare the image regions.
*/

/*
for (m = 1 ... nRowsLeft){
    // Set min/max row bounds for image block.
    minrow = max(1,m-halfBlockSize);
    maxrow = min(nRowsLeft,m+halfBlockSize);
    for (n = 1 ... nColsLeft){
        mincol = max(1,n-halfBlockSize);
        maxcol = min(nColsLeft,n+halfBlockSize);
        // Compute disparity bounds.
        mindisp = max( -disparityRange, 1-mincol );
        maxdisp = min( disparityRange, nColsLeft-maxcol );
        // Construct template and region of interest.

	template = rightImage-> (minrow to maxrow) (mincol to maxcol)

        templateCenter = floor((size(template)+1)/2);

	calculate centre of template: centreX, centreY
        regionOfInterest = [mincol+centreX+mindisp-1   minrow+centreY-1   maxdisp-mindisp+1   1];

	// Then I guess... search within that region of interest for the best match?
	// find 'loc'

	index = loc - regionOfInterest + mindisp;

	outputImage(m,n) = index;

	// DbasicSubpixel(m,n) = ix - 0.5 * (a2(2,3) - a2(2,1)) / (a2(2,1) - 2*a2(2,2) + a2(2,3));
	// Where a2(2,1) = left neighbour pixel, a2(3,1) = right neighbour pixel, a2(2,1) = pixel
	// Maybe run this after all are completed? Is it neighbours in original image?


     }
}
// Need to normalise output image to [0...255]
// For display purposes, we saturate the depth map to have only positive values.
*/

// left image = input_image
// right image = right_image
void StereoProcessor::dynamicProgramming(){
	// nRowsLeft = number of rows in left image
	int nRowsLeft = input_image.rows;
	// nColsLeft = number of cols in left image
	int nColsLeft = input_image.cols;
	// set output image to all zeroes
	genericDepthMap = Mat(nRowsLeft, nColsLeft, CV_64F, Scalar(0));
	int disparityRange = 15;
	int halfBlockSize = 3;
	int blockSize = 2*halfBlockSize+1;
	
}

