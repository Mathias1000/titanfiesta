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
	void HideEditor(BOOL bUpdate = TRUE);

	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
