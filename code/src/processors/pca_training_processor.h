#ifndef OUR_PCA_TRAINING_PROCESSOR_H
#define OUR_PCA_TRAINING_PROCESSOR_H

#include "processor.h"
#include <highgui.h>
#include <QtCore/QFileInfo>

using namespace cv;
using namespace std;

class PcaTrainingProcessor : public Processor
{
	Q_OBJECT

		Q_PROPERTY(QFileInfo FileList READ fileList WRITE setFileList USER true)
		Q_CLASSINFO("FileList", "filetype=text;")

public:
	PcaTrainingProcessor(QObject *parent = 0);
	~PcaTrainingProcessor();

	QString name() {return "PCA training";}

	QFileInfo fileList() {QMutexLocker l(&mutex); return file_list;}
	void setFileList(QFileInfo path);

	bool PCATrain();
	void run();

	//	void testProgram(double smoothWeight, int mult, const char * lOut, const char * rOut, const char * lIn, const char * rIn);
	//	double testStereoResults(const char * testImageName, const char * idealImageName);

private:

	bool loadImages();

	Mat trainingSetImages;
	int numImages;
	int pixelsPerImage;

	QFileInfo file_list;

};

#endif // OUR_PCA_TRAINING_PROCESSOR_H
