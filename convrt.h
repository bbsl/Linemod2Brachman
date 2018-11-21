#ifndef __CONV_RT__
#define __CONV_RT__
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


void getLinemod_R(string &rot_path, Matx33f &rmat);//获取linemod坐标系的矩阵
void getLinemod_t(string &tra_path, Matx31f &tvec);
void convLine2brachman(Matx33f &rmat, Matx31f &tvec, string plypath);//转换格式到brachman坐标系
vector<std::string> split(string str, string pattern);//分割字符串

static void convLinemod2cv(Matx33f &rmat, Matx31f &tvec);//转换linemod格式到opencv坐标系
static void readPLYfile(string plypath, vector<vector<float>> &pts);//读.ply文件
static void calExtreme(string plypath, Matx31f &min, Matx31f &max);//计算ply文件中每一个坐标的最大最小值
static void calCorrection(Matx31f min, Matx31f max, Matx31f &corr);//计算修正值
static void calExtent(Matx31f min, Matx31f max, Matx31f &extent);//计算extent

static Matx33f Line2CV( 1.0,  0.0,  0.0,
						0.0, -1.0,  0.0,
						0.0,  0.0, -1.0);
static Matx33f CV2brach( 0.0, -1.0,  0.0,
						 0.0,  0.0, -1.0,
						 1.0,  0.0,  0.0);
		

void getLinemod_R(string &rot_path, Matx33f &rmat)
{
	ifstream infile;
	infile.open(rot_path, ios::in);
	if (!infile) { cout << "open rotation file error" << endl; return ; }
	float tmp2[11] = {};
	for (int i = 0; i < 11; i++) infile >> tmp2[i];
	infile.close();
	for (int i = 0; i < 11; i++)
	{
		rmat.val[i] = tmp2[i + 2];
	}
}
void getLinemod_t(string &tra_path, Matx31f &tvec)
{
	//获取外参
	ifstream infile;
	infile.open(tra_path, ios::in);
	if (!infile){ cout << "open translation file error" << endl; return ; }
	float tmp1[5] = {};
	for (int i = 0; i < 5; i++) infile >> tmp1[i];
	infile.close();
	tvec.val[0] = tmp1[2];
	tvec.val[1] = tmp1[3];
	tvec.val[2] = tmp1[4];
}
void convLine2brachman(Matx33f &rmat, Matx31f &tvec, string plypath)
{
	
	Matx31f ctr,extent;
	Matx31f min, max;
	calExtreme(plypath, min, max);//计算最值
	calCorrection(min, max, ctr);//利用ply文件,计算修正值
	//calExtent(min, max, extent);//计算extent
	ctr = (rmat * ctr) * 0.1;
	tvec = tvec + ctr;

	convLinemod2cv(rmat,tvec);
	if (determinant(rmat) < 0)//相机后重建
	{
		rmat = -rmat;
		tvec = -tvec;
	}
	transpose(rmat,rmat);//转置
	rmat = CV2brach * rmat;//坐标系变换
	transpose(rmat, rmat);//转置
	tvec = tvec * 0.01;//[cm]->[m]
}




///////////////////////////////////////////////////////////////
//private function
//////////////////////////////////////////////////////////////
//剪切字符串
vector<std::string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}
//linemod坐标系转换到opencv坐标系
static void convLinemod2cv(Matx33f &rmat, Matx31f &tvec)
{
	rmat = Line2CV * rmat;
	tvec = Line2CV * tvec;
}
//计算修正值
static void calCorrection(Matx31f min,Matx31f max,Matx31f &corr)
{
	corr = (max + min) * 0.5;	
}
//计算extent
static void calExtent(Matx31f min, Matx31f max, Matx31f &extent)
{
	extent = max - min;
	extent = (CV2brach * extent) * 0.001;//[mm]->[m]
}
//计算每一个坐标的最值
static void calExtreme(string plypath, Matx31f &min, Matx31f &max)
{
	vector<vector<float>> pts;
	readPLYfile(plypath, pts);
	int size = pts.size();
	Matx31f min_tmp;
	Matx31f max_tmp;
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < size; i++)
		{
			if (pts.at(i).at(j) < min_tmp.val[j]) min_tmp.val[j] = pts.at(i).at(j);//一列的最小值[mm]
			if (pts.at(i).at(j) > max_tmp.val[j]) max_tmp.val[j] = pts.at(i).at(j);//一列的最大值[mm]
		}
	}
	min = min_tmp;
	max = max_tmp;
}
//读取ply文件
static void readPLYfile(string plypath, vector<vector<float>> &pts)
{
	ifstream plyfile(plypath, ios::in);
	if (!plyfile){ cout << "open ply file error" << endl; return ; }
	string line;//存储读取的一行
	int N;//定点个数
	stringstream strstream;
	vector<string> str;//存储定点
	while (getline(plyfile,line))//过滤头信息
	{
		str = split(line, " ");
		if (str.at(0)=="element" && str.at(1)=="vertex")
		{
			strstream << str.at(2);//string转换int
			strstream >> N;
			strstream.clear();
		}
		if (line == "end_header")
			break;
	}
	
	vector<vector<float>> pts_l;//读出的矩阵
	vector<float> nLinecoor;//每一行的坐标
	int i, j;
	float coor;//每一个坐标
	for (i = 0; i < N; i++)//继续读取坐标
	{
		getline(plyfile, line);
		str = split(line, " ");
		for (j = 0; j < 3; j++)
		{
			strstream << str.at(j);//string转换int
			strstream >> coor;
			strstream.clear();
			nLinecoor.push_back(coor); //[mm]
		}
		pts_l.push_back(nLinecoor);
		nLinecoor.clear();//每次需要清空
	}
	pts = pts_l;
}
#endif