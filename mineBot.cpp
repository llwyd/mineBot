#include <iostream>
#include <Windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <string.h>
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

//Counts squares and doesn't check them against the master grid
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
			if ((matchLoc.x > 30)&&(matchLoc.y>30)){
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


//Counts squares and checks them against the master grid
BOOL countSquares(Mat &f, Mat &t, Mat &r, double threshold, vector<Point> &squares, vector<Point>&masterGrid, Scalar colour){
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
	bool inside = FALSE;
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
			if ((matchLoc.x > 30) && (matchLoc.y>30)){
				edge = TRUE;
			}
			else{
				edge = FALSE;
			}
			for (int k = 0; k < masterGrid.size(); k++){
				if ((matchLoc.x >= masterGrid[k].x) && (matchLoc.x <= (masterGrid[k].x + 4)) && (matchLoc.y >= masterGrid[k].y) && (matchLoc.y <= (masterGrid[k].y + 4))){
					inside = TRUE;
					break;
				}
				else{
					inside = FALSE;
					//break;
				}
			}


			//cout << "Duplicates=" << dup << "\n";
			//dup = 0;
			if ((dup == 0) && (edge == TRUE)&&(inside==TRUE)){
				squares.push_back(matchLoc);
				rectangle(r, matchLoc, Point(matchLoc.x + t.cols, matchLoc.y + t.rows), Scalar(0, 0, 0), CV_FILLED, 8, 0);
				rectangle(f, matchLoc, Point(matchLoc.x + t.cols, matchLoc.y + t.rows), colour, 1, 8, 0);
			}
			//}
			//}
		}
	}
	greysrc.release();
	greyTemp.release();
	return TRUE;
}

void printSquares(vector<Point> &p){
	for (int k = 0; k < p.size(); k++){
		cout << "(" << p[k].x << "," << p[k].y << ")\n";
	}
}



bool sortX(Point p1, Point p2){return (p1.x < p2.x);}
bool sortY(Point p1, Point p2){ return (p1.y < p2.y); }

void sortGrid(vector<Point> &p){
	//9x9 Grid Sorting Algorithm
	std::sort(p.begin(), p.end(), sortX);
	for (int i = 0; i < 9; i++){
		std::sort(p.begin() + (i * 9), p.begin() + ((i + 1) * 9), sortY);
	}
}
void printGrid(char** &g){
	for (int j = 0; j < 9; j++){
		for (int k = 0; k < 9; k++){
			cout << g[k][j];
		}
		cout << "\n";
	}
}
void updateGrid(char** &g,vector<Point>&s,vector<Point>&masterGrid,char value){
	for (int j = 0; j < s.size(); j++){
		for (int k = 0; k < masterGrid.size(); k++){
			if ((s[j].x >= masterGrid[k].x) && (s[j].x <=(masterGrid[k].x + 16)) && (s[j].y >= masterGrid[k].y) && (s[j].y <= (masterGrid[k].y + 16))){
				int xpos = (int)(k / 9);
				int ypos = (int)(k % 9);
				g[xpos][ypos] = value;
			}
		}
	}
}
void clearGrid(char** &g){
	for (int i = 0; i < 9; i++){
		for (int k = 0; k < 9; k++){
			g[i][k] = 0;
		}
	}
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
	Mat twoTemp = imread("Images//two.jpg", CV_LOAD_IMAGE_COLOR);
	Mat threeTemp = imread("Images//three.jpg", CV_LOAD_IMAGE_COLOR);
	Mat fourTemp = imread("Images//four.jpg", CV_LOAD_IMAGE_COLOR);
	Mat blankTemp = imread("Images//blank.jpg", CV_LOAD_IMAGE_COLOR);
	//cvtColor(temp, temp, CV_8UC4,0);
	//Mat result;
	//Mat oneResult;

	//----------------------------------------------------------------------------
	//Initial read to define co-ordinates

	Mat gridResult;
	vector<Point> gridMap;
	captureFrame(frame, mineHandle, height, width);
	gridResult.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1);
	//unPressedMat = frame.clone();
	countSquares(frame, temp, gridResult, 0.920, gridMap, Scalar(0, 255, 255));
	cout << "Unpressed Squares: " << gridResult.size() << "\n";
	printSquares(gridMap);
	cv::imshow("Win", frame);
	gridResult.release();

	sortGrid(gridMap);
	printSquares(gridMap);
//	for (int i = 0; i < gridMap.size; i++){
		
