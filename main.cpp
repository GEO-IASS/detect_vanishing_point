#include "detectVP.h"

using namespace cv;
using namespace std;

int detectVanishingPoint(char* dirAndName, char* imgName)
{		
	string ins = (string)dirAndName;
	Mat inputImg = imread(ins);
	if (!inputImg.data)
	{
		return -1;
	}

	int procWidth = -1;
	int procHeight = -1;
	cv::Size procSize;

	int height = inputImg.rows;
	int width = inputImg.cols;

	// Resize	
	if(procWidth != -1)
	{

		procHeight = height*((double)procWidth/width);
		procSize = cv::Size(procWidth, procHeight);

		printf("Resize to: (%d x %d)\n", procWidth, procHeight);	
	}
	else
		procSize = cv::Size(width, height);

	// Create and init MSAC
	int mode = MODE_NIETO;
	bool verbose = false;
	MSAC msac;
	msac.init(mode, procSize, verbose);	


	// Resize to processing size
	cv::resize(inputImg, inputImg, procSize);		

	// Color Conversion
	Mat imgGRAY; 
	Mat outputImg;
	if(inputImg.channels() == 3)
	{
		cv::cvtColor(inputImg, imgGRAY, CV_BGR2GRAY);	
		inputImg.copyTo(outputImg);
	}
	else
	{
		inputImg.copyTo(imgGRAY);
		cv::cvtColor(inputImg, outputImg, CV_GRAY2BGR);
	}

	int numVps = 3;
	Rect r(0,0, imgGRAY.cols, imgGRAY.rows*0.7);
	Mat srcROI( imgGRAY,r );
	Mat srcROIR(imgGRAY.rows*0.7, imgGRAY.cols, srcROI.type());
	resize(srcROI, srcROIR, Size(srcROIR.cols, srcROIR.rows));
	std::vector<cv::Mat> vps = processImage(msac, numVps, srcROIR, outputImg);

	int flag = computeVPDirection(outputImg, vps);
	return flag;	
}

int main()
{
	char* dirAndName = "D:\\pic\\ori\\ori\\3b1ba92a3bec2c11be0a77dd9d6a9337.jpg";
	char* imgName = "3b1ba92a3bec2c11be0a77dd9d6a9337.jpg";
	detectVanishingPoint(dirAndName, imgName);
	return 1;
}