
// H264BSAnalyzerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "H264BSAnalyzer.h"
#include "H264BSAnalyzerDlg.h"
#include "afxdialogex.h"

#include "NaLParse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    RECT m_pRectLink;
public:
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


// CH264BSAnalyzerDlg dialog




CH264BSAnalyzerDlg::CH264BSAnalyzerDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CH264BSAnalyzerDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CH264BSAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    //DDX_Control(pDX, IDC_H264_INPUTURL, m_h264InputUrl);
    DDX_Control(pDX, IDC_H264_NALINFO, m_h264NalInfo);
    DDX_Control(pDX, IDC_H264_NALLIST, m_h264NalList);
    DDX_Control(pDX, IDC_EDIT_FILE, m_edFileUrl);
    DDX_Control(pDX, IDC_EDIT_HEX, m_edHexInfo);
}

BEGIN_MESSAGE_MAP(CH264BSAnalyzerDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_H264_INPUTURL_OPEN, &CH264BSAnalyzerDlg::OnBnClickedH264InputurlOpen)
    ON_WM_DROPFILES()
    ON_NOTIFY(LVN_ITEMACTIVATE, IDC_H264_NALLIST, &CH264BSAnalyzerDlg::OnLvnItemActivateH264Nallist)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_H264_NALLIST, &CH264BSAnalyzerDlg::OnNMCustomdrawH264Nallist)
    ON_COMMAND(ID_FILE_OPEN32771, &CH264BSAnalyzerDlg::OnFileOpen)
    ON_COMMAND(ID_HELP_ABOUT, &CH264BSAnalyzerDlg::OnHelpAbout)
    ON_COMMAND(ID_HOWTO_USAGE, &CH264BSAnalyzerDlg::OnHowtoUsage)
END_MESSAGE_MAP()


void CH264BSAnalyzerDlg::SystemClear()
{
    m_vNalInfoVector.clear();
    m_h264NalList.DeleteAllItems();
    m_nNalIndex = 1;
}

//���һ����¼
int CH264BSAnalyzerDlg::ShowNLInfo(NALU_t* nalu)
{
    //���ѡ���ˡ�������5000�������ж��Ƿ񳬹�5000��
    //if(m_vh264nallistmaxnum.GetCheck()==1&&nl_index>5000){
    //    return 0;
    //}

    CString strTempIndex;
    CString strOffset;
    CString strNalLen;
    CString strStartCode;
    CString strNalUnitType;
    CString strNalInfo;
    int nIndex=0;

    // NAL��Ԫ����
    switch (nalu->nal_unit_type)
    {
    case 0:
        strNalUnitType.Format(_T("Unspecified"));
        break;
    case 1:
        strNalUnitType.Format(_T("Coded slice of a non-IDR picture"));
        switch (nalu->slice_type)
        {
        case 0:
        case 5:
            strNalInfo.Format(_T("P Slice"));
            break;
        case 1:
        case 6:
            strNalInfo.Format(_T("B Slice"));
            break;
        case 2:
        case 7:
            strNalInfo.Format(_T("I Slice"));
            break;
        }
        break;
    case 2:
        strNalUnitType.Format(_T("DPA"));
        break;
    case 3:
        strNalUnitType.Format(_T("DPB"));
        break;
    case 4:
        strNalUnitType.Format(_T("DPC"));
        break;
    case 5:
        strNalUnitType.Format(_T("Coded slice of an IDR picture"));
        strNalInfo.Format(_T("IDR"));
        break;
    case 6:
        strNalUnitType.Format(_T("SEI"));
        strNalInfo.Format(_T("SEI"));
        break;
    case 7:
        strNalUnitType.Format(_T("Sequence parameter set"));
        strNalInfo.Format(_T("SPS"));
        break;
    case 8:
        strNalUnitType.Format(_T("Picture parameter set"));
        strNalInfo.Format(_T("PPS"));
        break;
    case 9:
        strNalUnitType.Format(_T("Access UD"));
        strNalInfo.Format(_T("AUD"));
        break;
    case 10:
        strNalUnitType.Format(_T("END_SEQUENCE"));
        break;
    case 11:
        strNalUnitType.Format(_T("END_STREAM"));
        break;
    case 12:
        strNalUnitType.Format(_T("FILLER_DATA"));
        break;
    case 13:
        strNalUnitType.Format(_T("SPS_EXT"));
        break;
    case 19:
        strNalUnitType.Format(_T("AUXILIARY_SLICE"));
        break;
    default:
        strNalUnitType.Format(_T("Other"));
        break;
    }

    // ���
    strTempIndex.Format(_T("%d"),m_nNalIndex);
    // ����ƫ��
    strOffset.Format(_T("%08x"), nalu->data_offset);
    // ����
    strNalLen.Format(_T("%d"),nalu->total_len);
    // ��ʼ��
    strStartCode.Format(_T("%s"), nalu->startcode_buf);

    //��ȡ��ǰ��¼����
    nIndex=m_h264NalList.GetItemCount();
    //���С����ݽṹ
    LV_ITEM lvitem;
    lvitem.mask=LVIF_TEXT;
    lvitem.iItem=nIndex;
    lvitem.iSubItem=0;
    //ע��vframe_index������ֱ�Ӹ�ֵ��
    //���ʹ��f_indexִ��Format!�ٸ�ֵ��
    lvitem.pszText=(char *)(LPCTSTR)strTempIndex;
    //------------------------
    //���vector��¼��nal��λ����Ϣ
    //ʹ�������ǿ��Ի�ȡ��NAL����ϸ��Ϣ
    //����Ҫ�洢������ʼ��ĳ���
    //��ʼ��ԭ������NAL��һ����
    NALInfo nalinfo;
    nalinfo.data_lenth = nalu->total_len;
    nalinfo.data_offset = nalu->data_offset;
    m_vNalInfoVector.push_back(nalinfo);

    //------------------------��ʾ��List��
    m_h264NalList.InsertItem(&lvitem);
    m_h264NalList.SetItemText(nIndex,1,strOffset);
    m_h264NalList.SetItemText(nIndex,2,strNalLen);
    m_h264NalList.SetItemText(nIndex,3,strStartCode);
    m_h264NalList.SetItemText(nIndex,4,strNalUnitType);
    m_h264NalList.SetItemText(nIndex,5,strNalInfo);
    
    m_nNalIndex++;

    return TRUE;
}

