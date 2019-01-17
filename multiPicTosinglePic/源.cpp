/*
* Copyright (c) 2019 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 ��Դ��	
*			��Ա�������ܲɼ�ͼƬʹ�á�
*			Ϊ�˱�֤ͼ��ɼ��ٶȺͱ����ٶȣ�ÿ֡ͼ����Ϊ1*(640*480)��Ϣ��
*			30֡ͼ����Ϊ307200*30��pngͼƬ��16λ��ȡ�
*
*	 ������̣�
*			����һ��ͼƬ��һ���������´���Ϊһ��640*480ͼƬ��16λ��ȡ�
*
*/


#include <stdio.h>
#include <io.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace std;
using namespace cv;


// �ָ���ͼƬ����λ��
#define FILEDIRNEW	"D:\\�����ɼ�\\restore\\rst\\"
#define FILEDIR		"D:\\�����ɼ�\\restore\\data"


int number = 0;

void imageCut(char* dir, char* file)
{
	char fileDir[300];
	char fileDirNew[300];

	strcpy(fileDirNew, FILEDIRNEW);

	strcpy(fileDir, dir);		//�ϳ��ļ�Ŀ¼
	strcat(fileDir, "\\");
	strcat(fileDir, file);

	Mat ori = imread(fileDir, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);

	Mat depth = Mat(Size(640, 480), CV_16U);
	for (int k = 0; k < 30; k++)
	{
		for (int i = 0; i < 480; i++)
		{
			for (int j = 0; j < 640; j++)
			{
				depth.at<ushort>(i, j) = ori.at<ushort>(k, i * 640 + j);
			}
		}
		//��ʾα��ɫͼƬ
		Mat colorMat;
		depth.convertTo(colorMat, CV_8U, 1.0 / 16, 0);
		applyColorMap(colorMat, colorMat, COLORMAP_JET);	  //��mImageDepth��ɿ��Ƶ����ͼ(α��ɫͼ)
		imshow("win", colorMat);
		waitKey(1);
		//�������ļ���
		char imgFile[200];
		sprintf(imgFile, "%s%06d.png", fileDirNew, number++);
		imwrite(imgFile, depth);
	}
}

void listFiles(const char * dir)
{
	char dirNew[200];
	strcpy(dirNew, dir);
	strcat(dirNew, "\\*.*");    // ��Ŀ¼�������"\\*.*"���е�һ������

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if (handle == -1)        // ����Ƿ�ɹ�
		return;

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;

			cout << findData.name << "\t<dir>\n";

			// ��Ŀ¼�������"\\"����������Ŀ¼��������һ������
			strcpy(dirNew, dir);
			strcat(dirNew, "\\");
			strcat(dirNew, findData.name);

			listFiles(dirNew);
		}
		else
		{
			strcpy(dirNew, dir);
			imageCut(dirNew, findData.name);
			cout << findData.name << "\t" << findData.size << " bytes.\n";
		}
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);    // �ر��������
}


void main()
{

	char* file = FILEDIR;

	listFiles(file);
}