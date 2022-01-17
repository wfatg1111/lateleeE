#pragma once

#include "H264Decode.h"

#define DLG_TITTLE "Play"

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
    // ���������õ���Ƶ�ļ���Ϣ
    void SetVideoInfo(CString strFileName, int nWidth, int nHeight, int nTotalFrame, float nFps);

private:
    BOOL m_fShowBlack;
    INT m_nWidth;
    INT m_nHeight;
    INT m_nTotalFrame;
    INT m_nFrameCount;
    float m_fFps;
    BYTE* m_pbBmpData;
    CString m_strFileUrl;   // ��Ƶ�ļ�

    CH264Decoder m_cDecoder;    // ������

    void ShowPicture(BYTE* pbData, int iSize);
    void Show(BYTE* pbData, int nSize, int nWidth, int nHeight);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnBnClickedBtPlay();
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();
};