int CH264BSAnalyzerDlg::ShowNLInfo_1(NALU_t* nalu)
{
    CString strTempIndex;
    CString strOffset;
    CString strNalLen;
    CString strStartCode;
    CString strNalUnitType;
    CString strNalInfo;
    int nIndex=0;

    // NAL��Ԫ����
    switch (nalu->nal_unit_type)
    {
    case 0:
        strNalUnitType.Format(_T("Unspecified"));
        break;
    case 1:
        strNalUnitType.Format(_T("Coded slice of a non-IDR picture"));
        switch (nalu->slice_type)
        {
        case 0:
        case 5:
            strNalInfo.Format(_T("P Slice"));
            break;
        case 1:
        case 6:
            strNalInfo.Format(_T("B Slice"));
            break;
        case 2:
        case 7:
            strNalInfo.Format(_T("I Slice"));
            break;
        }
        break;
    case 2:
        strNalUnitType.Format(_T("DPA"));
        break;
    case 3:
        strNalUnitType.Format(_T("DPB"));
        break;
    case 4:
        strNalUnitType.Format(_T("DPC"));
        break;
    case 5:
        strNalUnitType.Format(_T("Coded slice of an IDR picture"));
        strNalInfo.Format(_T("IDR"));
        break;
    case 6:
        strNalUnitType.Format(_T("SEI"));
        strNalInfo.Format(_T("SEI"));
        break;
    case 7:
        strNalUnitType.Format(_T("Sequence parameter set"));
        strNalInfo.Format(_T("SPS"));
        break;
    case 8:
        strNalUnitType.Format(_T("Picture parameter set"));
        strNalInfo.Format(_T("PPS"));
        break;
    case 9:
        strNalUnitType.Format(_T("Access UD"));
        strNalInfo.Format(_T("AUD"));
        break;
    case 10:
        strNalUnitType.Format(_T("END_SEQUENCE"));
        break;
    case 11:
        strNalUnitType.Format(_T("END_STREAM"));
        break;
    case 12:
        strNalUnitType.Format(_T("FILLER_DATA"));
        break;
    case 13:
        strNalUnitType.Format(_T("SPS_EXT"));
        break;
    case 19:
        strNalUnitType.Format(_T("AUXILIARY_SLICE"));
        break;
    default:
        strNalUnitType.Format(_T("Other"));
        break;
    }


    // ���
    strTempIndex.Format(_T("%d"),nalu->num);
    // ����ƫ��
    strOffset.Format(_T("%08x"), nalu->data_offset);
    // ����
    strNalLen.Format(_T("%d"),nalu->total_len);
    // ��ʼ��
    strStartCode.Format(_T("%s"), nalu->startcode_buf);

    //��ȡ��ǰ��¼����
    nIndex=m_h264NalList.GetItemCount();
    //���С����ݽṹ
    LV_ITEM lvitem;
    lvitem.mask=LVIF_TEXT;
    lvitem.iItem=nIndex;
    lvitem.iSubItem=0;
    //ע��vframe_index������ֱ�Ӹ�ֵ��
    //���ʹ��f_indexִ��Format!�ٸ�ֵ��
    lvitem.pszText=(char *)(LPCTSTR)strTempIndex;

#if 0
    //------------------------
    //���vector��¼��nal��λ����Ϣ
    //ʹ�������ǿ��Ի�ȡ��NAL����ϸ��Ϣ
    //����Ҫ�洢������ʼ��ĳ���
    //��ʼ��ԭ������NAL��һ����
    NALInfo nalinfo;
    nalinfo.data_lenth = nalu->total_len;
    nalinfo.data_offset = nalu->data_offset;
    m_vNalInfoVector.push_back(nalinfo);
#endif

    //------------------------��ʾ��List��
    m_h264NalList.InsertItem(&lvitem);
    m_h264NalList.SetItemText(nIndex,1,strOffset);
    m_h264NalList.SetItemText(nIndex,2,strNalLen);
    m_h264NalList.SetItemText(nIndex,3,strStartCode);
    m_h264NalList.SetItemText(nIndex,4,strNalUnitType);
    m_h264NalList.SetItemText(nIndex,5,strNalInfo);
    
    m_nNalIndex++;

    return TRUE;
}

