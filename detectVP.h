#ifdef WIN32
#include <windows.h>
#endif
#include <iostream>
#ifdef linux
#include <stdio.h>
#endif

#define USE_PPHT
#define MAX_NUM_LINES	200

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "MSAC.h"
#include "rectify.h"
#include "lsd.h"

std::vector<cv::Mat> processImage(MSAC &msac, int numVps, cv::Mat &img, cv::Mat &outputImg);
vector<string> FindAllFile(const char* dir, bool ishavedir);
int saveImgByVP(string saveDir, string imgName, Mat outputImg, vector<Mat> vps);
int computeVPDirection(Mat outputImg, vector<Mat> vps);