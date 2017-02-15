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
	GetClientRect(mineHandle, &winSize);
	height = winSize.bottom;
	width = winSize.right;

	frame.create(height, width, CV_8UC4);
	Point pt;
	pt.x = 0;
	pt.y = 0;
	until endOfTime{
		if (captureFrame(frame, mineHandle,height,width)==TRUE){
			if (!frame.empty()){
				imshow("Win", frame);
			}
		}
		if (waitKey(30) >= 0) break;
	}
	return 0;
}