// CH264BSAnalyzerDlg message handlers

BOOL CH264BSAnalyzerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
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

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here
    //����ѡ���б���ߣ���ͷ����������
    DWORD dwExStyle=LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_ONECLICKACTIVATE;
    //�����񣻵���ѡ�񣻸�����ʾѡ����
    m_h264NalList.ModifyStyle(0,LVS_SINGLESEL|LVS_REPORT|LVS_SHOWSELALWAYS);
    m_h264NalList.SetExtendedStyle(dwExStyle);

    // �����
    m_h264NalList.InsertColumn(0,_T("No."),LVCFMT_LEFT,50,0);
    m_h264NalList.InsertColumn(1,_T("Offset"),LVCFMT_LEFT,75,0);
    m_h264NalList.InsertColumn(2,_T("Length"),LVCFMT_LEFT,60,0);
    m_h264NalList.InsertColumn(3,_T("Start Code"),LVCFMT_LEFT,90,0);
    m_h264NalList.InsertColumn(4,_T("NAL Type"),LVCFMT_LEFT,190,0);
    m_h264NalList.InsertColumn(5,_T("Info"),LVCFMT_LEFT,65,0);
    //m_h264NalList.InsertColumn(6,_T("nal_ref_idc"),LVCFMT_LEFT,100,0);

    //---------------------
    //m_h264NalListmaxnum.SetCheck(1);
    m_nNalIndex = 1;
    //------------
    // ����ʹ��
    //m_h264InputUrl.EnableFileBrowseButton(NULL,"H.264 Files (*.264,*.h264)|*.264;*.h264|All Files (*.*)|*.*||");

    m_strFileUrl.Empty();

    m_edHexInfo.SetOptions(1, 1, 1, 1);
    m_edHexInfo.SetBPR(16); // 16�ֽ�

    // todo
    CString strSimpleInfo;
    strSimpleInfo.Format("todo  \r\n"
        "File name: hell.h264 \r\n"
        "Picture Size: xxx\r\n"
        " - Cropping Left       : xx\r\n"
        " - Cropping Right      : xx\r\n"
        " - Cropping Top        : xx\r\n"
        " - Cropping Bottom	    : xx\r\n"
        "Stream Type: High Profile @ Level xxx\r\n"
        "Video Type: xxx\r\n"
        "Encoding Type: xxx\r\n"
        );
    GetDlgItem(IDC_EDIT_SIMINFO)->SetWindowTextA(strSimpleInfo);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CH264BSAnalyzerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CH264BSAnalyzerDlg::OnPaint()
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
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CH264BSAnalyzerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// ��H264�����ļ�
void CH264BSAnalyzerDlg::OnBnClickedH264InputurlOpen()
{
    // TODO: Add your control notification handler code here
    SystemClear();
    UpdateData();

        ////////////////
    // ������
#if 0
    //char a[16]={0};
    char pszBuffer[1000] = "hello world 1234567 you go to hell hahahahahah";
    m_edHexInfo.SetOptions(1, 1, 1, 1);
    m_edHexInfo.SetBPR(16);
    m_edHexInfo.SetData((LPBYTE)pszBuffer, 100, strlen(pszBuffer));
    m_edHexInfo.SetFocus();
    return;
#endif

    CString strFilePath;
    m_edFileUrl.GetWindowText(m_strFileUrl);

    // test
    m_strFileUrl.Format("%s", "../foreman_cif.h264");
    

    if(m_strFileUrl.IsEmpty()==TRUE)
    {
        AfxMessageBox(_T("�ļ�·��Ϊ�գ�����ļ�����"));
        return;
    }

    strcpy(str_szFileUrl,m_strFileUrl.GetBuffer());

    //h264_nal_parse(this,str_szFileUrl);
    h264_nal_parse_1(str_szFileUrl, m_vNalTypeVector);

    for (int i = 0; i < (int)m_vNalTypeVector.size(); i++)
    {
        // ����SPS
        if (m_vNalTypeVector[i].nal_unit_type == 7)
        {
            parse_sps(str_szFileUrl, m_vNalTypeVector[i].data_offset, m_vNalTypeVector[i].total_len);
        }
        // ����PPS
        if (m_vNalTypeVector[i].nal_unit_type == 8)
        {
        
        }
        ShowNLInfo_1(&m_vNalTypeVector[i]);
        
    }
}

