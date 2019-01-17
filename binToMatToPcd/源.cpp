/*
* Copyright (c) 2019 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 ��Դ��
*			Ƥ���Ͽ�����ӡ������ָ������
*			ÿ֡ͼ�񱣴�Ϊһ��bin�ļ���������320*240ͼƬ���ݣ�16λ��ȡ�
*
*	 ������̣�
*			����һ��bin�ļ������´���Ϊһ��320*240ͼƬ��16λ��ȡ�
*			���ͼ�����˲������������תPointCloud���ơ�
*			����pcd��ʽ�����ļ���
*
*/

#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <list>
#include <conio.h>
#include <fstream>
#include <opencv2\opencv.hpp>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl\visualization\cloud_viewer.h>

using namespace cv;
using namespace std;
using namespace pcl;

#define   Img_width   (320)
#define   Img_height  (240)
#define   THRESHOLD	   3000
#define   FX  286.6034
#define   FY  287.3857	
#define   CX  176.2039
#define   CY  126.5788	
#define   K1  -0.12346
#define   K2  0.159423

Mat imgDev;

//ͼƬ�˲�
Mat imageFilter(Mat src, float threshold)
{
	imgDev = Mat::zeros(240, 320, CV_32F);
	Mat rst = Mat::zeros(src.size(), CV_16U);
	for (int i = 1; i < src.size().height - 1; i++)
	{
		for (int j = 1; j < src.size().width - 1; j++)
		{
			int dist[9];
			dist[0] = src.at<ushort>(i - 1, j - 1);
			dist[1] = src.at<ushort>(i - 1, j);
			dist[2] = src.at<ushort>(i - 1, j + 1);
			dist[3] = src.at<ushort>(i, j - 1);
			dist[4] = src.at<ushort>(i, j);
			dist[5] = src.at<ushort>(i, j + 1);
			dist[6] = src.at<ushort>(i + 1, j - 1);
			dist[7] = src.at<ushort>(i + 1, j);
			dist[8] = src.at<ushort>(i + 1, j + 1);

			//����ƽ��ֵ
			float aveg = 0;
			for (int k = 0; k < 9; k++)
			{
				aveg += dist[k];
			}
			aveg = aveg / 9;

			//���������
			float deviation = 0;
			for (int k = 0; k < 9; k++)
			{
				deviation += (dist[k] - aveg) * (dist[k] - aveg);
			}
			deviation = deviation / 9;
			imgDev.at<float>(i, j) = deviation;

			//���������ֵ����
			if (deviation > threshold)
			{
				rst.at<ushort>(i, j) = 0;
			}
			else
				rst.at<ushort>(i, j) = src.at<ushort>(i, j);
		}
	}

	return rst.clone();
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

//�������Ӧ�¼�
void on_mouse(int event, int x, int y, int flags, void* userdata)
{
	Mat hh;
	char file[64];
	hh = *(Mat*)userdata;
	Point p(x, y);
	if (event == EVENT_LBUTTONDOWN)
	{
		//sprintf(file, "(%d)", (int)(hh.at<uchar>(p)));
		//putText(src, file, p, FONT_HERSHEY_PLAIN, 1, Scalar(255), 1, 8);
		printf("b=%0.3f\n", hh.at<ushort>(p) / 30000.0*12.5);
		printf("n=%d\n", hh.at<ushort>(p));
		printf("d=%f\n", imgDev.at<float>(p));
		// circle(hh, p, 2, Scalar(255), 3);
	}
	else if (event == CV_EVENT_RBUTTONDOWN)
	{
		circle(hh, p, 2, Scalar(255), 3);
	}


}

//��ȡBin�ļ�תMat
//bin������ֵ0-30000
Mat  getDataFromfile(const char * filename)
{

	ifstream myi(filename, ios::binary);
	unsigned short * imagebuf = new unsigned short[Img_width*Img_height];
	Mat outmat(Img_height, Img_width, CV_16UC1);
	if (!myi.is_open())
	{
		return outmat;
	}
	//��ȡԭʼ����
	myi.read((char*)imagebuf, Img_width*Img_height*sizeof(unsigned short));
	myi.close();

	//��������תMat
	for (int i = 0; i < Img_height; i++)
	{
		for (int j = 0; j < Img_width; j++)
		{
			if (imagebuf[j + i * Img_width] < 30000)
				outmat.at<ushort>(i, j) = imagebuf[j + i * Img_width];
			else
				outmat.at<ushort>(i, j) = 0;
		}
	}

	delete[] imagebuf;
	return outmat;
}

int _tmain(int argc, _TCHAR* argv[])
{


	bool bsave = 0;
	char imgfilename85[_MAX_PATH];
	char outfilename[_MAX_PATH];
	_TCHAR inipath[_MAX_PATH];
	_TCHAR  depthcfgpath[_MAX_PATH];

	_TCHAR drive[_MAX_DRIVE];
	_TCHAR dir[_MAX_DIR];
	//	char	szdepthcfgpath[_MAX_PATH];
	_tsplitpath(argv[0], drive, dir, NULL, NULL);
	sprintf(imgfilename85, "D:\\����\\����ƴ�ӷָ�\\save\\save\\110915\\CAM2_4.bin");

	Mat outmat85 = getDataFromfile(imgfilename85);

	if (outmat85.empty())
	{
		cout << "err when open file!" << endl;
		return -1;
	}


	//�˲�
	outmat85 = imageFilter(outmat85, THRESHOLD);
	//�������
	outmat85 = imageUndist(outmat85);
	//תα��ɫͼ
	Mat zip;
	outmat85.convertTo(zip, CV_8U, 25.0 / 3000, 0);
	applyColorMap(zip, zip, COLORMAP_HSV);

	namedWindow("win1", 0);
	imshow("win1", zip);

	//����������¼�
	setMouseCallback("win1", on_mouse, &outmat85);
	waitKey(0);


	//����PCD�ļ�
	pcdSave(outmat85);

	return 0;
}



