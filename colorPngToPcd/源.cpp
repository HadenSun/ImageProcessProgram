/*
* Copyright (c) 2019 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 ��Դ��
*			EPC�ٷ�ԭʼ�����α��ɫͼ�񡣸���α��ɫ����תΪ���ͼ��
*
*	 ������̣�
*			����α��ɫPNGͼ�񣬸���α��ɫ����ת��Ϊ12λ������ݡ�
*			���������תΪ�������ݣ�������pcd�ļ���
*
*/


#include <opencv2\core\core.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <io.h>
#include <pcl\io\pcd_io.h>
#include <pcl\point_cloud.h>
#include <pcl\visualization\cloud_viewer.h>

using namespace std;
using namespace cv;
using namespace pcl;

#define RESOLUTION	1022
#define   FX  296.041247147697
#define   FY  296.839046024906	
#define   CX  178.091392048995
#define   CY  129.480179021482	
#define   K1  -0.145637046815776
#define   K2  0.276467521132465


Mat remapMatToDepthMat(Mat imgOrigin)
{
	Mat imgDepth = Mat::zeros(Size(320, 240), CV_16U);
	Mat lut = imread("lut.png", CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);

	for (int h = 6; h < imgOrigin.size().height - 6; h++)
	{
		for (int w = 4; w < imgOrigin.size().width - 4; w++)
		{
			if (imgOrigin.at<Vec3b>(h, w)[0] == 128 && imgOrigin.at<Vec3b>(h, w)[1] == 0 && imgOrigin.at<Vec3b>(h, w)[2] == 255)
			{
				imgDepth.at<ushort>(h - 6, w - 4) = 0;
				//printf("���ص�(%d,%d)\n", w-4, h-6);
				continue;
			}

			if (imgOrigin.at<Vec3b>(h, w)[0] == 0 && imgOrigin.at<Vec3b>(h, w)[1] == 0 && imgOrigin.at<Vec3b>(h, w)[2] == 0)
			{
				imgDepth.at<ushort>(h - 6, w - 4) = 0;
				//printf("��Ч��(%d,%d)\n", w - 4, h - 6);
				continue;
			}


			int img_1 = imgOrigin.at<Vec3b>(h, w)[0];
			int img_2 = imgOrigin.at<Vec3b>(h, w)[1];
			int img_3 = imgOrigin.at<Vec3b>(h, w)[2];

			if (img_3 == 0)
			{
				if (img_2 == 0)
				{
					imgDepth.at<ushort>(h - 6, w - 4) = img_1 - 127;
				}
				else
				{
					imgDepth.at<ushort>(h - 6, w - 4) = 128 + img_2;
				}
			}
			else
			{
				if (img_2 == 255)
				{
					imgDepth.at<ushort>(h - 6, w - 4) = 383 + img_3;
				}
				else
				{
					imgDepth.at<ushort>(h - 6, w - 4) = 638 + (255 - img_2) * 3 / 2;
				}
			}
			imgDepth.at<ushort>(h - 6, w - 4) = RESOLUTION - imgDepth.at<ushort>(h - 6, w - 4);
		}
	}

	return imgDepth.clone();
}

//�������
//���룺 ��������ͼƬ
//����� У�����ͼƬ
Mat imageUndist(Mat src)
{
	//�������
	Mat img;

	//�ڲξ���
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);		//3*3��λ����
	cameraMatrix.at<double>(0, 0) = FX;
	cameraMatrix.at<double>(0, 1) = 0;
	cameraMatrix.at<double>(0, 2) = CX;
	cameraMatrix.at<double>(1, 1) = FY;
	cameraMatrix.at<double>(1, 2) = CY;
	cameraMatrix.at<double>(2, 2) = 1;

	//�������
	Mat distCoeffs = Mat::zeros(5, 1, CV_64F);		//5*1ȫ0����
	distCoeffs.at<double>(0, 0) = K1;
	distCoeffs.at<double>(1, 0) = K2;
	distCoeffs.at<double>(2, 0) = 0;
	distCoeffs.at<double>(3, 0) = 0;
	distCoeffs.at<double>(4, 0) = 0;

	Size imageSize = src.size();
	Mat map1, map2;
	//����1������ڲξ���
	//����2���������
	//����3����ѡ���룬��һ�͵ڶ��������֮�����ת����
	//����4��У�����3X3�������
	//����5����ʧ��ͼ��ߴ�
	//����6��map1�������ͣ�CV_32FC1��CV_16SC2
	//����7��8�����X/Y������ӳ�����
	cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), cameraMatrix, imageSize, CV_32FC1, map1, map2);	//�������ӳ��
	//����1������ԭʼͼ��
	//����2�����ͼ��
	//����3��4��X\Y������ӳ��
	//����5��ͼ��Ĳ�ֵ��ʽ
	//����6���߽���䷽ʽ
	cv::remap(src, img, map1, map2, INTER_LINEAR);																	//�������
	return img.clone();
}

//pcd�ļ�����
void pcdSave(Mat mImageDepth)
{
	int imgWidth = mImageDepth.size().width;
	int imgHeight = mImageDepth.size().height;

	PointCloud<PointXYZ> pointCloud;
	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			float picDist = sqrt((i - imgHeight / 2.0)*(i - imgHeight / 2.0) + (j - imgWidth / 2.0)*(j - imgWidth / 2.0));	//ͼ���ϵ㵽���ĵ����ص����
			float picAngle = atan2(FX*(i - imgHeight / 2.0), FY*(j - imgWidth / 2.0));													//ͼ����x,y�����ĵ�Ƕȹ�ϵ
			float angle = atan(sqrt((j - imgWidth / 2.0)*(j - imgWidth / 2.0) / FX / FX + (i - imgHeight / 2.0)*(i - imgHeight / 2.0) / FY / FY));
			float dist = mImageDepth.at<ushort>(i, j) / 3000.0 * 125;				//ԭʼͼ�����

			PointXYZ p;
			p.z = dist*cos(angle);									//����任������
			p.x = dist*sin(angle)*cos(picAngle);
			p.y = dist*sin(angle)*sin(picAngle);

			pointCloud.points.push_back(p);
		}
	}

	//���ƿ��ӻ�
	PointCloud<PointXYZ>::Ptr cloud = pointCloud.makeShared();
	visualization::CloudViewer viewer("Cloud Viewer");
	viewer.showCloud(cloud);
	while (!viewer.wasStopped())
	{

	}


	//�������
	pointCloud.height = imgHeight;
	pointCloud.width = imgWidth;
	io::savePCDFileBinary("pcl.pcd", pointCloud);
}


int main()
{

	Mat srcImg = imread("D:\\����\\�泵���̲�ȡ\\ԭʼ����\\q-1_020.png");

	Mat depthImg = remapMatToDepthMat(srcImg);

	Mat undistImg = imageUndist(depthImg);

	pcdSave(undistImg);

	return 0;
}