#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "properties.h"
#include "pix2cam.h"
#include "convrt.h"
#include "getfilename.h"

using namespace std;
using namespace cv;
using namespace jp;


int main()
{
	string rotpath = "E:\\Datasets\\Eric2016\\ape\\result\\rot";//rot文件路径rot143.rot
	string trapath = "E:\\Datasets\\Eric2016\\ape\\result\\tra";//tra文件路径tra143.tra
	string segpath = "E:\\Datasets\\Eric2016\\ape\\result\\seg";//seg文件路径ape.obj143.png
	string segbinpath = "E:\\Datasets\\Eric2016\\ape\\result\\seg_bin";
	string depthpath = "E:\\Datasets\\Eric2016\\ape\\result\\depth";//depth文件路径depth143.dpt
	string objpath = "E:\\Datasets\\Eric2016\\ape\\result\\obj";//存obj文件的路径
	string plypath = "E:\\Datasets\\Eric2016\\ape\\mesh.ply";//ply文件

	string colordir = "E:\\Datasets\\Eric2016\\ape\\result\\rgb_noseg";
	vector<string> colorfiles;
	getFiles(colordir, colorfiles);//获取colorpa路径下的所有文件名
	for (int i = 0; i < colorfiles.size(); i++)//每次取一张图片出来处理
	{
		string colorpath = colorfiles.at(i);
		int num[2];//num[1]为图片序号--不同路径可能不同
		getNumfromString(colorpath, num);
		int serial =num[1];
		Mat segimg;
		Mat colorimg;
		segimg = imread(segpath +"\\ape.obj" + to_string(serial) + ".png");
		colorimg = imread(colorpath);
		if (segimg.empty()) { cout << "read segimg error" << endl; return -1; }
		if (colorimg.empty()) { cout << "read colorimg error" << endl; return -1; }
		//resize segimg
		resize(segimg, segimg, colorimg.size());
		//二值化
		Mat binimg;
		cvtColor(segimg,segimg, COLOR_RGB2GRAY);//灰度
		threshold(segimg, binimg, 1, 255, CV_THRESH_BINARY);//二值化
		imwrite(segbinpath + "\\segbin" + to_string(serial) + ".png", binimg);
		//取出原图像素计算，seg图像对比
		int cols = colorimg.cols;
		int rows = colorimg.rows;
		if (cols != segimg.cols || rows != segimg.rows){ cout << "different size" << endl; return -1; }
		
		//转换旋转矩阵和平移矩阵的坐标系
		Matx31f tvec;
		Matx33f rmat;
		getLinemod_t(trapath + "\\tra" + to_string(serial) + ".tra", tvec);//获取t矩阵[cm]
		getLinemod_R(rotpath + "\\rot" + to_string(serial) + ".rot", rmat);//获取R矩阵
		convLine2brachman(rmat, tvec, plypath);//linemod转换为brachman格式tvec的单位变为m
		Matx44f convmatrix;

		//像素坐标转换到相机坐标系
		img_coord_t camImg;
		getEye(depthpath + "\\depth" + to_string(serial) + ".dpt", camImg);
		//相机坐标系转换到世界坐标系
		//waitKey(0);
		coord3_t pix;
		for (int x = 0; x < camImg.rows; x++)
		{
			for (int y = 0; y < camImg.cols; y++)
			{
				if ((int)binimg.at<uchar>(x, y) == 255)
				{
					Matx31f pix_tmp(camImg(x, y)(0), camImg(x, y)(1), camImg(x, y)(2));
					pix_tmp = rmat * (pix_tmp - 1000 * tvec);
					/*
					if (pix_tmp.val[0] > 0)pix_tmp.val[0] = 0;
					else pix_tmp.val[0] = 255;
					if (pix_tmp.val[1] > 0)pix_tmp.val[1] = 0;
					else pix_tmp.val[1] = 255;
					if (pix_tmp.val[2] > 0)pix_tmp.val[2] = 0;
					else pix_tmp.val[2] = 255;*/
					
					pix(0) = pix_tmp.val[2];
					pix(1) = pix_tmp.val[1];
					pix(2) = pix_tmp.val[0];
					camImg(x, y) = pix;
				}
				else
				{
					pix(0) =0;
					pix(1) =0;
					pix(2) =0;
					camImg(x, y) = pix;
				}

			}
		}
		imwrite(objpath + "\\obj_"+to_string(serial)+".png", camImg);//保存obj文件
		cout << "file:"<<i+1 << endl;
	}
	return 0;
}

