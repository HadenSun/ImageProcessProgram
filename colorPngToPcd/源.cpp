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
#include <pcl\filters\statistical_outlier_removal.h>

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
			float picAngle = atan2(i - imgHeight / 2.0, j - imgWidth / 2.0);												//ͼ����x,y�����ĵ�Ƕȹ�ϵ
			float angle = atan(sqrt((j - imgWidth / 2.0)*(j - imgWidth / 2.0) / FX / FX + (i - imgHeight / 2.0)*(i - imgHeight / 2.0) / FY / FY));
			float dist = mImageDepth.at<ushort>(i, j) / 300.0 * 125;				//ԭʼͼ�����

			PointXYZ p;
			p.z = dist*cos(angle);									//����任������
			p.x = dist*sin(angle)*cos(picAngle);
			p.y = dist*sin(angle)*sin(picAngle);

			pointCloud.points.push_back(p);
		}
	}

	//�˲�
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);

	cloud = pointCloud.makeShared();
	//�����˲�������
	StatisticalOutlierRemoval<PointXYZ> sor;
	//���㷨���������������Σ��ڵ�һ�ε����ڼ䣬��������ÿ�������������k���ھ�֮���ƽ�����롣 ����ʹ��setMeanK��������k��ֵ��
	//������������������Щ�����ƽ��ֵ�ͱ�׼ƫ��Ա�ȷ��������ֵ��
	//������ֵ�����ڣ�mean + stddev_mult * stddev�� ����ʹ��setStddevMulThresh�������ñ�׼ƫ��ĳ�����
	//����һ�ε����ڼ䣬������ǵ�ƽ�����ھ���ֱ���ڻ���ڸ���ֵ����㽫������Ϊ�ڲ����쳣ֵ��
	//Ϊÿ����ѯ���ҵ����ھӽ���setInputCloud���������е����ҵ���������������setIndices������������Щ�㡣 
	//setIndices������������������Ϊ������ѯ������ĵ㡣
	sor.setInputCloud(cloud);
	sor.setMeanK(50);					//������50���ھӼ���ƽ������
	sor.setStddevMulThresh(1.0);		//���ñ�׼��Ŵ�ϵ��
	sor.filter(*cloud_filtered);

	int max = 0;
	//���ƿ��ӻ�
	PointCloud<PointXYZRGB> cloud_color;
	for (int i = 0; i < cloud_filtered->size(); i++)
	{
		PointXYZRGB p;
		p.x = cloud_filtered->points.at(i).x;
		p.y = cloud_filtered->points.at(i).y;
		p.z = cloud_filtered->points.at(i).z;

		if (max < p.z)
			max = p.z;
		if (p.z < 100)
			continue;

		p.z -= 200;
		if (p.z < 64)
		{
			p.r = 255;
			p.g = p.z*4;
			p.b = 0;
		}
		else if (p.z < 128)
		{
			p.r = 512 - p.z * 4;
			p.g = 255;
			p.b = 0;
		}
		else if (p.z < 192)
		{
			p.r = 0;
			p.g = 255;
			p.b = p.z*4 - 512;
		}
		else if (p.z < 256)
		{
			p.r = p.z * 4 - 768;
			p.g = 255;
			p.b = 255;
		}
		else
		{
			p.r = 100;
			p.g = 100;
			p.b = 100;
		}
		

		cloud_color.points.push_back(p);
	}
	printf("max is %d", max);
	visualization::CloudViewer viewer("Cloud Viewer");
	
	PointCloud<PointXYZRGB>::Ptr cloud_show = cloud_color.makeShared();
	viewer.showCloud(cloud_show);
	while (!viewer.wasStopped())
	{

	}


	//�������
	//cloud_filtered->height = imgHeight;
	//cloud_filtered->width = imgWidth;
	//io::savePCDFileBinary("pcl.pcd", *cloud_filtered);
	cloud_color.height = 1;
	cloud_color.width = cloud_color.size();
	io::savePCDFileASCII("pcl.pcd", cloud_color);
}


int main()
{

	Mat srcImg = imread("D:\\����\\�泵���̲�ȡ\\ԭʼ����\\q-1_020.png");

	Mat depthImg = remapMatToDepthMat(srcImg);

	Mat undistImg = imageUndist(depthImg);

	pcdSave(undistImg);

	return 0;
}