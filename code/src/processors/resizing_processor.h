#ifndef RESIZING_PROCESSOR_H
#define RESIZING_PROCESSOR_H

#include "processor.h"
#include <highgui.h>
#include <QtCore/QFileInfo>

using namespace cv;
using namespace std;

class ResizingProcessor : public Processor
{
	Q_OBJECT

		Q_PROPERTY(double Factor READ getFactor WRITE setFactor USER true)

public:
	ResizingProcessor(QObject *parent = 0);
	~ResizingProcessor();

	QString name() {return "Image resizing";}

	void run();

private:

	double factor;
	double getFactor() {QMutexLocker l(&mutex); return factor;}
	void setFactor(double fac);
	Mat resizeImage();

};

#endif // RESIZING_PROCESSOR_H
