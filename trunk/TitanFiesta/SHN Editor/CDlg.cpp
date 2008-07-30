
// CDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SHN Editor.h"
#include "CDlg.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDlg dialog




CDlg::CDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DATA, lstData);
	DDX_Control(pDX, IDC_EDIT, m_wndEdit);
}

BEGIN_MESSAGE_MAP(CDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool isReady = false;
CSHN* curFile;
// CDlg message handlers
void CDlg::OnSize(UINT nType, int cx, int cy){
	CDialog::OnSize(nType, cx, cy);
	
	if(nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED && isReady)
		lstData.MoveWindow(0,0,cx,cy);
}

BOOL CDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	curFile = new CSHN();
	curFile->Open("ItemInfo.shn");

	//lstData.Reset();
	//lstData.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	//lstData.SetDefaultEditor(NULL, NULL, &m_wndEdit);
	//ListView_SetExtendedListViewStyle(lstData.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	char temp[64];
	for(dword i = 0; i < curFile->columns.size(); i++){
		CSHN::SSHNColumn* curCol = curFile->columns[i];
		sprintf_s(temp, 64, "%s [%d]", curCol->name, curCol->type);
		lstData.InsertColumn(i, temp, LVCFMT_LEFT, 100);
	}

	lstData.SetItemCount(curFile->rows.size());
	/*for(dword i = 0; i < curFile->rows.size(); i++){
		CSHN::SSHNRow* curRow = curFile->rows[i];

		for(dword j = 0; j < curRow->data.size(); j++){
			CSHN::SSHNRowData* curData = curRow->data[j];
			switch(curFile->columns[j]->type){
				case 1:
					sprintf_s(temp, 64, "%d", curData->data[0]);
				break;
				case 2:
					sprintf_s(temp, 64, "%d", *reinterpret_cast<word*>(curData->data));
				break;
				case 3:
				case 11:
				case 18:
				case 27:
					sprintf_s(temp, 64, "%d", *reinterpret_cast<dword*>(curData->data));
				break;
				case 9:
				case 26:
					sprintf_s(temp, 64, "%s", reinterpret_cast<char*>(curData->data));
				break;
			};
			if(j == 0){
				lstData.InsertItem(i, (LPCTSTR)NULL);
			}else{
				//lstData.SetItemText(i, j, temp);
			}
		}
	}*/
	isReady = true;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
