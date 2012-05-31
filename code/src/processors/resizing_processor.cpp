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


