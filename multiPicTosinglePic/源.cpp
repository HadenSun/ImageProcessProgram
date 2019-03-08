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


//ԭʼͼ��λ��
#define FILEDIR		"D:\\����\\��������20190228\\2019-0228(9)\\"
// �ָ���ͼƬ����λ��
#define FILEDIRNEW	"E:\\�����ɼ�0228\\2019-0228(9)\\"
//��ʼ�ļ����� "NULL"��ͷ��ʼ
#define START_FILE_NAME	"NULL"
//�����ɫͼ�� 0������
#define SAVE_COLORMAP 1
//�������ͼ�� 0������
#define SAVE_DEPTH    1
//�����ʾ֡�� 0����ʾ
#define SHOW_NUM 0
//ÿ��ͼ���ڰ�����֡��
#define PIC_NUM 30


int number = 0;

void imageCut(char* dir, char* file)
{
	char fileDir[300];
	char fileDirNew[300];

	strcpy(fileDirNew, FILEDIRNEW);

	strcpy(fileDir, dir);		//�ϳ��ļ�Ŀ¼
	strcat(fileDir, file);

	Mat ori;
	try{
		ori = imread(fileDir, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	}
	catch (exception e)
	{
		sprintf("Can not open %s%s!\n", dir, file);
		return;
	}
	//���ͼƬ�ļ�
	if (ori.size().width == 0 || ori.size().height == 0)
	{
		printf("Open %s%s error!\n", dir, file);
		return;
	}

	Mat depth = Mat(Size(640, 480), CV_16U);
	MatConstIterator_<ushort> it = ori.begin<ushort>();
	for (int k = 0; k < PIC_NUM; k++)
	{
		for (int i = 0; i < 480; i++)
		{
			for (int j = 0; j < 640; j++)
			{
				//depth.at<ushort>(i, j) = ori.at<ushort>(k, i * 640 + j);
				depth.at<ushort>(i, j) = *(it++);
			}
		}

		Mat colorMat;
		char imgFile[200];
		//��ʾα��ɫͼƬ
		if (SHOW_NUM != 0 && number % SHOW_NUM == 0)
		{
			if (colorMat.size().height == 0)
			{
				depth.convertTo(colorMat, CV_8U, 1.0 / 16, 0);
				applyColorMap(colorMat, colorMat, COLORMAP_JET);	  //��mImageDepth��ɿ��Ƶ����ͼ(α��ɫͼ)
			}
			imshow("win", colorMat);
			waitKey(1);
		}
		if (SAVE_COLORMAP)
		{
			if (colorMat.size().height == 0)
			{
				depth.convertTo(colorMat, CV_8U, 1.0 / 16, 0);
				applyColorMap(colorMat, colorMat, COLORMAP_JET);	  //��mImageDepth��ɿ��Ƶ����ͼ(α��ɫͼ)
			}
			sprintf(imgFile, "%scolor-%06d.png", fileDirNew, number);
			imwrite(imgFile, colorMat);
		}
		if (SAVE_DEPTH)
		{
			//�������ļ���
			sprintf(imgFile, "%s%06d.png", fileDirNew, number);
			imwrite(imgFile, depth);
		}
		number++;
	}
}

void listFiles(const char * dir)
{
	int saveflag = 0;
	char dirNew[200];
	strcpy(dirNew, dir);
	strcat(dirNew, "*.*");    // ��Ŀ¼�������"\\*.*"���е�һ������

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
			strcat(dirNew, findData.name);
			strcat(dirNew, "\\");

			listFiles(dirNew);
		}
		else
		{
			//�ж���ʼ�ļ�
			if (!strcmp(findData.name,START_FILE_NAME))
			{
				saveflag = 1;
			}
			if (!strcmp("NULL", START_FILE_NAME))
				saveflag = 1;
		

			//�����ļ�
			if (saveflag)
			{
				strcpy(dirNew, dir);
				imageCut(dirNew, findData.name);
			}
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