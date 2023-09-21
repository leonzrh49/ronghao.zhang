
// VideoMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoMFC.h"
#include "VideoMFCDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRect rect;
CDC *pDC;
HDC hDC;
CWnd *pwnd;

CRect rect2;
CDC *pDC2;
HDC hDC2;
CWnd *pwnd2;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVideoMFCDlg dialog

IplImage* mat2IplImage(Mat image)
{
	IplImage* ipl_img = cvCreateImage(cvSize(image.cols, image.rows), IPL_DEPTH_8U, image.channels());
	memcpy(ipl_img->imageData, image.data, ipl_img->height * ipl_img->widthStep);
	return ipl_img;
}


CVideoMFCDlg::CVideoMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoMFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVideoMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVideoMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CVideoMFCDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &CVideoMFCDlg::OnBnClickedButton2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()
Edge *E;

int *p, *rango, *C, *COL;
int nRows;
int nColumns;
double *Int;
double thao = 100000.0;

void MST_Segmentation(Image **I, int channel)
{
	int i, j, k, r, c;
	int u, v, set1, set2, newSet;
	int C1, C2;
	int nEdges = 0;
	int r1, c1, r2, c2;
	double Int1, Int2;
	double MInt, cost;

	E = new Edge[5 * nRows*nColumns];
	p = new int[5 * nRows*nColumns];
	rango = new int[5 * nRows*nColumns];
	Int = new double[5 * nRows*nColumns];
	C = new int[5 * nRows*nColumns];
	COL = new int[5 * nRows*nColumns];

	for (i = 0; i < nRows; i++)
	{
		for (j = 0; j < nColumns; j++)
		{
			k = i * nColumns + j;

			make_set(k);

			COL[k] = -1;
			Int[k] = 0.0;
			C[k] = 1;

			for (r = i - 1; r <= i; r++)
			{
				for (c = j - 1; c <= j + 1; c++)
				{
					if (r == i && c == j)
						continue;

					if (r < 0 || c < 0 || c >= nColumns || r >= nRows)
						continue;

					E[nEdges].v1 = i * nColumns + j;
					E[nEdges].v2 = r * nColumns + c;
					E[nEdges].w = fabs(I[i][j].val[channel] - I[r][c].val[channel]);

					nEdges++;
				}
			}
		}
	}

	qsort(E, nEdges, sizeof(E[0]), compare);

	//printf("nEdges: %d\n", nEdges);

	for (i = 0; i < nEdges; i++)
	{
		u = E[i].v1;
		v = E[i].v2;
		cost = E[i].w;

		set1 = find_set(u);
		set2 = find_set(v);
		Int1 = Int[set1];
		Int2 = Int[set2];
		C1 = C[set1];
		C2 = C[set2];

		MInt = Min(Int1 + thao / C1, Int2 + thao / C2);

		if (set1 != set2 && cost < MInt)
		{
			union_set(u, v);

			newSet = find_set(u);

			Int[newSet] = Max(Int1, Int2);
			Int[newSet] = Max(Int[newSet], cost);
			C[newSet] = C1 + C2;
		}
	}

	for (i = 0; i < nRows*nColumns; i++)
	{
		r1 = i / nColumns;
		c1 = i % nColumns;

		k = find_set(i);

		if (COL[k] == -1)
		{
			COL[k] = i;

			I[r1][c1].val[RED] = rand() % 255;
			I[r1][c1].val[GREEN] = rand() % 255;
			I[r1][c1].val[BLUE] = rand() % 255;
			I[r1][c1].val[INTENSITY] = (I[r1][c1].val[RED] + I[r1][c1].val[GREEN] + I[r1][c1].val[BLUE]) / 3.0;
		}
		else
		{
			r2 = COL[k] / nColumns;
			c2 = COL[k] % nColumns;

			I[r1][c1].val[RED] = I[r2][c2].val[RED];
			I[r1][c1].val[GREEN] = I[r2][c2].val[GREEN];
			I[r1][c1].val[BLUE] = I[r2][c2].val[BLUE];
			I[r1][c1].val[INTENSITY] = I[r2][c2].val[INTENSITY];
		}

	}
}

void Smooth(Image **S, Image **D, int channel)
{
	int i, j;
	int r, c;
	double sum, k;

	double M[5][5] = {
		{2, 4, 5, 4, 2},
		{4, 9, 12, 9, 4},
		{5, 12, 15, 12, 5},
		{4, 9, 12, 9, 4},
		{2, 4, 5, 4, 2}
	};

	for (i = 0; i < nRows; i++)
	{
		for (j = 0; j < nColumns; j++)
		{
			k = 0.0;
			sum = 0.0;

			for (r = i - 2; r <= i + 2; r++)
			{
				for (c = j - 2; c <= j + 2; c++)
				{
					if (r < 0 || r >= nRows || c < 0 || c >= nColumns)
						continue;

					k += S[r][c].val[channel] * M[r - i + 2][c - j + 2];
					sum += M[r - i + 2][c - j + 2];
				}
			}

			D[i][j].val[channel] = k / sum;
		}
	}
}

