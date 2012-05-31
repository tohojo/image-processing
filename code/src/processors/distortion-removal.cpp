#include "distortion-removal.h"

#include "lens_distortion_estimation.h"

#include <QtCore/QList>
#include <iostream>
#include <fstream>
#include <highgui.h>


DistortionRemoval::DistortionRemoval(QObject *parent)
: Processor(parent)
{
	squares_across = 8;
	squares_down = 8;
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
	vector<Point2f> corners; //this will be filled by the detected corners
	// 1. Find corners to pixel accuracy:
	bool success = findChessboardCorners(input_image, patternSize, corners);
	// 2. Refine corners to sub-pixel accuracy:
	if(success){
		cornerSubPix(input_image, corners, Size(10, 10), Size(-1, -1), TermCriteria(TermCriteria::COUNT, 30, 0.1));
	}
	output_image = input_image.clone();
	if (corners.size() > 0){
		drawChessboardCorners(output_image, patternSize, Mat(corners), success);
	}
	if (corners.size() != numberOfCorners){
		qDebug("=======================");
		qDebug("Error in finding corners:");
		qDebug(" choose appropriate chessboard dimensions or cleaner chessboard image.");
		std::ostringstream strs;
		strs << "Corners expected = " << numberOfCorners << ", but corners found = " << corners.size() << ".";
		std::string output_str = strs.str();
		qDebug("%s", output_str.c_str());
		qDebug("=======================");
	} else {
		qDebug("Lines calculated. Sub-pixel corner points:");
		for (vector<Point2f>::iterator i = corners.begin(); i != corners.end(); i++){
			std::ostringstream strs;
			strs << "   Corner found: <" << (i->x) << "," << (i->y) << "> ";
			std::string output_str = strs.str();
			qDebug("%s", output_str.c_str());
		}
		// The following assumes that points from the chessboard are stored left-to-right, top-to-bottom.
		// This should be guaranteed by opencv.
		unsigned int lineLength = (squares_across-1);
		unsigned int lineHeight = (squares_down-1);
		ofstream outFile("line_segment_output.dat", ios::out);
		Point2f* points = &corners[0];
		int numberOfLines = lineLength+lineHeight;
		outFile << numberOfLines << "\n";
		for (unsigned int i = 0; i < numberOfCorners/lineLength; i++ ){ // horizontal lines
			outFile << lineLength << "\n";
			for (unsigned int j = 0; j < lineLength; j++ ){
				outFile << points[i*lineLength + j].x << " " << points[i*lineLength + j].y << "\n";
			}
		}
		for (unsigned int i = 0; i < numberOfCorners/lineHeight; i++ ){ // vertical lines
			outFile << lineHeight << "\n";
			for (unsigned int j = 0; j < lineHeight; j++ ){
				outFile << points[j*lineLength + i].x << " " << points[j*lineLength + i].y << "\n";
			}
		}
		outFile.close();

		// Since stage 1 was successful, progress to stage 2:
		// The actual distortion removal.

		std::string str_name = input_image_filename.toStdString();
		const char * input_image_name = (char*)str_name.c_str();
		const char * arguments[] = { "lens_distortion_estimation",
			input_image_name,
			"output_undistorted_image.tif",
			"line_segment_output.dat",
			"output_lens_distortion_models.dat" };
		LensDistortionEstimation lde = LensDistortionEstimation(5, arguments);
		qDebug("Distortion removed from image; corrected image stored in 'output_undistorted_image.tif'.");
	}

	// What is happening here?
	// 1. We use opencv to automatically extract corners from a distorted chessboard image.
	// 2. We reconstruct segments that should be straight lines from these points, using a naive algorithm
	//		which relies on the fact that opencv finds corners in a set order; the algorithm only works
	//		if all the internal points are found.
	// 3. For optimal distortion removal, we use use multiple lines, both vertical and horizontal.
	// 4. The actual distortion removal consists of first modelling the distortion (by determining
	//		the radial distortion coefficients) and then correcting the image based on that model.
	//		This is done using an open-source ANSI C library which uses an optimizer to determine
	//		the undistorted coefficients by minimizing the error between the radial distortion model
	//		and the image data.
	// 5. Note that this usage of the algorithm only works within certain constraints: only on
	//		relatively 'clean' images of [distorted] chessboards (due to opencv), and only on
	//		uncompressed .tif images (due to the library we refer to as IPOL_distort).
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
