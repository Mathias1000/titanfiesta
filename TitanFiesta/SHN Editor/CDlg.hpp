/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */


// CDlg.hpp : header file
//

#pragma once
typedef unsigned __int8 byte;
typedef unsigned __int16 word;
typedef unsigned __int32 dword;
#define DEL(x) if(x != NULL){ delete x; x = NULL; }
#define DELARR(x) if(x != NULL){ delete [] x; x = NULL; }
#define DELVEC(x) for(dword i = 0; i < x.size(); i++){DEL(x.at(i));} x.clear();
#define DELVECARR(x) for(dword i = 0; i < x.size(); i++){DELARR(x.at(i));} x.clear();

//#include "CListCtrlEx/listctrlex.h"
#include <vector>
#include "CTitanFile.hpp"
#include "CSHN.hpp"
#include "CTitanListCtrl.hpp"

extern CSHN* curFile;

// CDlg dialog
class CDlg : public CDialog
{
// Construction
public:
	CDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SHNEDITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CTitanListCtrl lstData;
	CEdit m_wndEdit;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	static BOOL InitEditor(CWnd** pWnd, int nRow, int nColumn, CString& strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate);
	static BOOL EndEditor(CWnd** pWnd, int nRow, int nColumn, CString& strSubItemText, DWORD_PTR dwItemData, void* pThis, BOOL bUpdate);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();
};