Image ** ConvImgToDbl(Mat img)
{
	Scalar color;
	Image **T;
	int i, j;

	T = new Image *[img.rows];
	for (i = 0; i < img.rows; i++)
	{
		T[i] = new Image[img.cols];
		for (j = 0; j < img.cols; j++)
		{
			//color = cvGet2D(img, i, j);

			color = img.at<Vec3b>(i, j);

			T[i][j].val[RED] = color(2);
			T[i][j].val[GREEN] = color(1);
			T[i][j].val[BLUE] = color(0);
			T[i][j].val[INTENSITY] = (color(2) + color(1) + color(0)) / 3.0;
		}
	}

	return T;
}

void ConvDblToImg(Image **I, Mat img, bool grayScale)
{
	int i, j;
	int r, g, b;

	for (i = 0; i < nRows; i++)
	{
		for (j = 0; j < nColumns; j++)
		{
			if (grayScale == true)
				r = g = b = (int)I[i][j].val[INTENSITY];
			else
			{
				r = (int)I[i][j].val[RED];
				g = (int)I[i][j].val[GREEN];
				b = (int)I[i][j].val[BLUE];
			}

			img.at<Vec3b>(i, j) = Vec3b(r, g, b);
		}
	}
}

void make_set(int x)
{
	p[x] = x;
	rango[x] = 0;
}

void link(int x, int y)
{
	if (rango[x] > rango[y])
		p[y] = x;
	else
	{
		p[x] = y;
		if (rango[x] == rango[y])
			rango[y] = rango[y] + 1;
	}
}

int find_set(int x)
{
	if (x != p[x])
		p[x] = find_set(p[x]);
	return p[x];
}

void union_set(int x, int y)
{
	link(find_set(x), find_set(y));
}

int Min(double a, double b)
{
	if (a < b)
		return a;
	else
		return b;
}

int Max(double a, double b)
{
	if (a > b)
		return a;
	else
		return b;
}

int compare(const void *a, const void *b)
{
	Edge *sp1 = (Edge *)a;
	Edge *sp2 = (Edge *)b;

	if (sp1->w < sp2->w)
		return -1;
	else if (sp1->w > sp2->w)
		return 1;

	return 0;
}

// CVideoMFCDlg message handlers

BOOL CVideoMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	pwnd = GetDlgItem(IDC_ShowImage);
	//pwnd->MoveWindow(35,30,352,288);
    pDC =pwnd->GetDC();
	//pDC =GetDC();
    hDC= pDC->GetSafeHdc();
	pwnd->GetClientRect(&rect);

	pwnd2 = GetDlgItem(IDC_ShowImage2);
	//pwnd->MoveWindow(35,30,352,288);
	pDC2 = pwnd2->GetDC();
	//pDC =GetDC();
	hDC2 = pDC2->GetSafeHdc();
	pwnd2->GetClientRect(&rect2);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVideoMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVideoMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVideoMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVideoMFCDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
		//--------------选择照片----------------------------
	/*CString filePath;*/ //保存打开文件的路径
	CString defaultDir = NULL;// _T("D:\\"); //设置默认打开文件夹
	CString fileFilter = _T("文件(*.jpg;*.bmp)|*.jpg;*.bmp|All File (*.*)|*.*||"); //设置文件过滤
	CFileDialog fileDlg(true, defaultDir, _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, fileFilter, NULL);

	//弹出选择文件对话框
	if (fileDlg.DoModal() == IDOK)
	{
		filePath = fileDlg.GetPathName();//得到完整的文件名和目录名拓展名  
		CString filename = fileDlg.GetFileName();
	}

	//g_srcImage = imread((LPCSTR)filePath);

	Mat frame = imread((LPCSTR)filePath); //imread("6.png");
	// 测试
	IplImage* m_Frame;
	m_Frame = mat2IplImage(frame);

	CvvImage m_CvvImage;
	m_CvvImage.CopyOf(m_Frame,1);	
	if (true)
	{
		m_CvvImage.DrawToHDC(hDC, &rect);
	}

	// 设置计时器,每10ms触发一次事件
	//SetTimer(1,10,NULL);
}


void CVideoMFCDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	Mat frame2 = imread((LPCSTR)filePath); //imread("6.png");


	//---------------------
	int i, j;

	Image **X;
	Image **imgI;

	Mat imgSrc;
	Mat imgDst;
	//Mat imgGray;


	srand(time(NULL));
	imgSrc = frame2;// imread("6.png");
	imgDst = imgSrc.clone();
	//imgGray = imgSrc.clone();

	nRows = imgSrc.rows;
	nColumns = imgSrc.cols;

	X = ConvImgToDbl(imgSrc);
	imgI = ConvImgToDbl(imgSrc);

	//ConvDblToImg(X, imgGray, true);

	Smooth(X, imgI, INTENSITY);

	MST_Segmentation(imgI, INTENSITY);

	ConvDblToImg(imgI, imgDst, false);

	//imwrite("mst_tiger_2.jpg", imgDst);

	//imwrite("mstG3.jpg", imgGray);


	// 测试
	IplImage* m_Frame2;
	m_Frame2 = mat2IplImage(imgDst);

	CvvImage m_CvvImage2;
	m_CvvImage2.CopyOf(m_Frame2, 1);
	if (true)
	{
		m_CvvImage2.DrawToHDC(hDC2, &rect2);
	}
}


void CVideoMFCDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CDialogEx::OnClose();
}
