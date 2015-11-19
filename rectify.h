#include "cv.h"      
#include "highgui.h" 
#include "cxcore.h"  

#include <vector>
#include <iostream>
#include <fstream>
#include <io.h>

using namespace std;
using namespace cv;

void rectifyImgAccordingtoVP(Mat inputImg, Mat& rectifyImg, vector<Mat> vanishP, Mat& transform, int& flag_rect);
void readLabelFile1(const char* labelFileName, vector<string>& imgNames,
	vector<vector<Rect>>& rectInImgs, vector<vector<int>>& wordCataNum);