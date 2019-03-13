#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\opencv.hpp>
#include <stdio.h>
#include <pcl\visualization\cloud_viewer.h>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>


//��ȡ�ļ�·��
#define FILEDIR		"D:\\pcd\\csv\\"
//�����ļ�·��
#define FILEDIRNEW	"D:\\pcd\\pcd\\"

#define   Img_width   (320)
#define   Img_height  (240)
#define   FX  196.638316
#define   FY  197.0980708
#define   CX  167.7838572
#define   CY  124.3448776
#define   K1  -0.289046233
#define   K2  0.091629024
#define   THRESHOLD	   3000

#define   IMAGE_SHOW    0
#define   CLOUD_SAVE	1
#define   CLOUD_SHOW	0


using namespace std;
using namespace cv;
using namespace pcl;


//�� CV_16UC1 ���ͼ ת����α��ɫͼ
Mat gray2rainbow(const Mat& scaledGray, int min, int max)
{
	Mat outputRainbow(scaledGray.size(), CV_8UC3);       //��ʼ����һ��outputRainbow�Ĳ�ɫͼ��
	unsigned short grayValue;
	float tempvalue;

	float par = (float)255 / (max - min);


	for (int y = 0; y < scaledGray.rows; y++)
	for (int x = 0; x < scaledGray.cols; x++)
	{

		grayValue = scaledGray.at<ushort>(y, x);
		if ((grayValue > 0) && (grayValue < min))        //���ܻ�����ҵ���min��������������Сֵ
		{
			tempvalue = (float)min;
		}
		else if (grayValue > max)                     //Ҳ���ܻ�����ҵ���max���������������ֵ
		{
			tempvalue = 0;
		}
		else
		{
			tempvalue = (float)(grayValue - min);
		}
		tempvalue = tempvalue*par;          //Ϊ�˰����ֵ�滮��(0~255֮��)
		/*
		* color    R   G   B   gray
		* red      255 0   0   255
		* orange   255 127 0   204
		* yellow   255 255 0   153
		* green    0   255 0   102
		* cyan     0   255 255 51
		* blue     0   0   255 0
		*/

		Vec3b& pixel = outputRainbow.at<Vec3b>(y, x);
		tempvalue = 256 - tempvalue;

		if ((tempvalue <= 0) || (tempvalue >= 255))
		{
			pixel[0] = 0;
			pixel[1] = 0;
			pixel[2] = 0;
		}
		else if (tempvalue <= 51)
		{
			pixel[0] = 255;
			pixel[1] = (unsigned char)(tempvalue * 5);
			pixel[2] = 0;
		}
		else if (tempvalue <= 102)
		{
			tempvalue -= 51;
			pixel[0] = 255 - (unsigned char)(tempvalue * 5);
			pixel[1] = 255;
			pixel[2] = 0;
		}
		else if (tempvalue <= 153)
		{
			tempvalue -= 102;
			pixel[0] = 0;
			pixel[1] = 255;
			pixel[2] = (unsigned char)(tempvalue * 5);
		}
		else if (tempvalue <= 204)
		{
			tempvalue -= 153;
			pixel[0] = 0;
			pixel[1] = 255 - static_cast<unsigned char>(tempvalue * 128.0 / 51 + 0.5);
			pixel[2] = 255;
		}
		else if (tempvalue < 255)
		{
			tempvalue -= 204;
			pixel[0] = 0;
			pixel[1] = 127 - static_cast<unsigned char>(tempvalue * 127.0 / 51 + 0.5);
			pixel[2] = 255;
		}
	}

	return outputRainbow;
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
void pcdSave(Mat mImageDepth, char* saveFile)
{
	int imgWidth = mImageDepth.size().width;
	int imgHeight = mImageDepth.size().height;

	PointCloud<PointXYZ> pointCloud;
	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			if (mImageDepth.at<ushort>(i, j) > 30000)
				continue;

			if (mImageDepth.at<ushort>(i, j) < 10)
				continue;

			float picDist = sqrt((i - imgHeight / 2.0)*(i - imgHeight / 2.0) + (j - imgWidth / 2.0)*(j - imgWidth / 2.0));	//ͼ���ϵ㵽���ĵ����ص����
			float picAngle = atan2(FX*(i - imgHeight / 2.0), FY*(j - imgWidth / 2.0));												//ͼ����x,y�����ĵ�Ƕȹ�ϵ
			float angle = atan(sqrt((j - imgWidth / 2.0)*(j - imgWidth / 2.0) / FX / FX + (i - imgHeight / 2.0)*(i - imgHeight / 2.0) / FY / FY));
			float dist = mImageDepth.at<ushort>(i, j) / 3000.0 * 125;				//ԭʼͼ�����



			PointXYZ p;
			p.z = dist*cos(angle);									//����任������
			p.x = dist*sin(angle)*cos(picAngle);
			p.y = dist*sin(angle)*sin(picAngle);

			pointCloud.points.push_back(p);
		}
	}

	if (CLOUD_SHOW)
	{
		//���ƿ��ӻ�
		PointCloud<PointXYZ>::Ptr cloud = pointCloud.makeShared();
		visualization::CloudViewer viewer("Cloud Viewer");
		viewer.showCloud(cloud);
		while (!viewer.wasStopped())
		{

		}
	}


	if (CLOUD_SAVE)
	{
		//�������
		pointCloud.height = 1;
		pointCloud.width = pointCloud.size();
		io::savePCDFileBinary(saveFile, pointCloud);		//�����Ʊ���
		//io::savePCDFile(saveFile, pointCloud);				//ASCII����
	}

}

void imageConvert(char* dir, char* file)
{

	char fileDir[300];
	char fileDirNew[300];

	strcpy(fileDirNew, FILEDIRNEW);

	strcpy(fileDir, dir);
	strcat(fileDir, file);

	Mat src_1 = imread(fileDir, CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);


	if (IMAGE_SHOW)
	{
		//α��ɫ��ʾ
		Mat color;
		src_1.convertTo(color, CV_8U);
		applyColorMap(color, color, COLORMAP_JET);
		imshow("win", color);

		waitKey(1);
	}


	sprintf(fileDirNew, "%s%s.pcd", FILEDIRNEW, file);
	//�������
	Mat undist = imageUndist(src_1);
	//���Ʊ任����
	pcdSave(undist, fileDirNew);
			
}

void listFiles(const char * dir)
{
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
			strcpy(dirNew, dir);
			imageConvert(dirNew, findData.name);
			cout << findData.name << "\t" << findData.size << " bytes.\n";
		}
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);    // �ر��������
}

int main()
{

	char* file = FILEDIR;

	listFiles(file);
}