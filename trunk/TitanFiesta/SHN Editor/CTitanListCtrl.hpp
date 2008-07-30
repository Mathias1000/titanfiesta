#pragma once

class CTitanListCtrl : public CListCtrl
{
public:
	CTitanListCtrl();

public:
	virtual ~CTitanListCtrl();

protected:
	CEdit m_wndEdit;

	BOOL EnsureSubItemVisible(int nItem, int nSubItem, CRect *pRect);

	BOOL DisplayEditor(int nItem, int nSubItem);
	void HideEditor();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void ShowColumnInHex();

	DECLARE_MESSAGE_MAP()

	int nCurEditItem;
	int nCurEditSubItem;

	int curEditColumn;
};
