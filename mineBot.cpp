#include <iostream>
#include <Windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
/*
mineBot.cpp
T.Lloyd

An attempt at an openCV minesweeper bot...

*/
using namespace cv;
using namespace std;

#define until for
#define endOfTime (;;)


BOOL captureFrame(Mat &f,HWND mineHandle,int height, int width){
	HDC mineHDC, mineCompat;
	HBITMAP mineBit;
	BITMAP b;
	BITMAPINFOHEADER mineH;
	if (mineHandle == NULL){
		cout << "Game Not Found\n";
		return FALSE;
	}
	mineHDC = GetDC(mineHandle);
	mineCompat = CreateCompatibleDC(mineHDC);
	if (!mineCompat){
		cout << "Error creating compatible DC\n";
		return FALSE;
	}
	//SetStretchBltMode(mineCompat, HALFTONE);
	SetStretchBltMode(mineCompat, HALFTONE);

	mineBit = CreateCompatibleBitmap(mineHDC, width, height);
	if (mineBit == NULL){
		cout << "Bitmap Creation Failed!\n";
		return FALSE;
	}
	mineH.biSize = sizeof(BITMAPINFOHEADER);
	mineH.biPlanes = 1;
	mineH.biWidth = width;
	mineH.biHeight = -height;
	mineH.biBitCount = 32;
	mineH.biCompression = BI_RGB;

	SelectObject(mineCompat, mineBit);
	if (!StretchBlt(mineCompat, 0, 0, width, height, mineHDC, 0, 0, width, height, SRCCOPY)){
		cout << "Error Stretch BLT\n";
		return FALSE;
	}

	//BitBlt(mineCompat, 0, 0, width, height, mineHDC, 0, 0, SRCCOPY);
	//GetObject(mineBit, sizeof(BITMAP), &b);
	//GetDIBits(mineHDC, mineBit, 0, height, frame.data, (BITMAPINFO*)&mineH, DIB_RGB_COLORS);
	GetDIBits(mineCompat, mineBit, 0, height, f.data, (BITMAPINFO*)&mineH, DIB_RGB_COLORS);

	DeleteObject(mineBit);
	DeleteDC(mineCompat);
	ReleaseDC(mineHandle, mineHDC);

	return TRUE;
}


BOOL countSquares(Mat &f,Mat &t, Mat &r,double threshold,vector<Point> &squares,Scalar colour){
	//Frame,Template,Result,Number of squares
	Mat greysrc = f.clone();
	Mat greyTemp = t.clone();
	cvtColor(f, greysrc, COLOR_BGR2GRAY);
	cvtColor(t, greyTemp, COLOR_BGR2GRAY);

	matchTemplate(greysrc, greyTemp, r, TM_CCOEFF_NORMED);
	normalize(r, r, 0, 1, NORM_MINMAX, -1, Mat());

	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;
	Point matchLoc;
	Point prevMatchLoc;
	squares.clear();
	bool edge = FALSE;
	int dup = 0; //count duplicates
	for (int i = 0, j = 0; i < 81; i++){
		//minesweeper has gradient so only detects two of the squares.
		minMaxLoc(r, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		//if (maxVal > 0.87){
		if ((maxVal > threshold)){
			matchLoc = maxLoc;
			for (int k = 0; k < squares.size(); k++){
				if (((abs(matchLoc.y - squares[k].y) <= 3) && (abs(matchLoc.x - squares[k].x) <= 3))){
					//cout << "!";
					dup++;
				}
			}
			if (matchLoc.x > 30){
				edge = TRUE;
			}
			else{
				edge = FALSE;
			}
				//cout << "Duplicates=" << dup << "\n";
				//dup = 0;
				if ((dup == 0)&&(edge==TRUE)){
					squares.push_back(matchLoc);
					rectangle(r, matchLoc, Point(matchLoc.x + t.cols, matchLoc.y + t.rows), Scalar(0, 0, 0), 2, 8, 0);
					rectangle(f, matchLoc, Point(matchLoc.x + t.cols, matchLoc.y + t.rows), colour, 1, 8, 0);
				}
			//}
			//}
		}
	}
	greysrc.release();
	greyTemp.release();
	//cout << "Vector Size=" << squares.size() << "\n";
	return TRUE;
}
int main(void){
	std::cout << "mineBot\nT.Lloyd\n";
	Mat frame;
	HWND mineHandle = FindWindow(NULL,L"Minesweeper");
	if (mineHandle == NULL){
		cout << "Game Not Found\n";
		return -1;
	}
	else{
		cout << "Game Successfully Found!\n";
	}
	namedWindow("Win", 1);
	RECT winSize;

	int height, width;
	int unpressed = 0;
	GetClientRect(mineHandle, &winSize);
	height = winSize.bottom;
	width = winSize.right;

	frame.create(height, width, CV_8UC4);
	Point pt;
	pt.x = 0;
	pt.y = 0;

	Mat temp = imread("Images//square2.jpg", CV_LOAD_IMAGE_COLOR);
	Mat oneTemp = imread("Images//one3.jpg", CV_LOAD_IMAGE_COLOR);
	//cvtColor(temp, temp, CV_8UC4,0);
	//Mat result;
	//Mat oneResult;
	captureFrame(frame, mineHandle, height, width);
	//imwrite("Images//Master.jpg", frame);
	//result.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1);
	vector<Point> unsquare;
	vector<Point> oneSquares;
	Mat unPressedMat;
	Mat oneMat;
	until endOfTime{
		if (captureFrame(frame, mineHandle,height,width)==TRUE){
			if (!frame.empty()){
				//matchTemplate(frame, temp, result, TM_CCOEFF_NORMED);
				Mat result;
				result.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1);
				//unPressedMat = frame.clone();
				countSquares(frame, temp, result,0.920, unsquare, Scalar(0, 0, 255));
				cout << "Unpressed Squares: " << unsquare.size() << "\n";
				result.release();
				if (unsquare.size() != 81){
					Mat oneResult;
					oneResult.create(frame.cols - oneTemp.cols + 1, frame.rows - oneTemp.rows + 1, CV_32FC1);
					//oneMat = frame.clone();
					countSquares(frame, oneTemp, oneResult, 0.9, oneSquares, Scalar(0, 255, 0));
					cout << "One Squares: " << oneSquares.size() << "\n";
					oneResult.release();
				}
				imshow("Win", frame);
				//imshow("One", oneMat);
				//Sleep(2000);
			}
		}
		if (waitKey(30) >= 0) break;
	}
	return 0;
}