#ifndef __CONV_RT__
#define __CONV_RT__
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


void getLinemod_R(string &rot_path, Matx33f &rmat);//��ȡlinemod����ϵ�ľ���
void getLinemod_t(string &tra_path, Matx31f &tvec);
void convLine2brachman(Matx33f &rmat, Matx31f &tvec, string plypath);//ת����ʽ��brachman����ϵ
vector<std::string> split(string str, string pattern);//�ָ��ַ���

static void convLinemod2cv(Matx33f &rmat, Matx31f &tvec);//ת��linemod��ʽ��opencv����ϵ
static void readPLYfile(string plypath, vector<vector<float>> &pts);//��.ply�ļ�
static void calExtreme(string plypath, Matx31f &min, Matx31f &max);//����ply�ļ���ÿһ������������Сֵ
static void calCorrection(Matx31f min, Matx31f max, Matx31f &corr);//��������ֵ
static void calExtent(Matx31f min, Matx31f max, Matx31f &extent);//����extent

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
	//��ȡ���
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
	calExtreme(plypath, min, max);//������ֵ
	calCorrection(min, max, ctr);//����ply�ļ�,��������ֵ
	//calExtent(min, max, extent);//����extent
	ctr = (rmat * ctr) * 0.1;
	tvec = tvec + ctr;

	convLinemod2cv(rmat,tvec);
	if (determinant(rmat) < 0)//������ؽ�
	{
		rmat = -rmat;
		tvec = -tvec;
	}
	transpose(rmat,rmat);//ת��
	rmat = CV2brach * rmat;//����ϵ�任
	transpose(rmat, rmat);//ת��
	tvec = tvec * 0.01;//[cm]->[m]
}




///////////////////////////////////////////////////////////////
//private function
//////////////////////////////////////////////////////////////
//�����ַ���
vector<std::string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;//��չ�ַ����Է������
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
//linemod����ϵת����opencv����ϵ
static void convLinemod2cv(Matx33f &rmat, Matx31f &tvec)
{
	rmat = Line2CV * rmat;
	tvec = Line2CV * tvec;
}
//��������ֵ
static void calCorrection(Matx31f min,Matx31f max,Matx31f &corr)
{
	corr = (max + min) * 0.5;	
}
//����extent
static void calExtent(Matx31f min, Matx31f max, Matx31f &extent)
{
	extent = max - min;
	extent = (CV2brach * extent) * 0.001;//[mm]->[m]
}
//����ÿһ���������ֵ
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
			if (pts.at(i).at(j) < min_tmp.val[j]) min_tmp.val[j] = pts.at(i).at(j);//һ�е���Сֵ[mm]
			if (pts.at(i).at(j) > max_tmp.val[j]) max_tmp.val[j] = pts.at(i).at(j);//һ�е����ֵ[mm]
		}
	}
	min = min_tmp;
	max = max_tmp;
}
//��ȡply�ļ�
static void readPLYfile(string plypath, vector<vector<float>> &pts)
{
	ifstream plyfile(plypath, ios::in);
	if (!plyfile){ cout << "open ply file error" << endl; return ; }
	string line;//�洢��ȡ��һ��
	int N;//�������
	stringstream strstream;
	vector<string> str;//�洢����
	while (getline(plyfile,line))//����ͷ��Ϣ
	{
		str = split(line, " ");
		if (str.at(0)=="element" && str.at(1)=="vertex")
		{
			strstream << str.at(2);//stringת��int
			strstream >> N;
			strstream.clear();
		}
		if (line == "end_header")
			break;
	}
	
	vector<vector<float>> pts_l;//�����ľ���
	vector<float> nLinecoor;//ÿһ�е�����
	int i, j;
	float coor;//ÿһ������
	for (i = 0; i < N; i++)//������ȡ����
	{
		getline(plyfile, line);
		str = split(line, " ");
		for (j = 0; j < 3; j++)
		{
			strstream << str.at(j);//stringת��int
			strstream >> coor;
			strstream.clear();
			nLinecoor.push_back(coor); //[mm]
		}
		pts_l.push_back(nLinecoor);
		nLinecoor.clear();//ÿ����Ҫ���
	}
	pts = pts_l;
}
#endif