#ifndef DISTORTION_H
#define DISTORTION_H

#include "processor.h"
#include <cv.h>

using namespace cv;
using namespace std;

class DistortionRemoval : public Processor
{
	Q_OBJECT

		Q_PROPERTY(int SquaresAcross READ squaresAcross WRITE setSquaresAcross USER true)
		Q_PROPERTY(int SquaresDown READ squaresDown WRITE setSquaresDown USER true)

public:
	DistortionRemoval(QObject *parent = 0);
	~DistortionRemoval();

	QString name() {return "Distortion removal";}

	int squaresAcross() {QMutexLocker locker(&mutex); return squares_across;}
	int squaresDown() {QMutexLocker locker(&mutex); return squares_down;}
	void setSquaresAcross(const int squares);
	void setSquaresDown(const int squares);

private:
	void run();
	void calculateLines();
	int squares_across;
	int squares_down;
};

#endif
