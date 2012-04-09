#include "distortion-removal.h"
#include <QtCore/QList>
#include <iostream>
#include <fstream>
#include <highgui.h>


DistortionRemoval::DistortionRemoval(QObject *parent)
: Processor(parent)
{
	squares_across = 4;
	squares_down = 4;
}

DistortionRemoval::~DistortionRemoval()
{
}

void DistortionRemoval::run()
{
	forever {
		if(abort) return;
		mutex.lock();
		bool isEmpty = input_image.empty();
		mutex.unlock();
		if(!isEmpty) {
			emit progress(0);
			qDebug("Processing... Calculating lines.");
			calculateLines();
			if(abort) return;
			if(!restart) {
				emit progress(100);
				output_image = input_image;
				emit updated();
			}
		}
		if(once) return;
		mutex.lock();
		if(!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}
}

void DistortionRemoval::calculateLines(){
	const unsigned int numberOfCorners = (squares_across-1)*(squares_down-1);
	Size patternSize(squares_across-1,squares_down-1);
	std::vector<CvPoint2D32f> cornerList(numberOfCorners);
	vector<Point2f> corners; //this will be filled by the detected corners
	// 1. Find corners to pixel accuracy:
	bool success = findChessboardCorners(input_image, patternSize, corners);
	// 2. Refine corners to sub-pixel accuracy:
	if(success){
		cornerSubPix(input_image, corners, Size(10, 10), Size(-1, -1), TermCriteria(TermCriteria::COUNT, 30, 0.1));
	}
	if (corners.size() != numberOfCorners){
		qDebug("Error in finding corners: choose appropriate chessboard size for input.");
	} else {
//		input_image.copyTo(output_image);
		output_image = input_image.clone();
		drawChessboardCorners(output_image, patternSize, Mat(corners), success);
		qDebug("Lines calculated. Sub-pixel corner points:");
		for (vector<Point2f>::iterator i = corners.begin(); i != corners.end(); i++){
			std::ostringstream strs;
			strs << "   Corner found: <" << (i->x) << "," << (i->y) << "> ";
			std::string output_str = strs.str();
			qDebug(output_str.c_str());
			//points[] = new Point2f(i->x, i->y);			
		}

		// The following assumes that points from the chessboard are stored left-to-right, top-to-bottom.
		// This should be guaranteed by opencv.
		int lineLength = (squares_across-1);
		int lineHeight = (squares_down-1);
		ofstream outFile("output.dat", ios::out);
		Point2f* points = &corners[0];
		int numberOfLines = lineLength+lineHeight;
		outFile << numberOfLines << "\n";
		for (int i = 0; i < numberOfCorners/lineLength; i++ ){ // horizontal lines
			outFile << lineLength << "\n";
			for (int j = 0; j < lineLength; j++ ){
				outFile << points[i*lineLength + j].x << " " << points[i*lineLength + j].y << "\n";
			}
		}
		for (int i = 0; i < numberOfCorners/lineHeight; i++ ){ // vertical lines
			outFile << lineHeight << "\n";
			for (int j = 0; j < lineHeight; j++ ){
				outFile << points[j*lineLength + i].x << " " << points[j*lineLength + i].y << "\n";
			}
		}
		outFile.close();
	}

	// Chessboard lines not being drawn correctly.

	// Next step: try to call the other IPOL_distort code directly.
	// 1. Construct segments that should be straight lines from the point coordinates in "corners".
	// 2. For the best results, use multiple lines.
	// 3. Needs both vertical and horizontal line segments for the optimization procedure to converge.
	// 4. Determine the radial distortion coefficients:
	//		There are numerous camera calibration packages (including one in OpenCV).
	//		But a particularly good open-source ANSI C library can be located here[5]. 
	//		The line segment coordinates are fed into an optimizer which determines the undistorted
	//		coefficients by minimizing the error between the radial distortion model and the training data.
	//		These coefficients can then be stored in a lookup table for run-time image correction.
}

void DistortionRemoval::setSquaresAcross(const int squares){
	QMutexLocker locker(&mutex);
	if(squares_across == squares) return;
	squares_across = squares;
	mutex.unlock();
	process();
}

void DistortionRemoval::setSquaresDown(const int squares){
	QMutexLocker locker(&mutex);
	if(squares_down == squares) return;
	squares_down = squares;
	mutex.unlock();
	process();
}
