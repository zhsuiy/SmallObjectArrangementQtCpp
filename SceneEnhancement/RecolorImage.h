#pragma once
#include "Utility.h"
#include <cv.h>
#include <opencv2\opencv.hpp>
using namespace cv;
namespace Utility
{

	// image processing
	QImage RecolorQImage(QString &filename, QColor &color);
	void CompMeanAndVariance(Mat &img, Vec3f &mean3f);
	void ColorTransfer(Mat &src, Vec3f &color, Mat &dst);

}