// ��������Ҫ����Accept FilesΪTRUE
void CH264BSAnalyzerDlg::OnDropFiles(HDROP hDropInfo)
{
    // TODO: Add your message handler code here and/or call default
    CDialogEx::OnDropFiles(hDropInfo);

    char* pFilePathName =(char *)malloc(MAX_URL_LENGTH);
    ::DragQueryFile(hDropInfo, 0, pFilePathName,MAX_URL_LENGTH);  // ��ȡ�Ϸ��ļ��������ļ�������ؼ���
    //m_h264InputUrl.SetWindowTextA(pFilePathName);
    m_edFileUrl.SetWindowTextA(pFilePathName);
    m_strFileUrl.Format("%s", pFilePathName);
    ::DragFinish(hDropInfo);   // ע����������٣��������ͷ�Windows Ϊ�����ļ��ϷŶ�������ڴ�
    free(pFilePathName);
}

// ˫��ĳһ�����NAL��ϸ����
void CH264BSAnalyzerDlg::OnLvnItemActivateH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: Add your control notification handler code here
    //----------------------
    POSITION ps;
    int nIndex;
    int ret = 0;

    ps=m_h264NalList.GetFirstSelectedItemPosition();
    nIndex=m_h264NalList.GetNextSelectedItem(ps);
    //----------------------
    int data_offset,data_lenth;
#if 0
    data_offset=m_vNalInfoVector[nIndex].data_offset;
    data_lenth=m_vNalInfoVector[nIndex].data_lenth;
#endif
    data_offset=m_vNalTypeVector[nIndex].data_offset;
    data_lenth=m_vNalTypeVector[nIndex].total_len;
    // 
    ret = probe_nal_unit(str_szFileUrl,data_offset,data_lenth,this);
    if (ret < 0)
    {
        AfxMessageBox("����NALʱ�����������ļ���ȡ����");
    }

    *pResult = 0;
}