//	}
	//Sleep(1000 * 5);
	//----------------------------------------------------------------------------
	
	//imwrite("Images//Master.jpg", frame);
	//result.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1);
	char **grid = (char **)calloc(9, sizeof(char));
	for (int k = 0; k < 9; k++){
		grid[k] = (char*)calloc(9, sizeof(char));
	}
//	char grid[9][9] = { 0 };
	vector<Point> unsquare;
	vector<Point> oneSquares;
	vector<Point> twoSquares;
	vector<Point> threeSquares;
	vector<Point> fourSquares;
	vector<Point> blankSquares;
	Mat unPressedMat;
	Mat oneMat;
	Mat twoMat;
	Mat threeMat;
	Mat fourMat;
	Mat blankMat;
	until endOfTime{
		clearGrid(grid);
		if (captureFrame(frame, mineHandle,height,width)==TRUE){
			if (!frame.empty()){
				//matchTemplate(frame, temp, result, TM_CCOEFF_NORMED);
				//Masking to get rid of shadows
				Mat m;
			//	inRange(frame, Scalar(71, 80, 88), Scalar(150, 150, 170), m);
				//inRange(frame, Scalar(71, 80, 88), Scalar(107, 102, 98), m);
			//	frame.setTo(Scalar(226, 206, 192), m);
				//inRange(frame, Scalar(160,0,0), Scalar(255, 170, 130), m);
				//inRange(frame, Scalar(160, 0, 0), Scalar(255, 210, 160), m);
				//frame.setTo(Scalar(255, 0, 0), m);


				//imshow("mask", m);
				Mat result;
				result.create(frame.cols - temp.cols + 1, frame.rows - temp.rows + 1, CV_32FC1);
				//unPressedMat = frame.clone();
				countSquares(frame, temp, result,0.920, unsquare, Scalar(0,255, 255));
				cout << "Unpressed Squares: " << unsquare.size() << "\n";
				//printSquares(unsquare);
				result.release();
				if (unsquare.size() != 81){
				//	/*
					
					//Mat blankResult;
					//blankResult.create(frame.cols - blankTemp.cols + 1, frame.rows - blankTemp.rows + 1, CV_32FC1);
					//countSquares(frame, blankTemp, blankResult, 0.7, blankSquares, Scalar(0, 0, 255));
					//cout << "Blank Squares: " << blankSquares.size() << "\n";
					//blankResult.release();
					
					Mat oneResult;
					oneResult.create(frame.cols - oneTemp.cols + 1, frame.rows - oneTemp.rows + 1, CV_32FC1);
					//oneMat = frame.clone();
					countSquares(frame, oneTemp, oneResult, 0.9, oneSquares, Scalar(255, 255, 0));
					cout << "One Squares: " << oneSquares.size() << "\n";
					oneResult.release();

					Mat twoResult;
					twoResult.create(frame.cols - twoTemp.cols + 1, frame.rows - twoTemp.rows + 1, CV_32FC1);
					//oneMat = frame.clone();
					countSquares(frame, twoTemp, twoResult, 0.9, twoSquares, Scalar(255, 255, 0));
					cout << "Two Squares: " << twoSquares.size() << "\n";
					twoResult.release();

					Mat threeResult;
					threeResult.create(frame.cols - threeTemp.cols + 1, frame.rows - threeTemp.rows + 1, CV_32FC1);
					//oneMat = frame.clone();
					countSquares(frame, threeTemp, threeResult, 0.95, threeSquares, Scalar(255, 255,0));
					cout << "Three Squares: " << threeSquares.size() << "\n";
					threeResult.release();

					Mat fourResult;
					fourResult.create(frame.cols - fourTemp.cols + 1, frame.rows - fourTemp.rows + 1, CV_32FC1);
					//oneMat = frame.clone();
					countSquares(frame, fourTemp, fourResult, 0.9, fourSquares, gridMap, Scalar(255, 255, 0));
					cout << "Four Squares: " << fourSquares.size() << "\n";
					fourResult.release();
				//	*/
				}
				//update grid
				updateGrid(grid, unsquare, gridMap, 'U');
				updateGrid(grid, oneSquares, gridMap, '1');
				updateGrid(grid, twoSquares, gridMap, '2');
				updateGrid(grid, threeSquares, gridMap, '3');
				updateGrid(grid, fourSquares, gridMap, '4');
				printGrid(grid);
				cv::imshow("Win", frame);
				//imshow("One", oneMat);
				//Sleep(1000);
			}
		}
		if (waitKey(30) >= 0) break;
	}
	return 0;
}