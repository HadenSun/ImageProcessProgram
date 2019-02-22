#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;


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


int main()
{
	Mat depthImg = imread("Camera0_Depth_60.png", CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);
	Mat zipImg;

	//depthImg.convertTo(zipImg, CV_8U, 1.0/5 , -240);

	Mat colorImg;
	//applyColorMap(zipImg, colorImg, COLORMAP_HSV);
	colorImg = gray2rainbow(depthImg, 800, 3096);

	imshow("d", colorImg);

	waitKey(0);
}