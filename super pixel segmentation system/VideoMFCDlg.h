
// VideoMFCDlg.h : header file
//

#pragma once

#include "cv.h"
#include "highgui.h"
#include "CvvImage.h"
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace cv;

enum CHANNELS
{
	RED = 0,
	GREEN,
	BLUE,
	INTENSITY
};

typedef struct stImage
{
	double val[4];
}Image;

typedef struct stEdge
{
	int v1;
	int v2;
	double w;
}Edge;






Image ** ConvImgToDbl(Mat img);
void ConvDblToImg(Image **, Mat img, bool);
void Smooth(Image **, Image **, int);
void MST_Segmentation(Image **, int);
int compare(const void *, const void *);

void make_set(int);
void link(int, int);
int find_set(int);
void union_set(int, int);

int Min(double, double);
int Max(double, double);



// CVideoMFCDlg dialog
class CVideoMFCDlg : public CDialogEx
{
// Construction
public:
	CVideoMFCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VIDEOMFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	IplImage* image;
//	CvCapture* capture;
	CString filePath;
	afx_msg void OnBnClickedButton1();
private:
//	void OnTimer(UINT nIDEvent);
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnClose();
};
