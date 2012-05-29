#ifndef OUR_PCA_TRAINING_PROCESSOR_H
#define OUR_PCA_TRAINING_PROCESSOR_H

#include "processor.h"
#include <highgui.h>
#include <QtCore/QFileInfo>

using namespace cv;
using namespace std;


struct class_of_training_images {
	std::string identifier; // Any string used in the input txt file to identify them
	std::vector<Mat> images; // In original 2D form, not vector form
};


class PcaTrainingProcessor : public Processor
{
	Q_OBJECT

		Q_PROPERTY(QFileInfo FileList READ fileList WRITE setFileList USER true)
		Q_CLASSINFO("FileList", "filetype=text;")
		Q_PROPERTY(int NumComponentsToKeep READ numComponentsToKeep WRITE setNumComponentsToKeep USER true)

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

	Mat convertImageToVector(Mat img);
	Mat convertVectorToImage(Mat vec);

	Mat trainingSetImages;
	int numImages;
	int pixelsPerImage;

	std::vector<class_of_training_images> classesOfTrainingImages; // Number of elements = number of classes
	// Each element stores the name for that class and all training images in that class

	int pcaImageWidth;
	int pcaImageHeight;

	int numCompsToKeep;
	float numComponentsToKeep() {QMutexLocker l(&mutex); return numCompsToKeep;}
	void setNumComponentsToKeep(int num);

	QFileInfo file_list;

};

#endif // OUR_PCA_TRAINING_PROCESSOR_H
