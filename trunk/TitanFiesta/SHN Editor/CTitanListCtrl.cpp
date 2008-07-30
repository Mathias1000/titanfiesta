#include "stdafx.h"
#include "SHN Editor.h"
#include "CDlg.hpp"

CTitanListCtrl::CTitanListCtrl()
{
}

CTitanListCtrl::~CTitanListCtrl()
{
}


BEGIN_MESSAGE_MAP(CTitanListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfoList)
	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

BOOL CTitanListCtrl::EnsureSubItemVisible(int nItem, int nSubItem, CRect *pRect){
	BOOL ret = EnsureVisible(nItem, FALSE);
	CRect rect;
	GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
	CRect rtList;
	GetClientRect(&rtList);
	if(rect.right > rtList.Width()) Scroll(CSize( rect.Width() > rtList.Width()?rect.left : rect.right - rtList.Width(), 0));
	if(rect.left < 0) Scroll(CSize(rect.left));
	if(pRect){
		GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rect);
		rect.right = min(rect.right, rtList.Width()-4);
		*pRect = rect;
	}
	return ret;
}

BOOL CTitanListCtrl::DisplayEditor(int nItem, int nSubItem){
	if(!IsWindow(m_wndEdit.m_hWnd)){		
		m_wndEdit.Create(~WS_VISIBLE & WS_CHILD, CRect(10, 10, 100, 100), this, 1337);
	}
	CRect rectSubItem;
	EnsureSubItemVisible(nItem, nSubItem, &rectSubItem);

	CString text =  GetItemText(nItem, nSubItem);
	m_wndEdit.SetWindowPos(NULL, rectSubItem.left, rectSubItem.top, rectSubItem.Width(), rectSubItem.Height(), SWP_SHOWWINDOW);	
	return TRUE;
}

void CTitanListCtrl::HideEditor(BOOL bUpdate){
}

BOOL CTitanListCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	if(!pNMListView) return FALSE;
	int nItem = pNMListView->iItem, nSubItem = pNMListView->iSubItem;		

	*pResult = DisplayEditor(nItem, nSubItem);

	return *pResult;
}

void CTitanListCtrl::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult){
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int itemid = pItem->iItem;
	if(pItem->mask & LVIF_TEXT){
		CString text;
		CSHN::SSHNRow* curRow = curFile->rows[pItem->iItem];
		CSHN::SSHNRowData* curData = curRow->data[pItem->iSubItem];
		CSHN::SSHNColumn* curCol = curFile->columns[pItem->iSubItem];
		switch(curCol->type){
			case 1:
				sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", curData->data[0]);
			break;
			case 2:
				sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", *reinterpret_cast<word*>(curData->data));
			break;
			case 3:
			case 11:
			case 18:
			case 27:
				sprintf_s(pItem->pszText, pItem->cchTextMax, "%d", *reinterpret_cast<dword*>(curData->data));
			break;
			case 9:
			case 26:
				sprintf_s(pItem->pszText, pItem->cchTextMax, "%s", reinterpret_cast<char*>(curData->data));
			break;
		}
	}

	*pResult = 0;
}
