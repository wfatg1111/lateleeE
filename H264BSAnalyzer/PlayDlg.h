#pragma once

#include <vector>
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
    int SetVideoInfo(CString strFileName, int nWidth, int nHeight, int nTotalFrame, float nFps);
    void ShowFirstFrame();
    void SetBlack();
private:
    BOOL m_fShowBlack;
    BOOL m_fPlayed;
    BOOL m_fClosed;
    BOOL m_fLoop;
    BOOL m_fInit;
    INT m_nWidth;
    INT m_nHeight;
    INT m_nTotalFrame;
    INT m_nFrameCount;
    float m_fFps;
    BYTE* m_pbBmpData;
    INT m_iBmpSize;
    CString m_strFileUrl;   // ��Ƶ�ļ�

    CH264Decoder m_cDecoder;    // ������

    std::vector<std::vector<int> > m_vStartX;

    void ShowPicture(BYTE* pbData, int iSize);
    void Show(BYTE* pbData, int nSize, int nWidth, int nHeight);

    void ShowingFrame();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnBnClickedBtPlay();
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CButton m_bPlay;
    CButton m_bSaveFrame;
    CButton m_bStop;
    CButton m_bNextFrame;
    afx_msg void OnBnClickedBtNext();
    afx_msg void OnBnClickedBtSave();
    afx_msg void OnBnClickedBtStop();
    afx_msg void OnBnClickedCkLoop();
};
