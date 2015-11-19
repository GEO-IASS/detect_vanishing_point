#include "rectify.h"

void readLabelFile1(const char* labelFileName, vector<string>& imgNames,
	vector<vector<Rect>>& rectInImgs, vector<vector<int>>& wordCataNum)
{
	ifstream infile(labelFileName);
	if(!infile)
	{
		cout << "open error!" << endl; 
		return;
	}

	int lineNum = 0;
	char dstBuf[1024];
	memset(dstBuf,0,1024);
	while(!infile.eof()/* && lineNum > 0*/)
	{
		++lineNum;
		string str;
		getline(infile, str);
		if(str.size() < 5)
		{
			continue;
		}
		//图片名
		int ind1 = str.find_first_of(" ");
		string oneImgName = str.substr(0, ind1);
		//String_UTF_8toANSI(oneImgName.c_str(),dstBuf);
		//oneImgName = dstBuf;

		vector<Rect> rectInOneImg;
		vector<int> wordCataNumInOneImg;
		//框个数:文件名后第一个数字
		string str_cutN = str.substr(ind1+1, str.size() - ind1);
		int ind2 = str_cutN.find_first_of(" ");
		int rectNum = atoi((str_cutN.substr(0, ind2)).c_str());

		//框个数:文件名后第二个数字
		//string str_cutN1 = str.substr(ind1+1, str.size() - ind1);
		//int indA = str_cutN1.find_first_of(" ");
		//string str_cutN = str_cutN1.substr(indA+1, str.size() - indA);
		//int ind2 = str_cutN.find_first_of(" ");
		//int rectNum = atoi((str_cutN.substr(0, ind2)).c_str());

		//框坐标:rect中为（y, x, height, width）
		string str_c = str_cutN.substr(ind2, str.size()-ind2+1);
		int str_c_i = 0;		
		for (int i = 0; i <rectNum; ++i)
		{
			int space_i = 0;
			int lastSpaceNum = str_c_i;
			++str_c_i;
			vector<int> rectPara;
			if (i == rectNum-1)
			{
				while(space_i < 4)
				{
					if (str_c[str_c_i] == ' ')
					{					
						string temp = str_cutN.substr(ind2+lastSpaceNum, str_c_i-lastSpaceNum);
						rectPara.push_back(atoi(temp.c_str()));
						lastSpaceNum = str_c_i;					
						++space_i;
					}
					++str_c_i;
				}
				string temp = str_c.substr(lastSpaceNum+1, str_c.size()-lastSpaceNum-1);
				rectPara.push_back(atoi(temp.c_str()));
			}
			else
			{
				while(space_i < 5)
				{
					if (str_c[str_c_i] == ' ')
					{					
						string temp = str_cutN.substr(ind2+lastSpaceNum, str_c_i-lastSpaceNum);
						rectPara.push_back(atoi(temp.c_str()));
						lastSpaceNum = str_c_i;					
						++space_i;
					}
					++str_c_i;
				}
			}
			Rect oneRect;
			oneRect.y = rectPara[1];
			oneRect.x = rectPara[2];
			oneRect.height = rectPara[3];
			oneRect.width = rectPara[4];
			rectInOneImg.push_back(oneRect);
			wordCataNumInOneImg.push_back(rectPara[0]);
		}
		imgNames.push_back(oneImgName);
		rectInImgs.push_back(rectInOneImg);
		wordCataNum.push_back(wordCataNumInOneImg);
	}
	//string temp = imgNames[0].substr(1, imgNames.size()-1);
	//imgNames[0] = temp;
}


