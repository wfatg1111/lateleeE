// PlayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "H264BSAnalyzer.h"
#include "PlayDlg.h"
#include "afxdialogex.h"


// CPlayDlg dialog

IMPLEMENT_DYNAMIC(CPlayDlg, CDialogEx)

CPlayDlg::CPlayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPlayDlg::IDD, pParent)
{
    m_fShowBlack = FALSE;
    m_nWidth = 0;
    m_nHeight = 0;
    m_nTotalFrame = 0;
    m_nFrameCount = 0;
    m_fFps = 0.0;
    m_pbBmpData = NULL;
    m_strFileUrl.Empty();
    m_pParentWnd = NULL;
}

CPlayDlg::~CPlayDlg()
{
}

void CPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

inline void RenderBitmap(CWnd *pWnd, Bitmap* pbmp)
{
    RECT rect;
    pWnd->GetClientRect( &rect );

    Graphics grf( pWnd->m_hWnd );
    if ( grf.GetLastStatus() == Ok )
    {
        Rect rc( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );

        grf.DrawImage(pbmp, rc);
    }
}

// ��ʾͼƬ
void CPlayDlg::ShowPicture(BYTE* pbData, int iSize)
{
    if (pbData == NULL) return;
    CWnd* pWnd=GetDlgItem(IDC_VIDEO);   // IDC_VIDEO��picture contral �ؼ�ID
    IStream* pPicture = NULL;
    CreateStreamOnHGlobal(NULL,TRUE,&pPicture);
    if( pPicture != NULL )
    {
        pPicture->Write(pbData,  iSize, NULL);
        LARGE_INTEGER liTemp = { 0 };
        pPicture->Seek(liTemp, STREAM_SEEK_SET, NULL);
        Bitmap TempBmp(pPicture);
        RenderBitmap(pWnd, &TempBmp);
    }
    if(pPicture != NULL)
    {
        pPicture->Release();
        pPicture = NULL;
    }

    m_fShowBlack = FALSE;
}

void CPlayDlg::Show(BYTE* pbData, int nSize, int nWidth, int nHeight)
{
    MYBITMAPFILEHEADER bmpHeader;
    MYBITMAPINFOHEADER bmpInfo;
    //BYTE* pbBmpData = NULL;
    //static BYTE pbBmpData[1920*1080*3 + 54] = {};
    // �����BMPͷ
    bmpHeader.bfType = 'MB';
    bmpHeader.bfSize = nSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpHeader.bfReserved1 = 0;
    bmpHeader.bfReserved2 = 0;
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpInfo.biSize   = sizeof(BITMAPINFOHEADER);
    bmpInfo.biWidth  = nWidth;
    bmpInfo.biHeight = -nHeight;    // note BMPͼ���ǵ�������
    bmpInfo.biPlanes = 1;
    bmpInfo.biBitCount = 24;
    bmpInfo.biCompression = BI_RGB;
    bmpInfo.biSizeImage   = 0;//nSize;
    bmpInfo.biXPelsPerMeter = 0;
    bmpInfo.biYPelsPerMeter = 0;
    bmpInfo.biClrUsed = 0;
    bmpInfo.biClrImportant = 0;

    memcpy(m_pbBmpData, &bmpHeader, sizeof(BITMAPFILEHEADER));
    memcpy(m_pbBmpData+sizeof(BITMAPFILEHEADER), &bmpInfo, sizeof(BITMAPINFOHEADER));
    memcpy(m_pbBmpData+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), pbData, nSize);
    // ��ʾ
    ShowPicture(m_pbBmpData, bmpHeader.bfSize);
}

void CPlayDlg::SetVideoInfo(CString strFileName, int nWidth, int nHeight, int nTotalFrame, float nFps)
{
    m_strFileUrl = strFileName;
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nTotalFrame = nTotalFrame;
    m_fFps = nFps;

    m_pbBmpData = new BYTE[m_nWidth * m_nHeight * 3 + 54];
}

BEGIN_MESSAGE_MAP(CPlayDlg, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BT_PLAY, &CPlayDlg::OnBnClickedBtPlay)
    ON_WM_CLOSE()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CPlayDlg message handlers
BOOL CPlayDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��

    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣: OCX ����ҳӦ���� FALSE
}


void CPlayDlg::OnPaint()
{
    //picture�ؼ�����ɫΪ��ɫ
    if (m_fShowBlack)
    {
        CRect rtTop;
        CStatic *pWnd = (CStatic*)GetDlgItem(IDC_VIDEO);
        CDC *cDc = pWnd->GetDC();
        pWnd->GetClientRect(&rtTop);
        cDc->FillSolidRect(rtTop.left, rtTop.top, rtTop.Width(), rtTop.Height(),RGB(0,0,0));
        Invalidate(FALSE);
    }

    CPaintDC dc(this); // device context for painting
    // TODO: �ڴ˴������Ϣ����������
    // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
}

// ����
void CPlayDlg::OnBnClickedBtPlay()
{
    if (m_strFileUrl.IsEmpty())
    {
        MessageBox("Sorry, you open no file.");
        return;
    }

    m_cDecoder.openVideoFile(m_strFileUrl.GetBuffer());

    // ��ʼ����ʱ��
    SetTimer(1,(UINT)((float)1000/m_fFps),NULL);
}

// ���ڹر�
void CPlayDlg::OnClose()
{
    // �ͷš��ָ�
    m_cDecoder.closeVideoFile();
    m_nFrameCount = 0;

    // �������Ȳ��ͷ�BMPͼƬ����
    KillTimer(1);

    CDialogEx::OnClose();
}

void CPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
    BYTE* pRgbBuffer = NULL;
    int nSize = 0;
    int nWidth = 0;
    int nHeight = 0;
    int ret = 0;
    CString strTittle;

    ret = m_cDecoder.getFrame(NULL, &pRgbBuffer, &nSize, &nWidth, &nHeight);
    if (ret < 0) return;
    else if ( ret > 0)
    {
        m_nFrameCount++;
        strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
        this->SetWindowText(strTittle);
        Show(pRgbBuffer, nSize, nWidth, nHeight);
    }
    else
    {
        ret = m_cDecoder.getSkippedFrame(NULL, &pRgbBuffer, &nSize, &nWidth, &nHeight);
        if ( ret > 0)
        {
            m_nFrameCount++;
            strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
            this->SetWindowText(strTittle);
            Show(pRgbBuffer, nSize, nWidth, nHeight);
        }
    }

    if (m_nFrameCount >= m_nTotalFrame)
    {
        KillTimer(1);
    }
}
