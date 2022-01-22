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
    DDX_Control(pDX, IDC_BT_PLAY, m_bPlay);
    DDX_Control(pDX, IDC_BT_SAVE, m_bSaveFrame);
    DDX_Control(pDX, IDC_BT_STOP, m_bStop);
    DDX_Control(pDX, IDC_BT_NEXT, m_bNextFrame);
}

BEGIN_MESSAGE_MAP(CPlayDlg, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BT_PLAY, &CPlayDlg::OnBnClickedBtPlay)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BT_NEXT, &CPlayDlg::OnBnClickedBtNext)
    ON_BN_CLICKED(IDC_BT_SAVE, &CPlayDlg::OnBnClickedBtSave)
    ON_BN_CLICKED(IDC_BT_STOP, &CPlayDlg::OnBnClickedBtStop)
END_MESSAGE_MAP()


// CPlayDlg message handlers
BOOL CPlayDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // ��ͼ
    m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PLAY)));
    m_bPlay.EnableWindow(FALSE);
    m_bStop.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_STOP)));
    m_bStop.EnableWindow(FALSE);
    m_bNextFrame.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_NEXT)));
    m_bNextFrame.EnableWindow(FALSE);
    m_bSaveFrame.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_SAVE)));
    m_bSaveFrame.EnableWindow(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣: OCX ����ҳӦ���� FALSE
}


void CPlayDlg::OnPaint()
{
    //picture�ؼ�����ɫΪ��ɫ
    //if (m_fShowBlack)
    if (0)
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
    int ret = 0;
    m_strFileUrl = strFileName;
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nTotalFrame = nTotalFrame;
    m_fFps = nFps;

    //return;

    if (m_fFps <= 0 ) m_fFps = 25.0;

    m_pbBmpData = new BYTE[m_nWidth * m_nHeight * 3 + 54];
    if (m_pbBmpData == NULL)
    {
        MessageBox("Malloc buffer for RGB data failed.");
        return;
    }
    if (m_strFileUrl.IsEmpty())
    {
        MessageBox("Sorry, you open no file...");
        return;
    }

    ret = m_cDecoder.openVideoFile(m_strFileUrl.GetBuffer());
    if (ret < 0)
    {
        MessageBox("Sorry, open video decoder failed.");
        return;
    }

    BYTE* pRgbBuffer = NULL;
    int nSize = 0;
    CString strTittle;

    Sleep(1*1000);
    while (m_cDecoder.getFrame(NULL, &pRgbBuffer, &nSize) > 0)
    {
        m_nFrameCount++;
        strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
        this->SetWindowText(strTittle);
        Show(pRgbBuffer, nSize, m_nWidth, m_nHeight);
        break;
    }

    // ��ͼ
    m_bPlay.EnableWindow(TRUE);
    m_bStop.EnableWindow(TRUE);
    m_bNextFrame.EnableWindow(TRUE);
    m_bSaveFrame.EnableWindow(TRUE);
}

// ���ڹر�
// �������Ȳ��ͷ�BMPͼƬ����
void CPlayDlg::OnClose()
{
    // �ͷš��ָ�
    m_cDecoder.closeVideoFile();
    m_nFrameCount = 0;

    // ֹͣ��ʱ��
    KillTimer(1);

    CDialogEx::OnClose();
}

void CPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
    BYTE* pRgbBuffer = NULL;
    int nSize = 0;
    int ret = 0;
    CString strTittle;

    ret = m_cDecoder.getFrame(NULL, &pRgbBuffer, &nSize);
    if (ret < 0) return;
    else if ( ret > 0)
    {
        m_nFrameCount++;
        strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
        this->SetWindowText(strTittle);
        Show(pRgbBuffer, nSize, m_nWidth, m_nHeight);
    }
    else
    {
        ret = m_cDecoder.getSkippedFrame(NULL, &pRgbBuffer, &nSize);
        if ( ret > 0)
        {
            m_nFrameCount++;
            strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
            this->SetWindowText(strTittle);
            Show(pRgbBuffer, nSize, m_nWidth, m_nHeight);
        }
    }

    if (m_nFrameCount >= m_nTotalFrame)
    {
        KillTimer(1);
    }
}

void CPlayDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // TODO: �ڴ˴������Ϣ����������
}

// ����
void CPlayDlg::OnBnClickedBtPlay()
{
    if (m_strFileUrl.IsEmpty())
    {
        MessageBox("Sorry, you open no file.");
        return;
    }
#if 0
    BYTE* pRgbBuffer = NULL;
    int nSize = 0;
    int ret = 0;
    CString strTittle;

    while (m_cDecoder.getFrame(NULL, &pRgbBuffer, &nSize) > 0)
    {
        m_nFrameCount++;
        strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
        this->SetWindowText(strTittle);
        Show(pRgbBuffer, nSize, m_nWidth, m_nHeight);
        break;
    }
#endif
    // ��ʼ����ʱ��
    SetTimer(1,(UINT)((float)1000/m_fFps),NULL);
}

void CPlayDlg::OnBnClickedBtStop()
{
    KillTimer(1);
}

void CPlayDlg::OnBnClickedBtNext()
{
    KillTimer(1);

    BYTE* pRgbBuffer = NULL;
    int nSize = 0;
    CString strTittle;

    Sleep(1*1000);
    while (m_cDecoder.getFrame(NULL, &pRgbBuffer, &nSize) > 0)
    {
        m_nFrameCount++;
        strTittle.Format("%d/%d  %0.2ffps --  %s", m_nFrameCount, m_nTotalFrame, m_fFps, DLG_TITTLE);
        this->SetWindowText(strTittle);
        Show(pRgbBuffer, nSize, m_nWidth, m_nHeight);
        break;
    }
}


void CPlayDlg::OnBnClickedBtSave()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
}