void CH264BSAnalyzerDlg::OnNMCustomdrawH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
    //This code based on Michael Dunn's excellent article on
    //list control custom draw at http://www.codeproject.com/listctrl/lvcustomdraw.asp

    COLORREF clrNewTextColor, clrNewBkColor;
    int nItem;
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
    {
        // This is the notification message for an item.  We'll request
        // notifications before each subitem's prepaint stage.

        *pResult = CDRF_NOTIFYSUBITEMDRAW;
    }
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
    {
        clrNewTextColor = RGB(0,0,0);
        clrNewBkColor = RGB(255,255,255);

        nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

        CString strTemp = m_h264NalList.GetItemText(nItem,5);   // ��5�������ͣ��ж�֮
        if(strcmp(strTemp,"SLICE")==0)
        {
            clrNewBkColor = RGB(0,255,255);       //��ɫ
        }
        else if(strcmp(strTemp,"SPS")==0)
        {
            clrNewBkColor = RGB(255,255,0);        //��ɫ
        }
        else if(strcmp(strTemp,"PPS")==0)
        {
            clrNewBkColor = RGB(255,153,0);        //����ɫ
        }
        else if(strcmp(strTemp,"SEI")==0)
        {
            clrNewBkColor = RGB(255,66,255);       //�ۺ�ɫ
        }
        else if(strcmp(strTemp,"IDR")==0)
        {
            clrNewBkColor = RGB(255,0,0);          //��ɫ
        }
        else if(strcmp(strTemp,"P Slice")==0)
        {
            // ֻ�е�5�в���ʾ�������õ���ɫ
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(0,0,255); // Blue
        }
        else if(strcmp(strTemp,"B Slice")==0)
        {
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(0,255,0); // Green
        }
        else if(strcmp(strTemp,"I Slice")==0)
        {
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(255,0,0); // Red
        }

        pLVCD->clrText = clrNewTextColor;
        pLVCD->clrTextBk = clrNewBkColor;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
    }
}


void CH264BSAnalyzerDlg::OnFileOpen()
{
    // TODO: Add your command handler code here
    //AfxMessageBox("����NALʱ����");

    //CMFCEditBrowseCtrl eb_h264InputUrl;
    //eb_h264InputUrl.EnableFileBrowseButton(NULL,
    //        "H.264 Files (*.264,*.h264)|*.264;*.h264|All Files (*.*)|*.*||");

    CString strFilePath;
    char szFilter[] = "H.264 Files(*.h264;*.264)|*.h264;*.264|All Files(*.*)|*.*||";
    CFileDialog fileDlg(TRUE, "H.264", NULL, OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, szFilter);
    fileDlg.GetOFN().lpstrTitle = _T("ѡ��H.264�����ļ�");   // ����
    if (fileDlg.DoModal() != IDOK)
    {
        return;
    }

    m_strFileUrl = fileDlg.GetPathName();

    m_edFileUrl.SetWindowTextA(m_strFileUrl);
}


void CH264BSAnalyzerDlg::OnHelpAbout()
{
    // TODO: Add your command handler code here
    CAboutDlg dlg;
    dlg.DoModal();
}


void CH264BSAnalyzerDlg::OnHowtoUsage()
{
    // TODO: Add your command handler code here
    char* help = "�÷���\r\n"
        "1�����ļ�\r\n"
        "1)ʹ��file�˵��򿪣�2)���ļ��ϵ������棻3)���ļ��༭�������ļ�����·��\r\n"
        "2�����start����ʼ����NAL\r\n"
        "3��˫��ĳһ��NAL�����ɵõ���ϸ��Ϣ\r\n"
        "���ƣ���������ܷ���H264�����ļ��������ļ��޷�����\r\n"
        "�������ܷ��������ļ������\r\n";
    AfxMessageBox(help);
}

// about�Ի���Ķ���
BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    GetDlgItem(IDC_STATIC_URL)->GetWindowRect(&m_pRectLink);
    ScreenToClient(&m_pRectLink);
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default
    if (point.x > m_pRectLink.left && point.x < m_pRectLink.right && point.y < m_pRectLink.bottom && point.y > m_pRectLink.top)
    {
        //�������
        HCURSOR hCursor;
        hCursor = ::LoadCursor (NULL, IDC_HAND);
        ::SetCursor(hCursor);
    }
    CDialogEx::OnMouseMove(nFlags, point);
}


void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default
    CString strLink;

    if (point.x > m_pRectLink.left && point.x < m_pRectLink.right && point.y < m_pRectLink.bottom && point.y > m_pRectLink.top)
    {
        if (nFlags == MK_LBUTTON)
        {
            GetDlgItem(IDC_STATIC_URL)->GetWindowText(strLink);
            ShellExecute(NULL, NULL, strLink, NULL, NULL, SW_NORMAL);
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}
