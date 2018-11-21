#ifndef __PIX2CAM_H__
#define __PIX2CAM_H__
#include <iostream>
#include <string>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "properties.h"
using namespace std;
using namespace cv;
using namespace jp;

static IplImage * loadDepth(string a_name)
{
	ifstream l_file(a_name.c_str(), ofstream::in | ofstream::binary);
	if (l_file.fail() == true)
	{
		printf("cv_load_depth: could not open file for writing!\n");
		return NULL;
	}
	int l_row;
	int l_col;
	l_file.read((char*)&l_row, sizeof(l_row));
	l_file.read((char*)&l_col, sizeof(l_col));
	IplImage * lp_image = cvCreateImage(cvSize(l_col, l_row), IPL_DEPTH_16U, 1);
	for (int l_r = 0; l_r<l_row; ++l_r)
	{
		for (int l_c = 0; l_c<l_col; ++l_c)
		{
			l_file.read((char*)&CV_IMAGE_ELEM(lp_image, unsigned short, l_r, l_c), sizeof(unsigned short));
		}
	}
	l_file.close();
	return lp_image;
}
static void getDepth(string path, img_depth_t &img)
{
	
	IplImage* depImg = loadDepth(path);
	Mat depth(depImg,true);
	img = (img_depth_t &)depth;//[mm]
}

static coord3_t pxToEye(int x, int y, depth_t depth)
{
	coord3_t eye;

	if (depth == 0) // depth hole -> no camera coordinate
	{
		eye(0) = 0;
		eye(1) = 0;
		eye(2) = 0; 
		return eye;
	}

	GlobalProperties* gp = GlobalProperties::getInstance();

	eye(0) = (short)((x - (gp->fP.imageWidth / 2.f + gp->fP.xShift)) / (gp->fP.focalLength / depth));
	eye(1) = (short)-((y - (gp->fP.imageHeight / 2.f + gp->fP.yShift)) / (gp->fP.focalLength / depth));
	eye(2) = (short)-depth; // camera looks in negative z direction
	return eye;
}

void getEye(string path, img_coord_t& img)
{
	img_depth_t imgDepth;
	getDepth(path, imgDepth);
	img = img_coord_t(imgDepth.rows, imgDepth.cols);
	for (int x = 0; x < img.cols; x++)
		for (int y = 0; y < img.rows; y++)
		{
			img(y, x) = pxToEye(x, y, imgDepth(y, x));
		}
}
#endif
