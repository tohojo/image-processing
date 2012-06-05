#include "resizing_processor.h"
#include "util.h"

#include <QDebug>

ResizingProcessor::ResizingProcessor(QObject *parent)
: Processor(parent)
{
	factor = 1.0;
        uses_colour = true;
}


ResizingProcessor::~ResizingProcessor()
{
}


void ResizingProcessor::run()
{

	// Please ignore this terrible hack :P
	/*
	factor = 0.25;
	for (int i = 4071; i <= 4114; i++){
		std::string left = "norm_0.25_1.2/DSCF";
		std::string right = "norm_0.25_1.2/DSCF";
		std::stringstream ss;
		ss << i;
		left.append(ss.str());
		right.append(ss.str());
		std::stringstream leftout;
		leftout << left;
		leftout << "rec_l.normal_smallD_smth.png";
		std::stringstream rightout;
		rightout << right;
		rightout << "rec_r.normal_smallD_smth.png";
		left.append("rec_l.normalD_smth.png");
		right.append("rec_r.normalD_smth.png");
		{
			Mat img = imread(left.c_str());
			Mat resized;
			double colsX = (img.cols * factor);
			if ((colsX - (int)(colsX)) > 0.5) colsX += 0.5;
			double rowsX = (img.rows * factor);
			if ((rowsX - (int)(rowsX)) > 0.5) rowsX += 0.5;
			Size sizeFactor = Size((int)colsX, (int)rowsX); // Width, height
			cv::resize(img, resized, sizeFactor);
			imwrite(leftout.str().c_str(), resized);
		}
		{
			Mat img = imread(right.c_str());
			Mat resized;
			double colsX = (img.cols * factor);
			if ((colsX - (int)(colsX)) > 0.5) colsX += 0.5;
			double rowsX = (img.rows * factor);
			if ((rowsX - (int)(rowsX)) > 0.5) rowsX += 0.5;
			Size sizeFactor = Size((int)colsX, (int)rowsX); // Width, height
			cv::resize(img, resized, sizeFactor);
			imwrite(rightout.str().c_str(), resized);
		}
	}*/

	forever {
		if(abort) return;
		emit progress(0);

		Mat result = resizeImage();
		if (! result.empty()){
			mutex.lock();
			output_image = result;
			qDebug() << "Finished resizing.\n";
		} else {
			//
		}
		emit progress(100);
		emit updated();
		if(once) return;

		if(!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}
}

Mat ResizingProcessor::resizeImage(){
	if (input_image.empty()) return Mat();
	Mat resized;
	double colsX = (input_image.cols * factor);
	if ((colsX - (int)(colsX)) > 0.5) colsX += 0.5;
	double rowsX = (input_image.rows * factor);
	if ((rowsX - (int)(rowsX)) > 0.5) rowsX += 0.5;
	Size sizeFactor = Size((int)colsX, (int)rowsX); // Width, height
	cv::resize(input_image, resized, sizeFactor);
	return resized;
}

void ResizingProcessor::setFactor(double fac)
{
	QMutexLocker locker(&mutex);
	factor = fac;
	mutex.unlock();
	Processor::process();
}


