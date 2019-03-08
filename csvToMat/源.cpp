/*
* Copyright (c) 2019 HadenSun
* Contact: http://www.sunhx.cn
* E-mail: admin@sunhx.cn
*
* Description:
*	 ��Դ��
*			EPC���Ĭ�ϱ����ʽCSV�ļ���
*			ÿ֡����(320+8)*(240+12)
*
*	 ������̣�
*			����csv�ļ��������зָ�ɵ�֡����
*			ÿ֡����ת��Mat��ʽ��CV_16U��
*			ת��PCD����
*
*/


#include <opencv2\opencv.hpp>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <stdio.h>
#include <pcl\visualization\cloud_viewer.h>


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

#define   LEFT_BLANK	0
#define	  RIGHT_BLANK	0
#define	  TOP_BLANK		0
#define   BOTTOM_BLANK	0

#define   IMAGE_SAVE	0
#define   IMAGE_SHOW    0
#define   CLOUD_SAVE	1
#define   CLOUD_SHOW	0


using namespace std;
using namespace cv;
using namespace pcl;







//ͼƬ�˲�
Mat imageFilter(Mat src, float threshold)
{
	Mat imgDev = Mat::zeros(240, 320, CV_32F);
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

//��ȡȫ·���е��ļ���׺��
std::string getFileType(std::string file_name)
{
	std::string subname;
	for (auto i = file_name.end() - 1; *i != '.'; i--)
	{
		subname.insert(subname.begin(), *i);
	}

	return subname;
}

void imageConvert(char* dir, char* file)
{
	if (getFileType(file) != "csv")
		return;

	char fileDir[300];
	char fileDirNew[300];

	strcpy(fileDirNew, FILEDIRNEW);

	strcpy(fileDir, dir);
	strcat(fileDir, file);

	//���ļ�
	FILE *p_openFile = NULL;
	p_openFile = fopen(fileDir, "r");			//���ļ�
	if (!p_openFile)
	{
		printf("�ļ������ڣ�\r\n");
		exit(-1);
	}

	char c;
	int pixelDep = 0;					//���ص���ֵ
	int pixelCounter = 0;				//�����ؼ���
	int rowCounter = 0;					//�м���
	int frameCounter = 0;				//֡����
	uint16_t depth[240][320];
	printf("�ļ��򿪳ɹ���\r\n");
	while (1)
	{
		c = fgetc(p_openFile);

		if (c == -1)
		{
			printf("��ɣ�\r\n");
			break;
		}

		if ((pixelCounter >= LEFT_BLANK && pixelCounter < 320 + LEFT_BLANK) && (rowCounter >= TOP_BLANK && rowCounter < 240 + TOP_BLANK))
		{
			if (c == 'N' || c == 'a')
				pixelDep = 0;
			else if (c != ',' && c != '\n')
				pixelDep = pixelDep * 10 + c - '0';

			if (c == ',')
			{
				depth[rowCounter - TOP_BLANK][pixelCounter - LEFT_BLANK] = pixelDep;
			}
		}


		if (c == ',')
		{

			pixelDep = 0;
			pixelCounter++;
		}

		//if (c >= '0' && c <= '9')
		//c = c * 10 + c - '0';

		if (c == '\n')
		{
			pixelCounter = 0;
			rowCounter++;
			if (rowCounter > 239 + TOP_BLANK + BOTTOM_BLANK)		//��֡
			{
				rowCounter = 0;
				frameCounter++;
				printf("�ɹ��и�%d\r\n", frameCounter);
				Mat src_1(240, 320, CV_16UC1, Scalar(0));
				for (int i = 0; i < 240; i++)
				{
					for (int j = 0; j < 320; j++)
					{
						if (depth[240 - i][j] > 30000)
							src_1.at<ushort>(i, j) = 0;
						else
							src_1.at<ushort>(i, j) = depth[240 - i][j];
					}
				}

				if (IMAGE_SAVE)
				{
					//ͼ�񱣴�
					sprintf(fileDirNew, "%s%s-%d.png", FILEDIRNEW, file, frameCounter);
					imwrite(fileDirNew, src_1);
				}
				if (IMAGE_SHOW)
				{
					//α��ɫ��ʾ
					Mat color;
					src_1.convertTo(color, CV_8U);
					applyColorMap(color, color, COLORMAP_JET);
					imshow("win", color);

					waitKey(1);
				}
					

				sprintf(fileDirNew, "%s%s-%d.pcd", FILEDIRNEW, file, frameCounter);
				//�������
				Mat undist = imageUndist(src_1);
				//���Ʊ任����
				pcdSave(undist, fileDirNew);
			}
		}
	}
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

void main()
{

	char* file = FILEDIR;

	listFiles(file);
}
