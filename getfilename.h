#ifndef __GETFILENAME_H__
#define __GETFILENAME_H__
#include <io.h>  
#include <iostream>  
#include <vector> 
#include <sstream>
using namespace std;

void getFiles(string path, vector<string>& files);//获取路径下所有文件名（绝对路径）
void getNumfromString(string path, int *num);//获取路径中所有数字，并存进数组





void getFiles(string path, vector<string>& files)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}  
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void getNumfromString(string path, int *num)
{
	istringstream is(path);
	int i = 0;
	char ch;
	while (is >> ch)
	{
		if (ch >= '0'&&ch <= '9')
		{
			is.putback(ch);
			is >> num[i];
			i++;
		}
	}
}
#endif