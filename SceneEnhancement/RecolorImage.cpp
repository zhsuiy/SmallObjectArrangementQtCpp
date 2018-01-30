#include "RecolorImage.h"

using namespace cv;
#define max_uchar(a, b)    (((a) > (b)) ? (a) : (b))  
#define min_uchar(a, b)    (((a) < (b)) ? (a) : (b))  

QImage Utility::RecolorQImage(QString &filename, QColor& color)
{
	int r, g, b;
	color.getRgb(&r, &g, &b);
	Mat tar(1, 1, CV_8UC3, Scalar(b,g,r));
	cv::Mat tar_lab; // make the same cv::Mat
	cvtColor(tar, tar_lab, CV_BGR2Lab); // cvtColor Makes a copt, that what i need
	uchar *tardata = tar_lab.data;
	float L, A, B;
	L = tardata[0];
	A = tardata[1];
	B = tardata[2];

	cv::Mat src = cv::imread(filename.toStdString(), 3);	
	Mat dst;
	//int hue, sat, val;
	
	//color.getHsv(&hue, &sat, &val);
	
	Vec3f lab;
	lab[0] = L; lab[1] = A; lab[2] = B;
	ColorTransfer(src, lab, dst);


	// convert dst to QImage
	cv::Mat temp; // make the same cv::Mat
	cvtColor(dst, temp, CV_BGR2RGB); // cvtColor Makes a copt, that what i need
	QImage dest((const uchar *)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	dest.bits(); // enforce deep copy, see documentation 
				 // of QImage::QImage ( const uchar * data, int width, int height, Format format )
	
	return dest;
}

// 计算彩色图像均值和标准差  
void Utility::CompMeanAndVariance(Mat &img, Vec3f &mean3f)
{
	int row = img.rows;
	int col = img.cols;
	int total = row * col;
	float sum[3] = { 0.0f };
	// 均值  
	uchar *pImg = img.data;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			sum[0] += pImg[3 * j + 0];
			sum[1] += pImg[3 * j + 1];
			sum[2] += pImg[3 * j + 2];
		}
		pImg += img.step;
	}

	mean3f[0] = sum[0] / total;
	mean3f[1] = sum[1] / total;
	mean3f[2] = sum[2] / total;
}


// 颜色转换  
void Utility::ColorTransfer(Mat &src, Vec3f &tar_color, Mat &dst)
{
	Mat srcLab, tarHsv;
	Vec3f srcMean3f, tarMean3f;// 源/目标图像均值

						  // BGR空间转Lab空间  
	cvtColor(src, srcLab, CV_BGR2Lab);

	// 计算当前图像与目标图像均值及标准差  
	CompMeanAndVariance(srcLab, srcMean3f);

	// 计算颜色转换值  
	int row = srcLab.rows;
	int col = srcLab.cols;
	uchar *pImg = srcLab.data;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{			
			pImg[3 * j + 0] = (uchar)min_uchar(255, max_uchar(0, pImg[3 * j + 0] + tar_color[0] - srcMean3f[0]));
			pImg[3 * j + 1] = (uchar)min_uchar(255, max_uchar(0, pImg[3 * j + 1] + tar_color[1] - srcMean3f[1]));
			pImg[3 * j + 2] = (uchar)min_uchar(255, max_uchar(0, pImg[3 * j + 2] + tar_color[2] - srcMean3f[2]));
		}
		pImg += srcLab.step;
	}

	// Lab空间转BGR空间  
	cvtColor(srcLab, dst, CV_Lab2BGR);
}