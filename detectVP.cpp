#include "detectVP.h"

using namespace std;
using namespace cv;

vector<string> FindAllFile(const char* dir, bool ishavedir=false)
{
	_finddata_t file;
	vector<string> file_list;
	long lf;
	if((lf = _findfirst(dir,&file))==-1L){
		return file_list;
	}else{
		do{
			if(ishavedir){
				string tmppath = dir;
				int index = tmppath.find_last_of("*.")-1;
				tmppath = tmppath.substr(0,index)+file.name;
				file_list.push_back(tmppath);
			}else{
				file_list.push_back(file.name);
			}
		}while(_findnext(lf,&file)==0);
		_findclose(lf);
		return file_list;
	}

}


int computeVPDirection(Mat outputImg, vector<Mat> vps)
{
	int flag = 0;
	for(unsigned int vpNum=0; vpNum < vps.size(); vpNum++)
	{
		if(vps[vpNum].at<float>(2,0) != 0)
		{
			Point2f vp(vps[vpNum].at<float>(0,0), vps[vpNum].at<float>(1,0));
			int v = vpNum;

			double vpNorm = cv::norm(vps[v]);
			if(fabs(vpNorm - 1) < 0.001)
			{
				if (vpNum == vps.size()-1)
				{
					flag = 0;//????ʧ??
					//string saveName = saveDir + "noVP\\" + imgName;
					//imwrite(saveName, outputImg);
					return flag;
				}
				else
					continue;
			}
			else
			{
				if (vps[v].at<float>(0,0) < 0)//left
				{
					flag = 1;
					//string saveName = saveDir + "left\\" + imgName;
					//imwrite(saveName, outputImg);
					return flag;
				}
				else if (vps[v].at<float>(0,0) > outputImg.cols-1)//right
				{
					flag = 2;
					//string saveName = saveDir + "right\\" + imgName;
					//imwrite(saveName, outputImg);
					return flag;
				}
				else//middle
				{
					if (vpNum == vps.size()-1)
					{
						flag = 0;
						//string saveName = saveDir + "middle\\" + imgName;
						//imwrite(saveName, outputImg);
						return flag;
					}
					else
						continue;
				}
			}
		}
	}
}

void ConvertGray(IplImage *img, double *pOutImg) 
{  
	int nHeight=img->height; 
	int nWidth=img->width; 
	int step=img->widthStep/sizeof(uchar); 
	int channels=img->nChannels; 
	uchar* data=(uchar*)img->imageData;     
	int vB, vG, vR; 

	for (int i = 0; i < nHeight; i++) 
	{ 
		for (int j = 0; j < nWidth;j++) 
		{
			vB=data[i*step+j*channels+0]; 
			vG=data[i*step+j*channels+1]; 
			vR=data[i*step+j*channels+2];                   
			*pOutImg++= RGB2GRAY(vR,vG,vB);
		} 
	} 
}

/** This function contains the actions performed for each image*/
std::vector<cv::Mat> processImage(MSAC &msac, int numVps, cv::Mat &img, cv::Mat &outputImg)
{
	double * image;
	double * out;
	int x,y,i,j,n;

	vector<vector<cv::Point> > lineSegments;
	vector<cv::Point> aux;

	int X = img.cols;
	int Y = img.rows;

	/* create a simple image: left half black, right half gray */
	image = (double *) malloc( X * Y * sizeof(double) );
	if( image == NULL )
	{
		fprintf(stderr,"error: not enough memory\n");
		exit(EXIT_FAILURE);
	}

	IplImage img_IPL = img;
	ConvertGray(&img_IPL,image);

	/* LSD call */
	out = lsd(&n,image,X,Y);

	for(size_t i=0; i<n; i++)
	{		
		Point pt1, pt2;
		pt1.x = out[7*i+0];
		pt1.y = out[7*i+1];
		pt2.x = out[7*i+2];
		pt2.y = out[7*i+3];

		//去除竖直线
		double k = ((double)pt2.y - pt1.y)/(pt2.x - pt1.x);
		if(k < -1 || k > 1)
		{
			continue;
		}
		line(outputImg, pt1, pt2, CV_RGB(0,0,0), 2);
		/*circle(outputImg, pt1, 2, CV_RGB(255,255,255), CV_FILLED);
		circle(outputImg, pt1, 3, CV_RGB(0,0,0),1);
		circle(outputImg, pt2, 2, CV_RGB(255,255,255), CV_FILLED);
		circle(outputImg, pt2, 3, CV_RGB(0,0,0),1);*/

		// Store into vector of pairs of Points for msac
		aux.clear();
		aux.push_back(pt1);
		aux.push_back(pt2);
		lineSegments.push_back(aux);
	}

	// Multiple vanishing points
	std::vector<cv::Mat> vps;			// vector of vps: vps[vpNum], with vpNum=0...numDetectedVps
	std::vector<std::vector<int> > CS;	// index of Consensus Set for all vps: CS[vpNum] is a vector containing indexes of lineSegments belonging to Consensus Set of vp numVp
	std::vector<int> numInliers;

	std::vector<std::vector<std::vector<cv::Point> > > lineSegmentsClusters;
	
	// Call msac function for multiple vanishing point estimation
	msac.multipleVPEstimation(lineSegments, lineSegmentsClusters, numInliers, vps, numVps);

	// Draw line segments according to their cluster
	msac.drawCS(outputImg, lineSegmentsClusters, vps);
	
	free( (void *) image );
	free( (void *) out );

	return vps;
}

int saveImgByVP(string saveDir, string imgName, Mat outputImg, vector<Mat> vps)
{
	int flag = 0;
	for(unsigned int vpNum=0; vpNum < vps.size(); vpNum++)
	{
		if(vps[vpNum].at<float>(2,0) != 0)
		{
			Point2f vp(vps[vpNum].at<float>(0,0), vps[vpNum].at<float>(1,0));
			int v = vpNum;

			double vpNorm = cv::norm(vps[v]);
			if(fabs(vpNorm - 1) < 0.001)
			{
				if (vpNum == vps.size()-1)
				{
					flag = 2;//无消失点
					string saveName = saveDir + "noVP\\" + imgName;
					imwrite(saveName, outputImg);
					return flag;
				}
				else
					continue;
			}
			else
			{
				if (vps[v].at<float>(0,0) < 0)//left
				{
					flag = -1;
					string saveName = saveDir + "left\\" + imgName;
					imwrite(saveName, outputImg);
					return flag;
				}
				else if (vps[v].at<float>(0,0) > outputImg.cols-1)//right
				{
					flag = 1;
					string saveName = saveDir + "right\\" + imgName;
					imwrite(saveName, outputImg);
					return flag;
				}
				else//middle
				{
					if (vpNum == vps.size()-1)
					{
						flag = 0;
						string saveName = saveDir + "middle\\" + imgName;
						imwrite(saveName, outputImg);
						return flag;
					}
					else
						continue;
				}
			}
		}
	}
}
