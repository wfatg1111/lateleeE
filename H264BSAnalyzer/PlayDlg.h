#pragma once

class CH264BSAnalyzerDlg;

// CPlayDlg dialog

class CPlayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPlayDlg)

public:
	CPlayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlayDlg();

// Dialog Data
	enum { IDD = IDD_PLAYDLG };

public:
    void SetParentWnd(CH264BSAnalyzerDlg* pWnd) {m_pParentWnd = pWnd;}
private:
    CH264BSAnalyzerDlg *m_pParentWnd; // ���ڲ�������

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