//flag_rect 1:矫正  2：消失点问题导致无矫正(图外消失点)  3：扩展图太大导致无矫正  4：消失点问题导致无矫正（图内消失点）
void rectifyImgAccordingtoVP(Mat inputImg, Mat& rectifyImg, vector<Mat> vanishP, Mat& transform, int& flag_rect)
{
	//暂时用原图尺寸作为矫正图尺寸，以后根据消失点计算校正图尺寸
	vector<Point2f> points_rect;//lu,ld,ru,rd
	points_rect.push_back(Point2f(0,0));
	points_rect.push_back(Point2f(0, inputImg.rows-1));
	points_rect.push_back(Point2f(inputImg.cols-1, 0));
	points_rect.push_back(Point2f(inputImg.cols-1, inputImg.rows-1));

	Mat imgE;
	vector<Point2f> corners(4);
	for (int i = 0; i < vanishP.size(); ++i)
	{		
		if (vanishP[i].at<float>(0,0) < 0 /* && gradients[i][0]*gradients[i][1] < 0*/)//消失点在左侧
		{
			float ku = (vanishP[i].at<float>(1, 0)- points_rect[0].y) / (vanishP[i].at<float>(0, 0) - points_rect[0].x);
			float kd = (vanishP[i].at<float>(1, 0) - points_rect[1].y) / (vanishP[i].at<float>(0, 0) - points_rect[1].x);

			if (ku >= (float)inputImg.rows/inputImg.cols || kd <= (float)inputImg.rows/inputImg.cols*(-1))
			{
				if (i == vanishP.size()-1)//没有校正
				{
					transform = getPerspectiveTransform(points_rect,points_rect);
					flag_rect = 2;
					rectifyImg = inputImg.clone();
					return;
				}
				else
					continue;		
			}

			float bu = points_rect[0].y-ku*points_rect[0].x;
			float bd = points_rect[1].y-kd*points_rect[1].x;

			points_rect[2].y = (float)cvRound(ku*points_rect[2].x+bu);
			points_rect[3].y = (float)cvRound(kd*points_rect[3].x+bd);

			//扩大图
			if (points_rect[3].y-points_rect[2].y+1 > 10000)//扩展图太大
			{
				transform = getPerspectiveTransform(points_rect,points_rect);
				flag_rect = 3;
				rectifyImg = inputImg.clone();
				return;
			}
			imgE = Mat::zeros(points_rect[3].y-points_rect[2].y+1, inputImg.cols, inputImg.type());
			int cutStartY = (points_rect[0].y > points_rect[2].y) ? points_rect[0].y : points_rect[2].y;
			int cutEndY = (points_rect[1].y < points_rect[3].y) ? points_rect[1].y : points_rect[3].y;
			inputImg(Rect(0, cutStartY, inputImg.cols, cutEndY-cutStartY+1)).copyTo(imgE(Rect(0, abs(points_rect[2].y), inputImg.cols, cutEndY-cutStartY+1)));
			for(int i = 0; i < points_rect.size(); ++i)
			{
				corners[i] = Point2f(points_rect[i].x, points_rect[i].y-points_rect[2].y);
			}
			break;
		}
		else if (vanishP[i].at<float>(0,0) > inputImg.cols)//消失点在右侧
		{
			float ku = (vanishP[i].at<float>(1,0) - points_rect[2].y) / (vanishP[i].at<float>(0,0) - points_rect[2].x);
			float kd = (vanishP[i].at<float>(1,0) - points_rect[3].y) / (vanishP[i].at<float>(0,0) - points_rect[3].x);

			if (ku >= (float)inputImg.rows/inputImg.cols || kd <= (float)inputImg.rows/inputImg.cols*(-1))
			{
				if (i == vanishP.size()-1)
				{
					transform = getPerspectiveTransform(points_rect,points_rect);
					flag_rect = 2;
					rectifyImg = inputImg.clone();
					return;
				}
				else
					continue;
			}

			float bu = points_rect[2].y-ku*points_rect[2].x;
			float bd = points_rect[3].y-kd*points_rect[3].x;

			points_rect[0].y = (float)cvRound(ku*points_rect[0].x+bu);
			points_rect[1].y = (float)cvRound(kd*points_rect[1].x+bd);

			//扩大图
			if (points_rect[1].y-points_rect[0].y+1 > 10000)//扩展图太大
			{
				transform = getPerspectiveTransform(points_rect,points_rect);
				flag_rect = 3;
				rectifyImg = inputImg.clone();
				return;
			}

			imgE = Mat::zeros(points_rect[1].y-points_rect[0].y+1, inputImg.cols, inputImg.type());
			int cutStartY = (points_rect[0].y > points_rect[2].y) ? points_rect[0].y : points_rect[2].y;
			int cutEndY = (points_rect[1].y < points_rect[3].y) ? points_rect[1].y : points_rect[3].y;
			inputImg(Rect(0, cutStartY, inputImg.cols, cutEndY-cutStartY+1)).copyTo(imgE(Rect(0, abs(points_rect[0].y), inputImg.cols, cutEndY-cutStartY+1)));

			for(int i = 0; i < points_rect.size(); ++i)
			{
				corners[i] = Point2f(points_rect[i].x, points_rect[i].y-points_rect[0].y);
			}
			break;
		}
		else//无需矫正
		{
			if (i == vanishP.size()-1)
			{
				transform = getPerspectiveTransform(points_rect,points_rect);
				flag_rect = 4;
				rectifyImg = inputImg.clone();
				return;
			}
			else
				continue;			
		}
	}
	line(imgE, Point(corners[0].x, corners[0].y+1),  Point(corners[2].x, corners[2].y+1), Scalar(0,0,255), 2, CV_AA);
	line(imgE, Point(corners[1].x, corners[1].y-1),  Point(corners[3].x, corners[3].y-1), Scalar(0,0,255), 2, CV_AA);

	vector<Point2f> corners_trans(4);  
	corners_trans[0] = Point2f(0,0); //ru
	corners_trans[1] = Point2f(0,inputImg.rows-1);  //rd
	corners_trans[2] = Point2f(inputImg.cols-1, 0);  //lu
	corners_trans[3] = Point2f(inputImg.cols-1, inputImg.rows-1);  //ld

	transform = getPerspectiveTransform(corners,corners_trans);  
	//cout<<transform<<endl;  

	rectifyImg = Mat::zeros(inputImg.rows,inputImg.cols,CV_8UC3); 
	warpPerspective(imgE, rectifyImg, transform, rectifyImg.size());
}