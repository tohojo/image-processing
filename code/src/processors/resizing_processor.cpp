#include "resizing_processor.h"
#include "util.h"

#include <QDebug>

ResizingProcessor::ResizingProcessor(QObject *parent)
: Processor(parent)
{
	factor = 1.0;
}


ResizingProcessor::~ResizingProcessor()
{
}


void ResizingProcessor::set_input(const Mat img){
	// Overwrites the greyscale image loader.
	// Unfortunately, this means the image will be loaded twice:
	// once in "ProcessingGUI::load_image(QString filename)" and then immediately again, in colour,
	// when it calls "current_processor->set_input(input_image)" - i.e. this function
	QMutexLocker locker(&mutex);
	input_image = Util::load_image_colour(input_image_filename);
}


void ResizingProcessor::run()
{

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


