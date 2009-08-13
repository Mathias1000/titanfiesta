/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */


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
	ON_COMMAND(ID_FILE_OPEN32772, &CDlg::OnFileOpen)
END_MESSAGE_MAP()

bool isReady = false;
CShn* curFile = NULL;
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

void CDlg::OnFileOpen(){
	CFileDialog FileDlg(TRUE, ".shn", NULL, 0, "SHN Files (*.shn)|*.shn||");

	if( FileDlg.DoModal() != IDOK ) return;

	BSTR fileTitle = FileDlg.GetFileTitle().AllocSysString();
	BSTR fileName = (FileDlg.GetFolderPath() + "\\" + FileDlg.GetFileName()).AllocSysString();

	char temp2[64];
	char tempPath[MAX_PATH];
	wcstombs_s(NULL, temp2, 64, fileTitle, 64);
	sprintf_s(tempPath, MAX_PATH, "%s - SHN Editor", temp2);
	SetWindowText(tempPath);	
	wcstombs_s(NULL, tempPath, MAX_PATH, fileName, MAX_PATH);
	SysFreeString(fileName);
	SysFreeString(fileTitle);

	DEL(curFile);
	curFile = new CShn();
	if (!curFile->Open(tempPath)) {
		MessageBoxA("There was an error loading the SHN.", "ERROR LOADING SHN");
		return;
	}

	lstData.DeleteAllItems();
	lstData.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	char temp[64];
	for(dword i = 0; i < curFile->ColCount(); i++){
		CShnColumn* curCol = curFile->Column(i);
		sprintf_s(temp, 64, "%s [%d] [%d]", curCol->name, curCol->type, curCol->columnSize);
		lstData.InsertColumn(i, temp, LVCFMT_LEFT, 100);
	}

	lstData.SetItemCount(curFile->RowCount());
}
