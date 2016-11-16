// LAUMp4TestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LAUMp4Test.h"
#include "LAUMp4TestDlg.h"
#include "DlgUpdate.h"
#include "DlgSet.h"
#include "DlgUserIP.h"
#include "DlgVSTransUser.h"
#include "../lib/NetHostLib.h"
#include "DlgAudioTalk.h"
#include "DlgUartTran.h"

#include "obj-tracking.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_STHHAPPENED	WM_USER + 1
#define WM_MYCOMMAND	WM_USER + 2

HANDLE m_hMsgListWriteMutex = NULL;

using namespace std;
using namespace cv;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

static LONG s_ChanSock[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; 
static CHANNEL_CLIENTINFO * s_pVSChann[16] ={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static DWORD s_port[16] = {3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000};
static CLAUMp4TestDlg *m_MainDlgWnd = NULL;
HANDLE m_hostalarmmutex = NULL;
LONG   s_jpegHandle = -1;

static void WINAPI s_messagecallback(LONG hHandle,int wParam,int lParam,void *context)
{
	int i = 0;
	MSGCALLBKPARAM* pMSGCALLBKPARAM = new MSGCALLBKPARAM();
	pMSGCALLBKPARAM->hHandle = hHandle;
	pMSGCALLBKPARAM->wParam	= wParam;
	pMSGCALLBKPARAM->lParam  = lParam;
	pMSGCALLBKPARAM->context = context;
	//push server test
	if (context == (void*)0X88FF)
	{
		VSNET_ClientSetWnd(hHandle, m_MainDlgWnd->g_VideoWndInfo[15].m_pShowWnd->m_hWnd);
		VSNET_ClientStartView(hHandle);
	}
	PostMessage(m_MainDlgWnd->m_hWnd,WM_STHHAPPENED,0,(LONG)(pMSGCALLBKPARAM));
}

static int WINAPI s_UserServercallback(char *m_sername,int channel,BOOL bStart,char *m_userurl,char *username,char *password);

static void WINAPI jpegdatacallback(LONG hHandle,int m_ch,char *pBuffer,int size,void *userdata)
{
	if(pBuffer && size > 0)
	{
		FILE *fpFile =fopen("C:\\capture.jpg","wb");
		if(fpFile)
		{
			fwrite(pBuffer,size,1,fpFile);
			fclose(fpFile);
		}
	}
}

void WINAPI licdatacallback(char *pbuff,int size,void *pdlg)
{
	FILE *fpFile =fopen("C:\\getlicense.lic","wb");
	if(fpFile)
	{
		fwrite(pbuff,size,1,fpFile);
		fclose(fpFile);
	}
}

static void WINAPI pAlarmCallback(SERVERALARMINFO *pAlarmInfo,void *pcontext)
{
	int nItem;
	char lpstr[100];
	
	if(m_hostalarmmutex == NULL)
		m_hostalarmmutex = CreateMutex(0,FALSE,0);
	else
		WaitForSingleObject(m_hostalarmmutex,-1);
	
	//����������
	nItem = m_MainDlgWnd->m_listalarm.InsertItem(0,pAlarmInfo->pSerName);
	//������IP��ַ
	m_MainDlgWnd->m_listalarm.SetItemText(nItem,1,pAlarmInfo->IpAddr);
	//�������˿ں�
	sprintf(lpstr,"%d",pAlarmInfo->wPort);
	m_MainDlgWnd->m_listalarm.SetItemText(nItem,2,lpstr);
	//ͨ����
	sprintf(lpstr,"%d",pAlarmInfo->nAlarmChannel);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,3,lpstr);

	//��������
	CString str;
	switch(pAlarmInfo->nAlarmType)
	{
	case VSNETALARMMSG_SERSTART:
		str.LoadString(IDS_STRING102);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_SERSTOP:
		str.LoadString(IDS_STRING103);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_MOTION:
		str.LoadString(IDS_STRING104);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_SENSOR:
		str.LoadString(IDS_STRING105);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_HIDEALARM:
		str.LoadString(IDS_STRING106);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_VIDEOLOST:
		str.LoadString(IDS_STRING107);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	case VSNETALARMMSG_DISKFULL:
		str.LoadString(IDS_STRING108);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	default:
		str.LoadString(IDS_STRING109);
		m_MainDlgWnd->m_listalarm.SetItemText(nItem,4,str);
		break;
	}
	if(m_MainDlgWnd->m_listalarm.GetItemCount() >= 12)
		m_MainDlgWnd->m_listalarm.DeleteItem(11);
	if(m_hostalarmmutex != NULL)
		ReleaseMutex(m_hostalarmmutex);
}

static int WINAPI serchancheck(const char *sername, const char *url, WORD port,
							   const char *serialno, unsigned char pyhmac[6],
							   int channels, int AlarmInNum, int AlarmOutNum, int ch, BOOL bLogin, LONG hChHandle)
{
	if (bLogin)
	{
		if (s_ChanSock[15] != -1) return 0;
		TRACE("login\n");
		VSNET_RVSRegMsgCallback(hChHandle, s_messagecallback, (void*)0X88FF);
		VSNET_RVSSetChanServerID(hChHandle, "pushtest");
		VSNET_ClientMediaData(hChHandle, TRUE);
		s_ChanSock[15] = hChHandle;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bLocalRecing	= FALSE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iPTZActStatus	= 0;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bViewing		= TRUE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bRedirecting	= FALSE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sServerName		= "pushtest";
		m_MainDlgWnd->g_VideoChlInfo[15].m_sServerUrl		= url;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iPort			= port;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sUserName		= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_sPsw			= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_iChlSn			= ch;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iTransType		= 3;
		m_MainDlgWnd->g_VideoChlInfo[15].m_hLnk			= hChHandle;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sMultiCastURL	= "234.5.6.8";
		m_MainDlgWnd->g_VideoChlInfo[15].m_iMultiCastPort	= 6000;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iMultiCastLocalPort = 3000;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iWndSn			= 15;
	}
	else
	{
		TRACE("logout\n");
		s_ChanSock[15] = -1;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bLocalRecing	= FALSE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iPTZActStatus	= 0;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bViewing		= FALSE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_bRedirecting	= FALSE;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sServerName		= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_sServerUrl		= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_iPort			= -1;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sUserName		= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_sPsw			= "";
		m_MainDlgWnd->g_VideoChlInfo[15].m_iChlSn			= -1;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iTransType		= -1;
		m_MainDlgWnd->g_VideoChlInfo[15].m_hLnk			= -1;
		m_MainDlgWnd->g_VideoChlInfo[15].m_sMultiCastURL	= "234.5.6.8";
		m_MainDlgWnd->g_VideoChlInfo[15].m_iMultiCastPort	= 6000;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iMultiCastLocalPort = 3000;
		m_MainDlgWnd->g_VideoChlInfo[15].m_iWndSn			= -1;
	}
	
	return 0;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLAUMp4TestDlg dialog

CLAUMp4TestDlg::CLAUMp4TestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLAUMp4TestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLAUMp4TestDlg)
	m_sername = _T("video server");
	m_url = _T("192.168.1.122");
	m_username = _T("888888");
	m_password = _T("888888");
	m_channel = 0;
	m_trantype = 2;
	m_port = 3000;
	m_tranmultiurl = _T("234.5.6.7");
	m_tranmultiport = 6000;
	m_tranlocalport = 3000;
	m_CB_iMode = 3;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_MainDlgWnd = this;

	m_hTalkHandle = -1;
}

void CLAUMp4TestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLAUMp4TestDlg)
	DDX_Control(pDX, IDC_STATIC_VIDEO14, m_video14);
	DDX_Control(pDX, IDC_STATIC_VIDEO16, m_video16);
	DDX_Control(pDX, IDC_STATIC_VIDEO15, m_video15);
	DDX_Control(pDX, IDC_STATIC_VIDEO13, m_video13);
	DDX_Control(pDX, IDC_STATIC_VIDEO12, m_video12);
	DDX_Control(pDX, IDC_STATIC_VIDEO11, m_video11);
	DDX_Control(pDX, IDC_STATIC_VIDEO10, m_video10);
	DDX_Control(pDX, IDC_STATIC_VIDEO9, m_video9);
	DDX_Control(pDX, IDC_STATIC_VIDEO8, m_video8);
	DDX_Control(pDX, IDC_STATIC_VIDEO7, m_video7);
	DDX_Control(pDX, IDC_STATIC_VIDEO6, m_video6);
	DDX_Control(pDX, IDC_STATIC_VIDEO5, m_video5);
	DDX_Control(pDX, IDC_LIST_ALARM, m_listalarm);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_VolumeCtrl);
	DDX_Control(pDX, IDC_STATIC_VIDEO4, m_video4);
	DDX_Control(pDX, IDC_STATIC_VIDEO3, m_video3);
	DDX_Control(pDX, IDC_STATIC_VIDEO2, m_video2);
	DDX_Control(pDX, IDC_STATIC_VIDEO1, m_video1);
	DDX_Text(pDX, IDC_EDIT_SERNAME, m_sername);
	DDV_MaxChars(pDX, m_sername, 24);
	DDX_Text(pDX, IDC_EDIT_URL, m_url);
	DDV_MaxChars(pDX, m_url, 40);
	DDX_Text(pDX, IDC_EDIT_USER, m_username);
	DDV_MaxChars(pDX, m_username, 20);
	DDX_Text(pDX, IDC_EDIT_PASS, m_password);
	DDV_MaxChars(pDX, m_password, 20);
	DDX_CBIndex(pDX, IDC_COMBO_CHANNEL, m_channel);
	DDX_CBIndex(pDX, IDC_COMBO_TRANTYPE, m_trantype);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Text(pDX, IDC_EDIT_TRAN_MULTIURL, m_tranmultiurl);
	DDV_MaxChars(pDX, m_tranmultiurl, 15);
	DDX_Text(pDX, IDC_EDIT_TRAN_MULTIPORT, m_tranmultiport);
	DDX_Text(pDX, IDC_EDIT_TRAN_LOCALPORT, m_tranlocalport);
	DDX_CBIndex(pDX, IDC_COMBO_Mode, m_CB_iMode);
	DDX_Control(pDX, IDC_LUSERPARAMXCTRL, m_paramctrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLAUMp4TestDlg, CDialog)
	//{{AFX_MSG_MAP(CLAUMp4TestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_STATIC_VIDEO1, OnStaticVideo1)
	ON_BN_CLICKED(IDC_STATIC_VIDEO2, OnStaticVideo2)
	ON_BN_CLICKED(IDC_STATIC_VIDEO3, OnStaticVideo3)
	ON_BN_CLICKED(IDC_STATIC_VIDEO4, OnStaticVideo4)
	ON_CONTROL(STN_DBLCLK,IDC_STATIC_VIDEO1,OnDbStatic1)
	ON_CONTROL(STN_DBLCLK,IDC_STATIC_VIDEO2,OnDbStatic2)
	ON_CONTROL(STN_DBLCLK,IDC_STATIC_VIDEO3,OnDbStatic3)
	ON_CONTROL(STN_DBLCLK,IDC_STATIC_VIDEO4,OnDbStatic4)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_UPDATE, OnBtnUpdate)
	ON_BN_CLICKED(IDC_BTN_CONFIG, OnBtnConfig)
	ON_BN_CLICKED(IDC_BTN_RESET, OnBtnReset)
	ON_BN_CLICKED(IDC_BTN_USERIP, OnBtnUserip)
	ON_BN_CLICKED(IDC_BTN_TRANSMIT, OnBtnTransmit)
	ON_BN_CLICKED(IDC_BTN_TALK, OnBtnTalk)
	ON_BN_CLICKED(IDC_BTN_LOCALREC, OnBtnLocalRec)
	ON_BN_CLICKED(IDC_BTN_UARTTRAN, OnBtnUarttran)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_VSTRANSUSER, OnBtnVstransuser)
	ON_BN_CLICKED(IDC_BTN_SYSPARAM, OnBtnSysparam)
	ON_BN_CLICKED(IDC_CHECK_VIEW, OnCheckView)
	ON_BN_CLICKED(IDC_BTN_CLEAR, OnBtnClear)
	ON_BN_CLICKED(IDC_BTN_BRADCAST, OnBtnBradcast)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BTN_CAPTURE, OnBtnCapture)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_STATIC_VIDEO5, OnStaticVideo5)
	ON_BN_CLICKED(IDC_STATIC_VIDEO6, OnStaticVideo6)
	ON_BN_CLICKED(IDC_STATIC_VIDEO7, OnStaticVideo7)
	ON_BN_CLICKED(IDC_STATIC_VIDEO8, OnStaticVideo8)
	ON_BN_CLICKED(IDC_STATIC_VIDEO9, OnStaticVideo9)
	ON_BN_CLICKED(IDC_STATIC_VIDEO10, OnStaticVideo10)
	ON_BN_CLICKED(IDC_STATIC_VIDEO11, OnStaticVideo11)
	ON_BN_CLICKED(IDC_STATIC_VIDEO12, OnStaticVideo12)
	ON_BN_CLICKED(IDC_STATIC_VIDEO13, OnStaticVideo13)
	ON_BN_CLICKED(IDC_STATIC_VIDEO14, OnStaticVideo14)
	ON_BN_CLICKED(IDC_STATIC_VIDEO15, OnStaticVideo15)
	ON_BN_CLICKED(IDC_STATIC_VIDEO16, OnStaticVideo16)
	ON_CBN_SELCHANGE(IDC_COMBO_Mode, OnSelchangeCOMBOMode)
	ON_BN_CLICKED(IDC_BTN_PTZLEFT, OnBtnPtzleft)
	ON_BN_CLICKED(IDC_BTN_PTZRight, OnBTNPTZRight)
	ON_BN_CLICKED(IDC_BTN_PTZUP, OnBtnPtzup)
	ON_BN_CLICKED(IDC_BTN_PTZDOWN, OnBtnPtzdown)
	ON_BN_CLICKED(IDC_BTN_PTZAUTO, OnBtnPtzauto)
	ON_BN_CLICKED(IDC_BTN_BMPCAPTURE, OnBtnBmpcapture)
	ON_BN_CLICKED(IDC_BTN_SERVERJPGCAPTURE, OnBtnServerjpgcapture)
	ON_MESSAGE(WM_STHHAPPENED,OnMsgSthHappened)
	ON_BN_CLICKED(IDC_BTN_PRESET, OnBtnPreset)
	ON_BN_CLICKED(IDC_BTN_PLAYSTOP, PlayStopSwitch)
	ON_BN_CLICKED(IDC_BTN_CALL, OnBtnCall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLAUMp4TestDlg message handlers

BOOL CLAUMp4TestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_LUSERPARAMXCTRL)->ShowWindow(SW_HIDE);

	g_VideoWndInfo[0].m_pShowWnd = &m_video1;
	g_VideoWndInfo[1].m_pShowWnd = &m_video2;
	g_VideoWndInfo[2].m_pShowWnd = &m_video3;
	g_VideoWndInfo[3].m_pShowWnd = &m_video4;
	g_VideoWndInfo[4].m_pShowWnd = &m_video5;
	g_VideoWndInfo[5].m_pShowWnd = &m_video6;
	g_VideoWndInfo[6].m_pShowWnd = &m_video7;
	g_VideoWndInfo[7].m_pShowWnd = &m_video8;
	g_VideoWndInfo[8].m_pShowWnd = &m_video9;
	g_VideoWndInfo[9].m_pShowWnd = &m_video10;
	g_VideoWndInfo[10].m_pShowWnd = &m_video11;
	g_VideoWndInfo[11].m_pShowWnd = &m_video12;
	g_VideoWndInfo[12].m_pShowWnd = &m_video13;
	g_VideoWndInfo[13].m_pShowWnd = &m_video14;
	g_VideoWndInfo[14].m_pShowWnd = &m_video15;
	g_VideoWndInfo[15].m_pShowWnd = &m_video16;

	for(int i = 0; i < MAX_VIDEOWND; i ++)
	{
		g_VideoWndInfo[i].m_iChlSn  = i;
	}

	for(int i = 0; i < MAX_VIDEOWND; i ++)
	{
		g_VideoChlInfo[i].m_bLocalRecing	= FALSE;
		g_VideoChlInfo[i].m_iPTZActStatus	= 0;
		g_VideoChlInfo[i].m_bViewing		= FALSE;
		g_VideoChlInfo[i].m_bRedirecting	= FALSE;
		g_VideoChlInfo[i].m_sServerName		= "";
		g_VideoChlInfo[i].m_sServerUrl		= "";
		g_VideoChlInfo[i].m_iPort			= -1;
		g_VideoChlInfo[i].m_sUserName		= "";
		g_VideoChlInfo[i].m_sPsw			= "";
		g_VideoChlInfo[i].m_iChlSn			= -1;
		g_VideoChlInfo[i].m_iTransType		= -1;
		g_VideoChlInfo[i].m_hLnk			= -1;
		g_VideoChlInfo[i].m_sMultiCastURL	= "234.5.6.8";
		g_VideoChlInfo[i].m_iMultiCastPort	= 6000;
		g_VideoChlInfo[i].m_iMultiCastLocalPort = 3000;
		g_VideoChlInfo[i].m_iWndSn			= -1;
	}

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//�������籨����������
	VSNET_ClientStartup(WM_MYCOMMAND,m_hWnd);
	VSNET_RVSRegSerCheckCallback(serchancheck);
	VSNETRVSINFO rvsinfo;
	rvsinfo.m_rvsbindurl = NULL;
	rvsinfo.m_rvsbindport = 3004;
	VSNET_RVSStartServer(&rvsinfo);

	NetHostLibInit();
	NetHostLibRegCallBack(pAlarmCallback,NULL,NULL,NULL,NULL,NULL);
	NetHostAlarm_Start(NULL,10000);
	
	
	// TODO: Add extra initialization here
	CString str;
	m_VolumeCtrl.SetRange(0,9,TRUE);
	str.LoadString(IDS_STRING110);
	m_listalarm.InsertColumn(0, str , LVCFMT_LEFT, 130);
	str.LoadString(IDS_STRING111);
	m_listalarm.InsertColumn(1, str , LVCFMT_LEFT, 130);
	str.LoadString(IDS_STRING112);
	m_listalarm.InsertColumn(2, str, LVCFMT_LEFT, 100);
	str.LoadString(IDS_STRING113);
	m_listalarm.InsertColumn(3, str, LVCFMT_LEFT, 100);
	str.LoadString(IDS_STRING114);
	m_listalarm.InsertColumn(4, str, LVCFMT_LEFT, 350);
	m_listalarm.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	g_GlobeEnvi.m_iSelWndSn = 0;
	SetBtnStatus();
	OnSelchangeCOMBOMode();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLAUMp4TestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLAUMp4TestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLAUMp4TestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CLAUMp4TestDlg::OnStaticVideo1() 
{
	OnClkVideoWnd(0,IDC_STATIC_VIDEO1);	
}

void CLAUMp4TestDlg::OnStaticVideo2() 
{
	OnClkVideoWnd(1,IDC_STATIC_VIDEO2);
}

void CLAUMp4TestDlg::OnStaticVideo3() 
{
	OnClkVideoWnd(2,IDC_STATIC_VIDEO3);
}

void CLAUMp4TestDlg::OnStaticVideo4() 
{
	OnClkVideoWnd(3,IDC_STATIC_VIDEO4);
}
void CLAUMp4TestDlg::OnDbStatic1()
{
	//g_GlobeEnvi.m_iSelWndSn = 0;
	//	m_CB_iMode = 0;
	//	UpdateData(FALSE);
	//	OnSelchangeCOMBOMode();
	
}

void CLAUMp4TestDlg::OnDbStatic2()
{
//	g_GlobeEnvi.m_iSelWndSn = 1;
	//	m_CB_iMode = 0;
	//	UpdateData(FALSE);
	//	OnSelchangeCOMBOMode();
	
}

void CLAUMp4TestDlg::OnDbStatic3()
{
//	g_GlobeEnvi.m_iSelWndSn = 2;
	//	m_CB_iMode = 0;
	//	UpdateData(FALSE);
	//	OnSelchangeCOMBOMode();
	
}

void CLAUMp4TestDlg::OnDbStatic4()
{
//	g_GlobeEnvi.m_iSelWndSn = 3;
	//	m_CB_iMode = 0;
	//	UpdateData(FALSE);
	//	OnSelchangeCOMBOMode();
	
}

void CLAUMp4TestDlg::PlayStopSwitch()
{
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
	{
		StopChlActs();
	}
	else
	{
		PlayChl();
	}
}

void CLAUMp4TestDlg::PlayChl() 
{
	UpdateData(TRUE);
	char url[40];
	char sername[24];
	char user[20];
	char password[20];

	CHANNEL_CLIENTINFO *pVSChann;
	pVSChann = new CHANNEL_CLIENTINFO();
	memset(pVSChann,0,sizeof(CHANNEL_CLIENTINFO));
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);
#if 1
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1) StopChlActs();
	pVSChann->m_buffnum    = 50;
	pVSChann->m_ch         = m_channel;//
	pVSChann->m_hVideohWnd = g_VideoWndInfo[g_GlobeEnvi.m_iSelWndSn].m_pShowWnd->GetSafeHwnd();

	pVSChann->m_hChMsgWnd  = NULL;
	pVSChann->m_nChmsgid   = 0;
	pVSChann->m_sername    = new char[40];
	memset(pVSChann->m_sername,0,40);
	memcpy(pVSChann->m_sername,sername,sizeof(sername));
	pVSChann->m_username   = new char[40];
	memset(pVSChann->m_username,0,40);
	memcpy(pVSChann->m_username,user,sizeof(user));
	pVSChann->m_password   = new char[40];
	memset(pVSChann->m_password,0,40);
	memcpy(pVSChann->m_password,password,sizeof(password));
	pVSChann->m_playstart  = TRUE ;
	pVSChann->m_tranType   = m_trantype + 1;
	pVSChann->m_useoverlay = FALSE;
	memcpy(pVSChann->url,url,sizeof(url));
	pVSChann->context      = NULL;
	pVSChann->m_messagecallback = s_messagecallback;
	s_ChanSock[g_GlobeEnvi.m_iSelWndSn] = VSNET_ClientStart(url,pVSChann,m_port);

	
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
	{
		pGet_SelChl()->m_bViewing = TRUE;
		pGet_SelChl()->m_sServerName	= m_sername;
		pGet_SelChl()->m_sServerUrl		= m_url;
		pGet_SelChl()->m_iPort			= m_port;
		pGet_SelChl()->m_sUserName		= m_username;
		pGet_SelChl()->m_sPsw			= m_password;
		pGet_SelChl()->m_iChlSn			= m_channel;
		pGet_SelChl()->m_iTransType		= m_trantype;
		pGet_SelChl()->m_iWndSn			= g_GlobeEnvi.m_iSelWndSn;
	}
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] == -1)
	{
		CString str;
		str.LoadString(IDS_STRING115);
		AfxMessageBox(str);
		if(pVSChann->m_sername) delete(pVSChann->m_sername);
		if(pVSChann->m_username) delete(pVSChann->m_username);
		if(pVSChann->m_password) delete(pVSChann->m_password);
		delete  pVSChann;
		return;
	}
	if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn])
	{
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_sername) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_sername);
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_username) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_username);
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_password) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_password);
		delete  (CHANNEL_CLIENTINFO*)s_pVSChann[g_GlobeEnvi.m_iSelWndSn];
		s_pVSChann[g_GlobeEnvi.m_iSelWndSn] = NULL;
	}
	s_pVSChann[g_GlobeEnvi.m_iSelWndSn] = pVSChann;
	s_port[g_GlobeEnvi.m_iSelWndSn] = m_port;
	SetBtnStatus();
	//VSNET_ClientSetDelayTime(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],10);
	VSNET_ClientSetRedirectDelayTime(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],10);
	int xxxx = VSNET_ClientGetFrameRate(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
#else
	int i;
	pVSChann.m_hChMsgWnd  = NULL;
	pVSChann.m_nChmsgid   = 0;
	pVSChann.m_sername    = sername;
	pVSChann.m_username   = user;
	pVSChann.m_password   = password;
	pVSChann.m_playstart  = TRUE;
	pVSChann.m_tranType   = m_trantype + 1;
	pVSChann.m_useoverlay = FALSE;
	pVSChann.nColorKey    = OVERLAY_KEY;
	for(i=0;i<4;i++)
	{
		if(s_ChanSock[i] != -1) 
		{
			VSNET_ClientStop(s_ChanSock[i]);
			s_ChanSock[i] = -1;
		}
		pVSChann.m_buffnum    = 20;
		pVSChann.m_ch         = i;
		if(i == 0)
			pVSChann.m_hVideohWnd = m_video1.GetSafeHwnd();
		else if(i == 1)
			pVSChann.m_hVideohWnd = m_video2.GetSafeHwnd();
		else if(i == 2)
			pVSChann.m_hVideohWnd = m_video3.GetSafeHwnd();
		else
			pVSChann.m_hVideohWnd = m_video4.GetSafeHwnd();
		pVSChann->context      = NULL;
		pVSChann->m_messagecallback = NULL;
		s_ChanSock[i] = VSNET_ClientStart(url,&pVSChann,m_port);
		if(s_ChanSock[i] == -1)
		{
			CString err;
			err.Format(IDS_STRING116,i+1);
			AfxMessageBox(err);
		}
		SetBtnStatus();
	}
#endif
}

void CLAUMp4TestDlg::StopChlActs() //ֹͣ��ǰͨ��һ�л
{
#if 1
	//ֹͣ¼��
	LocalRecStop();

	//ֹͣ��̨����
	VSNET_ClientPTZCtrl(s_ChanSock[g_GlobeEnvi.m_iSelWndSn], PTZ_STOP, 20);

	//ֹͣת��
	if(pGet_SelChl()->m_bRedirecting)
	{
		TransmitStop();
	}
	
	//ֹͣ����
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1) 
	{
		BOOL bSuc = VSNET_ClientStop(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
		if(bSuc)
		{
			s_ChanSock[g_GlobeEnvi.m_iSelWndSn] = -1;
			pGet_SelChl()->m_bViewing = FALSE;
		}
	}
	
	//��������Ӧ��ͨ����Ϣ
	if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]) 
	{
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_sername) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_sername);
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_username) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_username);
		if(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_password) delete(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_password);	
		delete (CHANNEL_CLIENTINFO*)s_pVSChann[g_GlobeEnvi.m_iSelWndSn];
		s_pVSChann[g_GlobeEnvi.m_iSelWndSn] = NULL;
	}

#else
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1) 
	{
		VSNET_ClientStop(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
		s_ChanSock[g_GlobeEnvi.m_iSelWndSn] = -1;
	}
#endif
	//(CWnd::FromHandle(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_hVideohWnd))->Invalidate();
	//GetDlgItem(GetWindowLong(g_VideoWndInfo[g_GlobeEnvi.m_iSelWndSn].m_pShowWnd, GWL_ID))->Invalidate();

	//�ػ�ô���
	g_VideoWndInfo[g_GlobeEnvi.m_iSelWndSn].m_pShowWnd->Invalidate();
	SetBtnStatus();
}

void CLAUMp4TestDlg::OnDestroy() 
{
	OnCancel();
	/*
	int i;
	//VSNET_ClientTalkFreeAll();
	if(m_hostalarmmutex != NULL)
	{
		CloseHandle(m_hostalarmmutex);
		m_hostalarmmutex = NULL;
	}

	for(i=0;i<4;i++)
	{
		if(s_ChanSock[i] != -1)
		{
			VSNET_ClientStop(s_ChanSock[i]);
			s_ChanSock[i] = -1;
		}
	}
	for(i=0;i<4;i++)
	{
		if(s_pVSChann[i]) 
		{
			if(s_pVSChann[i]->m_sername) delete(s_pVSChann[i]->m_sername);
			if(s_pVSChann[i]->m_username) delete(s_pVSChann[i]->m_username);
			if(s_pVSChann[i]->m_password) delete(s_pVSChann[i]->m_password);
			delete (CHANNEL_CLIENTINFO*)s_pVSChann[i];
			s_pVSChann[i] = NULL;
		}
	}
	TransmitStop();
	VSNET_ClientCleanup();
	//NetHostAlarm_Stop();
	//NetHostLibFree();
	CDialog::OnDestroy();*/	
}

void CLAUMp4TestDlg::OnBtnUpdate() 
{
	CDlgUpdate dlg;
	UpdateData(TRUE);
	sprintf(dlg.m_url,"%s",m_url);
	sprintf(dlg.m_username,"%s",m_username);
	sprintf(dlg.m_password,"%s",m_password);
	dlg.m_port = m_port;
	dlg.DoModal();
}

void CLAUMp4TestDlg::OnCancel() 
{	
	StopAllChlActs();
	CDialog::OnCancel();
}

void CLAUMp4TestDlg::StopAllChlActs()
{
	//ֹͣ�Խ�
	if(m_hTalkHandle != -1)
	{
		VSNET_ClientTalkExchangeStop(m_hTalkHandle);
		m_hTalkHandle = -1;
	}
	
	//ֹͣ����ͨ��һ�л
	for(int i = 0; i < MAX_VIDEOWND; i ++)
	{
		g_GlobeEnvi.m_iSelWndSn = i;
		//ֹͣ��ǰͨ��һ�л
		StopChlActs();
	}

	//���SDk Socket����
	VSNET_RVSStopServer();
	VSNET_ClientCleanup();

}

void CLAUMp4TestDlg::OnBtnConfig() 
{
	ShellExecute(NULL, "cmd","regsvr32", "LUSerParamX.ocx", NULL, SW_SHOW);
	UpdateData(TRUE);
	char sername[24];
	char url[40];
	short port;
	char user[20];
	char password[20];
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	port = m_port;
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);
	
	m_paramctrl.ShowParamPage(sername,url,port,
		user,password);

/*	
	int ret;
	//char url[40];
	//char sername[24];
	//char user[20];
	//char password[20];
	CDlgSet dlg;
	
	sprintf(dlg.m_sername,"%s",m_sername);
	sprintf(dlg.m_url,"%s",m_url);
	sprintf(dlg.m_username,"%s",m_username);
	sprintf(dlg.m_password,"%s",m_password);
	dlg.m_port = m_port;

	if((ret = VSNET_ClientAffirmUser(dlg.m_sername,dlg.m_url,dlg.m_username,dlg.m_password,m_port)) != 0)
	{
		TRACE("VSNET_ClientAffirmUser ret : %d\n",ret);
		if(ret == -1)
			AfxMessageBox("���ӷ�����ʧ��");
		else
			AfxMessageBox("�û������������");
		return;
	}
	dlg.DoModal();*/
}

void CLAUMp4TestDlg::OnBtnReset() 
{
	CString str;
	str.LoadString(IDS_STRING117);
	if(MessageBox(str,"",MB_ICONQUESTION|MB_OKCANCEL) != IDOK)
		return;
	UpdateData(TRUE);
	char url[40];
	char sername[24];
	char user[20];
	char password[20];
	
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);
	if(VSNET_ClientResetServer(sername,url,user,password,m_port))
	{
		str.LoadString(IDS_STRING118);
		AfxMessageBox(str);
	}
	else
	{
		str.LoadString(IDS_STRING119);
		AfxMessageBox(str);
	}
}

void CLAUMp4TestDlg::OnBtnUserip() 
{
	UpdateData(TRUE);	
	CDlgUserIP dlg;
	sprintf(dlg.m_sername,"%s",m_sername);
	sprintf(dlg.m_url,"%s",m_url);
	sprintf(dlg.m_username,"%s",m_username);
	sprintf(dlg.m_password,"%s",m_password);
	dlg.m_port = m_port;
	dlg.DoModal();
}

void CLAUMp4TestDlg::SetBtnStatus()
{
	BOOL bChlLnked = s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1;
	CString str;
	if(bChlLnked)
	{
		str.LoadString(IDS_STRING103);
		GetDlgItem(IDC_BTN_PLAYSTOP)->SetWindowText(str);
		m_sername	= pGet_SelChl()->m_sServerName;
		m_url		= pGet_SelChl()->m_sServerUrl;
		m_port		= pGet_SelChl()->m_iPort;
		m_username	= pGet_SelChl()->m_sUserName;
		m_password	= pGet_SelChl()->m_sPsw;
		m_channel	= pGet_SelChl()->m_iChlSn;
		m_trantype	= pGet_SelChl()->m_iTransType;
		UpdateData(FALSE);
	}
	else
	{
		str.LoadString(IDS_STRING120);
		GetDlgItem(IDC_BTN_PLAYSTOP)->SetWindowText(str);
	}
	GetDlgItem(IDC_BTN_LOCALREC)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_PTZLEFT)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_PTZRight)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_PTZUP)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_PTZDOWN)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_PTZAUTO)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_CHECK_VIEW)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_CAPTURE)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_BMPCAPTURE)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_BTN_SERVERJPGCAPTURE)->EnableWindow(bChlLnked);
	GetDlgItem(IDC_EDIT_SERNAME)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_EDIT_URL)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_EDIT_PORT)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_EDIT_USER)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_EDIT_PASS)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_COMBO_TRANTYPE)->EnableWindow(!bChlLnked);
	GetDlgItem(IDC_COMBO_CHANNEL)->EnableWindow(!bChlLnked);

	BOOL bRedirecting = pGet_SelChl()->m_bRedirecting;
	if(bRedirecting)
	{
		str.LoadString(IDS_STRING121);
		GetDlgItem(IDC_BTN_TRANSMIT)->SetWindowText(str);
		
		m_tranmultiurl = pGet_SelChl()->m_sMultiCastURL;
		m_tranmultiport = pGet_SelChl()->m_iMultiCastPort;
		m_tranlocalport = pGet_SelChl()->m_iMultiCastLocalPort;
		UpdateData(FALSE);
	}
	else
	{
		str.LoadString(IDS_STRING122);
		GetDlgItem(IDC_BTN_TRANSMIT)->SetWindowText(str);
	}
	GetDlgItem(IDC_EDIT_TRAN_MULTIURL)->EnableWindow(!bRedirecting);
	GetDlgItem(IDC_EDIT_TRAN_MULTIPORT)->EnableWindow(!bRedirecting);
	GetDlgItem(IDC_EDIT_TRAN_LOCALPORT)->EnableWindow(!bRedirecting);


	BOOL bChlViewing = pGet_SelChl()->m_bViewing;
	if(bChlViewing)
	{
		str.LoadString(IDS_STRING123);
		GetDlgItem(IDC_CHECK_VIEW)->SetWindowText(str);
	}
	else
	{
		str.LoadString(IDS_STRING124);
		GetDlgItem(IDC_CHECK_VIEW)->SetWindowText(str);		
	}

	BOOL bLocalRecing = pGet_SelChl()->m_bLocalRecing;
	if(bLocalRecing)
	{
		str.LoadString(IDS_STRING125);
		GetDlgItem(IDC_BTN_LOCALREC)->SetWindowText(str);
	}
	else
	{
		str.LoadString(IDS_STRING126);
		GetDlgItem(IDC_BTN_LOCALREC)->SetWindowText(str);
	}

	if(m_hTalkHandle != -1)
	{
		str.LoadString(IDS_STRING127);
		GetDlgItem(IDC_BTN_TALK)->SetWindowText(str);
	}
	else
	{
		str.LoadString(IDS_STRING128);
		GetDlgItem(IDC_BTN_TALK)->SetWindowText(str);
	}
	

	BOOL bPTZAct = pGet_SelChl()->m_iPTZActStatus & PTZ_STA_LEFT;
	if(bPTZAct)
	{
		GetDlgItem(IDC_BTN_PTZLEFT)->SetWindowText("|<");
	}
	else
	{
		GetDlgItem(IDC_BTN_PTZLEFT)->SetWindowText("<");
	}
	bPTZAct = pGet_SelChl()->m_iPTZActStatus & PTZ_STA_RIGHT;
	if(bPTZAct)
	{
		GetDlgItem(IDC_BTN_PTZRight)->SetWindowText(">|");
	}
	else
	{
		GetDlgItem(IDC_BTN_PTZRight)->SetWindowText(">");
	}
	bPTZAct = pGet_SelChl()->m_iPTZActStatus & PTZ_STA_UP;
	if(bPTZAct)
	{
		GetDlgItem(IDC_BTN_PTZUP)->SetWindowText("X");
	}
	else
	{
		GetDlgItem(IDC_BTN_PTZUP)->SetWindowText("A");
	}
	bPTZAct = pGet_SelChl()->m_iPTZActStatus & PTZ_STA_DOWN;
	if(bPTZAct)
	{
		GetDlgItem(IDC_BTN_PTZDOWN)->SetWindowText("X");
	}
	else
	{
		GetDlgItem(IDC_BTN_PTZDOWN)->SetWindowText("V");
	}
	bPTZAct = pGet_SelChl()->m_iPTZActStatus & PTZ_STA_AUTO;
	if(bPTZAct)
	{
		GetDlgItem(IDC_BTN_PTZAUTO)->SetWindowText("G");
	}
	else
	{
		GetDlgItem(IDC_BTN_PTZAUTO)->SetWindowText("O");
	}
}

void CLAUMp4TestDlg::OnBtnTransmit()
{
	if(pGet_SelChl()->m_bRedirecting)
	{
		TransmitStop();
	}
	else
	{
		TransmitStart();
	}
	SetBtnStatus();
}

void CLAUMp4TestDlg::TransmitStart() 
{
	UpdateData(TRUE);
	VSNET_REDIRECTORINFO pInfo;
	sprintf(pInfo.m_multiip,"%s",m_tranmultiurl);
	pInfo.m_wMultiPort = m_tranmultiport;
	pInfo.m_wLocaport  = m_tranlocalport;
	pInfo.m_videobuff  = 50;
	pInfo.m_audiobuff  = 50;
	pInfo.m_channuser  = 300;
	pInfo.m_totaluser  = 300;
	pInfo.m_UserCheckcallback = NULL;
	pInfo.m_UserConnectcallback = NULL;
	BOOL bSuc = VSNET_SetRedirectorInfo(&pInfo);
	int  iRet = VSNET_StartListenClient(NULL);
	if(bSuc && iRet == 0)
	{
		pGet_SelChl()->m_bRedirecting = TRUE;
		pGet_SelChl()->m_sMultiCastURL= m_tranmultiurl;
		pGet_SelChl()->m_iMultiCastPort= m_tranmultiport;
		pGet_SelChl()->m_iMultiCastLocalPort = m_tranlocalport;
	}
	else
	{
		CString strerr;
		strerr.Format(IDS_STRING129,iRet);
		AfxMessageBox(strerr);
	}
}

void CLAUMp4TestDlg::TransmitStop()
{
	if(VSNET_StopListenClient())
	{
		pGet_SelChl()->m_bRedirecting = FALSE;
	}
}

void CLAUMp4TestDlg::OnBtnTalk() 
{
	UpdateData(TRUE);
	char url[40];
	char user[20];
	char password[20];
	char sername[40];
	
	CString str;
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);
	sprintf(sername, "%s", m_sername);

	if(m_hTalkHandle != -1)
	{
		VSNET_ClientTalkExchangeStop(m_hTalkHandle);
		m_hTalkHandle = -1;
	}
	else
	{
		m_hTalkHandle = VSNET_ClientTalkExchangeStart(url,user,password,m_port, NULL, sername);
		if(m_hTalkHandle == -1)
		{
			str.LoadString(IDS_STRING130);
			AfxMessageBox(str);
		}
	}

	//ˢ�°�ť״̬
	SetBtnStatus();
}

void CLAUMp4TestDlg::OnBtnLocalRec()
{
	BOOL bLocalRecing = pGet_SelChl()->m_bLocalRecing;
	if(bLocalRecing)
	{
		LocalRecStop();
	}
	else
	{
		LocalRecStart();
	}
	SetBtnStatus();
}

void CLAUMp4TestDlg::LocalRecStart() 
{
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
	{
		char sRecFileName[MAX_PATH];
		sprintf(sRecFileName, "%s%d%s", fname_mp4, g_GlobeEnvi.m_iSelWndSn, ".mp4");
		BOOL bSuc = VSNET_ClientStartMp4Capture(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],sRecFileName);
		pGet_SelChl()->m_bLocalRecing = bSuc;
	}
}

void CLAUMp4TestDlg::LocalRecStop() 
{
	if(pGet_SelChl()->m_bLocalRecing)
	{
		VSNET_ClientStopMp4Capture(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
		pGet_SelChl()->m_bLocalRecing = FALSE;
	}
}


void CLAUMp4TestDlg::OnBtnUarttran() 
{
	CDlgUartTran dlg;
	dlg.DoModal();
	/*UpdateData(FALSE);
	char url[40];
	char sername[24];
	char user[20];
	char password[20];
	BYTE pBuff[10];
	//int i;

	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);
#if 0
	pBuff[0] = 0XFF;
	pBuff[1] = 0X01;
	pBuff[2] = 0X00;
	pBuff[3] = 0X04;
	pBuff[4] = 0X35;
	pBuff[5] = 0X28;
	pBuff[6] = pBuff[1] + pBuff[2] + pBuff[3] + pBuff[4] + pBuff[5];
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
		VSNET_ClientSerialSend(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],0,pBuff,7);
	else
		VSNET_ClientSerialSendToServer(sername,url,g_GlobeEnvi.m_iSelWndSn,user,password,0,pBuff,7,m_port);

#else
	pBuff[0] = 0XFA;
	pBuff[1] = 0X01;
	pBuff[2] = 0X22;
	pBuff[3] = 0X33;
	pBuff[4] = 0X10;

	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
		VSNET_ClientSerialSend(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],0,pBuff,7);
	else
		VSNET_ClientSerialSendToServer(sername,url,g_GlobeEnvi.m_iSelWndSn,user,password,0,pBuff,7,m_port);

#endif*/
}

void CLAUMp4TestDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == (CScrollBar*)GetDlgItem(IDC_SLIDER_VOLUME))
	{
		//65535
		if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
		{
			static WORD setvolumelist[] =
			{0,50000,55000,60000,61000,62000,63000,64000,65000,0XFFFF};

			WORD setvolume = setvolumelist[m_VolumeCtrl.GetPos()];
			VSNET_ClientAudioVolume(s_ChanSock[g_GlobeEnvi.m_iSelWndSn],setvolume);
		}
		//TRACE("pos : %d\n",m_VolumeCtrl.GetPos());
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

static int s_senddata[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int WINAPI s_UserServercallback(char *m_sername,int channel,BOOL bStart,char *m_userurl,char *username,char *password)
{
	TRACE("ת��: servername:%s,ch:%d,start:%d\n",m_sername,channel,bStart);
	if(bStart)
	{
		if(s_senddata[channel] <= 0 && s_ChanSock[channel] != -1)
		{
			VSNET_ClientMediaData(s_ChanSock[channel],TRUE);
		}
		s_senddata[channel]++;
	}
	else
	{
		s_senddata[channel] -= s_senddata[channel] > 0 ? 1 : 0;
		if(s_senddata[channel] <= 0 && s_ChanSock[channel] != -1)
		{
			VSNET_ClientMediaData(s_ChanSock[channel],FALSE);
		}
	}
	return 0;
}



void CLAUMp4TestDlg::OnBtnVstransuser() 
{
	CDlgVSTransUser dlg;
	dlg.DoModal();
}

void CLAUMp4TestDlg::OnBtnSysparam() 
{
	char string[10];
	int num;
	num = VSNET_ClientGetFrameRate(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
	sprintf(string,"%d",num);
	GetDlgItem(IDC_BTN_SYSPARAM)->SetWindowText(string);
}

void CLAUMp4TestDlg::OnCheckView() 
{
	((CButton*)GetDlgItem(IDC_CHECK_VIEW))->SetCheck(FALSE);
	if(s_ChanSock[g_GlobeEnvi.m_iSelWndSn] != -1)
	{
		if(pGet_SelChl()->m_bViewing)
		{
			BOOL bSuc = VSNET_ClientStopViewEx(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
			if(bSuc)
			{
				pGet_SelChl()->m_bViewing = FALSE;
			}
		}
		else
		{
			CString str;
			BOOL bSuc = VSNET_ClientStartViewEx(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
			if(bSuc)
			{
				pGet_SelChl()->m_bViewing = TRUE;
			}
			str.LoadString(IDS_STRING132);
			if(!VSNET_ClientEncKeyFrame(s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_sername,
				s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->url,
				s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_ch,
				s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_username,
				s_pVSChann[g_GlobeEnvi.m_iSelWndSn]->m_password,
				(WORD)s_port[g_GlobeEnvi.m_iSelWndSn]))
				MessageBox(str);
		}	
	}
	//Invalidate();
	SetBtnStatus();
}

void CLAUMp4TestDlg::OnBtnClear() 
{
	if(m_hostalarmmutex != NULL)
		WaitForSingleObject(m_hostalarmmutex,-1);
	
	m_listalarm.DeleteAllItems();
	if(m_hostalarmmutex != NULL)
		ReleaseMutex(m_hostalarmmutex);
}

void CLAUMp4TestDlg::OnBtnBradcast() 
{
	CDlgAudioTalk dlg;
	dlg.DoModal();
}

void CLAUMp4TestDlg::OnButton2() 
{
	CFileDialog dlgFile(
		TRUE, 
		_T("open License Cmmmand File"),
		"",
		OFN_HIDEREADONLY,
		"ptz Files(*.lic)|*.lic||",
		NULL);
	char lpszTitle[]={""};
	dlgFile.m_ofn.lpstrTitle = lpszTitle;
	char lppztfile[MAX_PATH];
	int iRet;
	char url[40];
	char sername[24];
	char user[20];
	char password[20];
	int size;
	FILE *fpFile = NULL;
	char  *pbuff = NULL;

	UpdateData(TRUE);
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);


	CString str;
	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}
	sprintf(lppztfile,"%s",dlgFile.GetPathName());
	
	fpFile = fopen(lppztfile,"rb");
	if(fpFile)
	{
		fseek(fpFile,0,SEEK_END);
		size = ftell(fpFile);
		fseek(fpFile,0,SEEK_SET);
		if(size > 1024)
		{
			fclose(fpFile);
			str.LoadString(IDS_STRING133);
			AfxMessageBox("�ļ�̫��");
			return;
		}
		pbuff = new char[size + 1];
		size = fread(pbuff,1,size,fpFile);
		pbuff[size] = '\0';
		fclose(fpFile); 
		fpFile = NULL;
	}
	else
	{
		str.LoadString(IDS_STRING134);
		AfxMessageBox(str);
		return;
	}
	iRet = VSNET_ClientSetLicenseFile(sername,url,user,password,pbuff,size,m_port);
	if(iRet == 0)
	{
		str.LoadString(IDS_STRING135);
		AfxMessageBox(str);
	}
	else
	{
		str.LoadString(IDS_STRING136);
		AfxMessageBox(str);
	}
	if(pbuff) delete pbuff;
}

void CLAUMp4TestDlg::OnButton3() 
{
	int iRet;
	char url[40];
	char sername[24];
	char user[20];
	char password[20];
	CString str;
	unsigned char pbuff[24];
	UpdateData(TRUE);
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);

	iRet = VSNET_ClientGetSerialNumData(sername,url,user,password,(char*)pbuff,m_port);
	if(iRet)
	{
		str.LoadString(IDS_STRING137);
		AfxMessageBox(str);
		TRACE("serial data: ");
		for(iRet=0;iRet<24;iRet++)
		{
			TRACE("%x ",pbuff[iRet]);
		}
		TRACE("\n");
		strcpy((char*)pbuff,"Hello World");  //��������24λ
		VSNET_ClientSetSerialNumData(sername,url,user,password,(char*)pbuff,m_port);


		strcpy((char*)pbuff,"12345678");  //��������8λ
		VSNET_ClientSetSerialNumPass(sername,url,user,password,(char*)pbuff,m_port);
	}
	else
	{
		str.LoadString(IDS_STRING138);
		AfxMessageBox(str);
	}

/*	iRet = VSNET_ClientGetLicenseFile(sername,url,user,password,licdatacallback,this,m_port);
	if(iRet == 1)
	{
		AfxMessageBox("��ȡlicense�ļ��ɹ�!");
	}
	else
	{
		AfxMessageBox("��ȡlicense�ļ�ʧ��!");
	}	*/
}

void CLAUMp4TestDlg::OnBtnCapture() 
{
	char url[40];
	char sername[24];
	char user[20];
	char password[20];
	UpdateData(TRUE);
	sprintf(sername,"%s",m_sername);
	sprintf(url,"%s",m_url);
	sprintf(user,"%s",m_username);
	sprintf(password,"%s",m_password);

	if(s_jpegHandle != -1)
		VSNET_ClientJpegCapStop(s_jpegHandle);
	s_jpegHandle = VSNET_ClientJpegCapStart(sername,url,user,password,m_port,jpegdatacallback,NULL);	
	if(s_jpegHandle != -1)
		VSNET_ClientJpegCapSingle(s_jpegHandle,pGet_SelChl()->m_iChlSn,100);//�ڶ�������Ҫע�⣬ֵ��ǰͨ����ͨ����ţ����ǵ�ǰ���ں�
}

void CLAUMp4TestDlg::OnButton4() 
{
/*	VSNETVIDEOMOVE_BLOCK block;
	VSNET_ClientGetVideoMove(s_ChanSock[0],&block);
	TRACE("Video Move block:%d,%d\n",block.m_channel,block.m_detectall);*/
	char m_infourl[30] = "220.112.14.181";
	WORD m_wInfoport = 7788;
	char m_sername[30] = "democam";
	char m_pgetip[30];
	WORD m_wgetport;
	VSNET_ClientInfoGetServerIP(m_infourl,m_wInfoport,m_sername,m_pgetip,&m_wgetport);
	TRACE("m_pgetip:%s,m_wgetport:%d\n",m_pgetip,m_wgetport);
}

void CLAUMp4TestDlg::OnClkVideoWnd(int Num, int Id)
{
	g_GlobeEnvi.m_iSelWndSn = Num;
	CString s;
	s.Format(IDS_STRING139, Num + 1, "/16");
	GetDlgItem(IDC_STATIC_SELWND)->SetWindowText(s);
	SetBtnStatus();
}
void CLAUMp4TestDlg::OnStaticVideo5() 
{
	OnClkVideoWnd(4,IDC_STATIC_VIDEO5);	
}

void CLAUMp4TestDlg::OnStaticVideo6() 
{
	OnClkVideoWnd(5,IDC_STATIC_VIDEO6);
}

void CLAUMp4TestDlg::OnStaticVideo7() 
{
	OnClkVideoWnd(6,IDC_STATIC_VIDEO7);
}

void CLAUMp4TestDlg::OnStaticVideo8() 
{
	OnClkVideoWnd(7,IDC_STATIC_VIDEO8);
}

void CLAUMp4TestDlg::OnStaticVideo9() 
{
	OnClkVideoWnd(8,IDC_STATIC_VIDEO9);
}

void CLAUMp4TestDlg::OnStaticVideo10() 
{
	OnClkVideoWnd(9,IDC_STATIC_VIDEO10);
}

void CLAUMp4TestDlg::OnStaticVideo11() 
{
	OnClkVideoWnd(10,IDC_STATIC_VIDEO11);
}

void CLAUMp4TestDlg::OnStaticVideo12() 
{
	OnClkVideoWnd(11,IDC_STATIC_VIDEO12);
}

void CLAUMp4TestDlg::OnStaticVideo13() 
{
	OnClkVideoWnd(12,IDC_STATIC_VIDEO13);
}

void CLAUMp4TestDlg::OnStaticVideo14() 
{
	OnClkVideoWnd(13,IDC_STATIC_VIDEO14);
}

void CLAUMp4TestDlg::OnStaticVideo15() 
{
	OnClkVideoWnd(14,IDC_STATIC_VIDEO15);
}

void CLAUMp4TestDlg::OnStaticVideo16() 
{
	OnClkVideoWnd(15,IDC_STATIC_VIDEO16);	
}

void CLAUMp4TestDlg::SetWndParkMode(int SingleLineMode)
{
	int Mode	= SingleLineMode;	//����ģʽ����1, 2, 3,4����Ӧ1,4,9,16����
	int WndCnt	= Mode * Mode;		//��������
	
	//�ӵ�ǰͨ����ʼ��WndCnt��ͨ�������°�����ʾλ�ã�����16���棬���0ͨ����ʼ��λ
	int FirstWndToShow = (Mode == 4) ? (0) : (g_GlobeEnvi.m_iSelWndSn);
	for (int i = 0; i < WndCnt; i ++)
	{
		g_VideoWndInfo[(FirstWndToShow + i) % MAX_VIDEOWND].m_pShowWnd->ShowWindow(SW_SHOW);

		//CRect mr;
		//CRect* m_pRect = GetWndRect(i + 1, Mode, &mr);
		CRect m_rect;
		GetWndRect(i + 1, Mode, &m_rect);
		g_VideoWndInfo[(FirstWndToShow + i) % MAX_VIDEOWND].m_pShowWnd->MoveWindow(m_rect);
		//delete m_pRect;
		//m_pRect = NULL;
	}

	//���ര��ȫ������
	int FirstWndToHide = (FirstWndToShow + WndCnt) % MAX_VIDEOWND;
	for (int j = 0; j < MAX_VIDEOWND - WndCnt; j ++)
	{
		g_VideoWndInfo[(FirstWndToHide + j) % MAX_VIDEOWND].m_pShowWnd->ShowWindow(SW_HIDE);
	}
}

void CLAUMp4TestDlg::GetWndRect(int CurWndSn, int SingleLineMode, CRect* m_rstRect)//���㻭��λ�úʹ�С,CurWndSn��ǰ������ţ���1��ʼ��SingleLineMode������ģʽ��1,2,3,4����1,4,9,16
{
	int SLWid	= 420;				//���߳��ȣ�������2*3��������
	int i		= CurWndSn;
	int mode	= SingleLineMode;

	//���������
	int levelX = (i % mode == 0) ? (mode) : (i % mode);	//�жϵ�ǰ���ڴ�����һ�У���1��ʼ��
	int x = 5 + (levelX - 1) * (SLWid / mode);
	
	//����������
	int temp = i - mode;
	int levelY = 1;
	while(temp > 0)										//�жϵ�ǰ���ڴ�����һ�У���1��ʼ��
	{
		levelY ++;
		temp = temp - mode;
	}
	int y = 5 + (levelY - 1) * (SLWid / mode);
	
	//���ؾ���
	m_rstRect->left	= x;
	m_rstRect->top		= y;
	m_rstRect->right	= (SLWid / mode) + x;
	m_rstRect->bottom	= (SLWid / mode) + y;
	//CRect* m_pRect = new CRect(x, y, (SLWid / mode) + x, (SLWid / mode) + y);
	//return m_pRect;
}

void CLAUMp4TestDlg::OnSelchangeCOMBOMode() 
{
	UpdateData(TRUE);
	int Mode = m_CB_iMode + 1;
	SetWndParkMode(Mode);
}

int CLAUMp4TestDlg::iGet_SelChlNo()
{
	return g_VideoWndInfo[g_GlobeEnvi.m_iSelWndSn].m_iChlSn;
}

VIDEOCHLINFO* CLAUMp4TestDlg::pGet_SelChl()
{
	return &(g_VideoChlInfo[g_VideoWndInfo[g_GlobeEnvi.m_iSelWndSn].m_iChlSn]);
}

void CLAUMp4TestDlg::PTZActSafeCtrl(int type, int val)	//������̨���˶�
{
	LONG m_hChlLnk = s_ChanSock[iGet_SelChlNo()];
	if(m_hChlLnk)
	{
		BOOL bSuc = VSNET_ClientPTZCtrl(m_hChlLnk, type, val);//���������˶��Ŀ⺯��

		//ִ�гɹ����޸�״̬λ
		if(bSuc)
		{
			//���ҿ��������Ӧ��״̬λ��־
			int PTZ_STA = 0x00000;
			if((type == PTZ_LEFT) | (type == PTZ_LEFTSTOP))
			{
				PTZ_STA = PTZ_STA_LEFT;
			}
			else if((type == PTZ_RIGHT) | (type == PTZ_RIGHTSTOP))
			{
				PTZ_STA = PTZ_STA_RIGHT;
			}
			else if((type == PTZ_UP) | (type == PTZ_UPSTOP))
			{
				PTZ_STA = PTZ_STA_UP;
			}
			else if((type == PTZ_DOWN) | (type == PTZ_DOWNSTOP))
			{
				PTZ_STA = PTZ_STA_DOWN;
			}
			else if((type == PTZ_AUTO) | (type == PTZ_AUTOSTOP))
			{
				PTZ_STA = PTZ_STA_AUTO;
			}

			//��״̬λΪ1���������λ����
			if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA)
			{
				pGet_SelChl()->m_iPTZActStatus = 0x00000;
			}
			//��״̬λΪ1�����˶�״̬��־ֱ�Ӹ���״̬λ
			//���ÿ�����ǰ��״̬λ����Ϊ����ִ�е��˶����������˶������ͻʱ����Ӻ���
			else
			{
				pGet_SelChl()->m_iPTZActStatus = PTZ_STA;			
			}
		}
		//ˢ�°�ť״̬
		SetBtnStatus();
	}
}

void CLAUMp4TestDlg::OnBtnPtzleft() 
{
	if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA_LEFT)
	{
		PTZActSafeCtrl(PTZ_LEFTSTOP, 10);
	}
	else
	{
		PTZActSafeCtrl(PTZ_LEFT, 10);
	}
}

void CLAUMp4TestDlg::OnBTNPTZRight() 
{
	if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA_RIGHT)
	{
		PTZActSafeCtrl(PTZ_RIGHTSTOP, 10);
	}
	else
	{
		PTZActSafeCtrl(PTZ_RIGHT, 10);
	}
}

void CLAUMp4TestDlg::OnBtnPtzup() 
{
	if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA_UP)
	{
		PTZActSafeCtrl(PTZ_UPSTOP, 10);
	}
	else
	{
		PTZActSafeCtrl(PTZ_UP, 10);
	}
}

void CLAUMp4TestDlg::OnBtnPtzdown() 
{
	if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA_DOWN)
	{
		PTZActSafeCtrl(PTZ_DOWNSTOP, 10);
	}
	else
	{
		PTZActSafeCtrl(PTZ_DOWN, 10);
	}
}

void CLAUMp4TestDlg::OnBtnPtzauto() 
{
	if(pGet_SelChl()->m_iPTZActStatus & PTZ_STA_AUTO)
	{
		PTZActSafeCtrl(PTZ_AUTOSTOP, 0);
	}
	else
	{
		PTZActSafeCtrl(PTZ_AUTO, 0);
	}
}

void CLAUMp4TestDlg::OnBtnPreset() 
{
	char lpstr[5];
	int type =	PTZ_SETPOINT;
	GetDlgItem(IDC_EDIT_PRESETNUM)->GetWindowText(lpstr,4);
	int val = atoi(lpstr);
	val = val > 255 ? 255 : val;

	LONG m_hChlLnk = s_ChanSock[iGet_SelChlNo()];
	if(m_hChlLnk)
	{
		BOOL bSuc = VSNET_ClientPTZCtrl(m_hChlLnk, type, val);
	}
}

void CLAUMp4TestDlg::OnBtnCall() 
{
	char lpstr[5];
	int type =	PTZ_GOTOPOINT;
	GetDlgItem(IDC_EDIT_PRESETNUM)->GetWindowText(lpstr,4);
	int val = atoi(lpstr);
	val = val > 255 ? 255 : val;
	
	LONG m_hChlLnk = s_ChanSock[iGet_SelChlNo()];
	if(m_hChlLnk)
	{
		BOOL bSuc = VSNET_ClientPTZCtrl(m_hChlLnk, type, val);
	}
}

//void CLAUMp4TestDlg::OnBtnBmpcapture() 
//{
	//int xxxx;
//	int fps = 25;
//	LONG m_hChl = s_ChanSock[iGet_SelChlNo()];
//	if(m_hChl != -1)
//	{
		//VSNET_ClientCapturePicture(m_hChl, "C:\\Users\\rsq\\Desktop\\BMPCAPTURE.bmp");
		//xxxx = VSNET_ClientGetFrameRate(s_ChanSock[g_GlobeEnvi.m_iSelWndSn]);
//		namedWindow("frame");

		

//		while (true)
//		{
//			VSNET_ClientCapturePicture(m_hChl, "C:\\Users\\rsq\\Desktop\\BMPCAPTURE.bmp");
//			Mat frame = imread("C:\\Users\\rsq\\Desktop\\BMPCAPTURE.bmp");
//			imshow("frame", frame);
//			waitKey(1000 / fps);

//		}
	
//	}
//}

void CLAUMp4TestDlg::OnBtnBmpcapture()//getFrame()
{
	LONG m_hChl = s_ChanSock[iGet_SelChlNo()];
  ObjTracking(m_hChl);
}

void CLAUMp4TestDlg::OnBtnServerjpgcapture() 
{
	LONG m_hChl = s_ChanSock[iGet_SelChlNo()];
	CString str;
	if(m_hChl != -1)
	{
		UpdateData(TRUE);
		
		char sername[24];
		char url[40];
		char user[20];
		char password[20];
		WORD port;

		sprintf(sername,"%s",m_sername);
		sprintf(url,"%s",m_url);
		sprintf(user,"%s",m_username);
		sprintf(password,"%s",m_password);
		port = m_port;
		
		BOOL bSuc = VSNET_ServerCapJPEG(sername, url, m_channel, user, password, 80, port);
		if(!bSuc)
		{
			str.LoadString(IDS_STRING140);
			AfxMessageBox(str);
		}
	}
}

LRESULT CLAUMp4TestDlg::OnMsgSthHappened(WPARAM wParam, LPARAM lParam)
{
	MSGCALLBKPARAM* pMsgCallBk = (MSGCALLBKPARAM*)lParam;
	VIDEOCHLINFO* pChnlMsgFrom = pHandleToChl(pMsgCallBk->hHandle);
	CString str;
	if(pChnlMsgFrom != NULL)
	{
		//׼������
		CString sServerName = pChnlMsgFrom->m_sServerName;
		CString sServerUrl  = pChnlMsgFrom->m_sServerUrl;
		CString sPort;
		sPort.Format("%d", pChnlMsgFrom->m_iPort);
		CString sChlSn;
		sChlSn.Format("%d", pChnlMsgFrom->m_iChlSn + 1);
		char chMsg[100];
		TranslateMsg(pMsgCallBk, chMsg);

		if (LAUMSG_LINKMSG == pMsgCallBk->wParam)
		{
			//�������
			WaitForSingleObject(m_hMsgListWriteMutex, -1);
			int nItem = m_MainDlgWnd->m_listalarm.InsertItem(0, sServerName);
			m_MainDlgWnd->m_listalarm.SetItemText(nItem, 1, sServerUrl);
			m_MainDlgWnd->m_listalarm.SetItemText(nItem, 2, sPort);
			m_MainDlgWnd->m_listalarm.SetItemText(nItem, 3, sChlSn);
			m_MainDlgWnd->m_listalarm.SetItemText(nItem, 4, chMsg);
			ReleaseMutex(m_hMsgListWriteMutex);
		}
		else
		{
			//������Ϣ
			if (pMsgCallBk->lParam & 1 << pChnlMsgFrom->m_iChlSn)
			{
                //�������
				WaitForSingleObject(m_hMsgListWriteMutex, -1);
				int nItem = m_MainDlgWnd->m_listalarm.InsertItem(0, sServerName);
				m_MainDlgWnd->m_listalarm.SetItemText(nItem, 1, sServerUrl);
				m_MainDlgWnd->m_listalarm.SetItemText(nItem, 2, sPort);
				m_MainDlgWnd->m_listalarm.SetItemText(nItem, 3, sChlSn);
				m_MainDlgWnd->m_listalarm.SetItemText(nItem, 4, chMsg);
				ReleaseMutex(m_hMsgListWriteMutex);
			}
		}
	}
	else
	{
		str.LoadString(IDS_STRING141);
		AfxMessageBox(str);
	}

	//���ָ��
	delete pMsgCallBk;
	pMsgCallBk = NULL;
	return TRUE;
}

VIDEOCHLINFO* CLAUMp4TestDlg::pHandleToChl(LONG mhHandle)
{
	for(int i = 0; i < MAX_VIDEOWND; i ++)
	{
		if(s_ChanSock[i] == mhHandle)
			return &(g_VideoChlInfo[i]);
	}
	return NULL;
}

void CLAUMp4TestDlg::TranslateMsg(MSGCALLBKPARAM* pMsgCallBkParam, char* chMsg)
{
	CString sMsgType = "";
	CString sMsgData = "";
	switch(pMsgCallBkParam->wParam)
	{
	case  LAUMSG_LINKMSG:
		{
//			sMsgType = "���ӷ�������Ϣ:";
			sMsgType.LoadString(IDS_STRING142);
			if(pMsgCallBkParam->lParam == 0)
			{
//				sMsgData = "���ӳɹ�";
				sMsgData.LoadString(IDS_STRING143);
			}
			else if(pMsgCallBkParam->lParam ==1)
			{
//				sMsgData = "�û�ֹͣ������";
				sMsgData.LoadString(IDS_STRING144);
			}
			else if(pMsgCallBkParam->lParam ==2)
			{
//				sMsgData = "����ʧ��, �����Զ�����";
				sMsgData.LoadString(IDS_STRING145);

				//�Զ�����
				OnClkVideoWnd(pHandleToChl(pMsgCallBkParam->hHandle)->m_iWndSn,0);
				StopChlActs();
				PlayChl();
			}
			else if(pMsgCallBkParam->lParam == 3)
			{
				//sMsgData = "���ӶϿ�, �����Զ�����";
				sMsgData.LoadString(IDS_STRING146);
				//�Զ�����
				OnClkVideoWnd(pHandleToChl(pMsgCallBkParam->hHandle)->m_iWndSn,0);
				StopChlActs();
				PlayChl();
			}
			else if(pMsgCallBkParam->lParam == 4)
			{
				//sMsgData = "�˿ڳ�ͻ";
				sMsgData.LoadString(IDS_STRING147);
			}
			else if(pMsgCallBkParam->lParam == 5)
			{
				//sMsgData = "�����ڴ�ʧ��";
				sMsgData.LoadString(IDS_STRING148);
			}
			else if(pMsgCallBkParam->lParam == 6)
			{
				//sMsgData = "��������������ʧ��";
				sMsgData.LoadString(IDS_STRING149);
			}
			else if(pMsgCallBkParam->lParam == -102)
			{
				//sMsgData = "�û����������";
				sMsgData.LoadString(IDS_STRING150);
			}
			else if(pMsgCallBkParam->lParam == -103)
			{
				//sMsgData = "ϵͳ�û���Ա";
				sMsgData.LoadString(IDS_STRING151);
			}
			else if(pMsgCallBkParam->lParam == -105)
			{
				//sMsgData = "ͨ���û���Ա";
				sMsgData.LoadString(IDS_STRING152);
			}
			else if(pMsgCallBkParam->lParam == -106)
			{
			//	sMsgData = "û��ָ����ͨ��";
				sMsgData.LoadString(IDS_STRING153);
			}
			else if(pMsgCallBkParam->lParam == -112)
			{
				//sMsgData = "û���ҵ�������";
				sMsgData.LoadString(IDS_STRING154);
			}
			else
			{
				//sMsgData = "δ֪";
				sMsgData.LoadString(IDS_STRING109);
			}
			break;
		}		
	case LAUMSG_VIDEOMOTION:
//	    sMsgType = "��Ƶ�ƶ�����";
		sMsgType.LoadString(IDS_STRING155);
		break;
	case LAUMSG_VIDEOLOST:
//	    sMsgType = "��Ƶ��ʧ����";
		sMsgType.LoadString(IDS_STRING156);
		break;
	case LAUMSG_ALARM:
		{
//	        sMsgType = "̽ͷ����:";
			sMsgType.LoadString(IDS_STRING157);
			int i;
			for(i = 0; i < 4; i ++)
			{
				if( (pMsgCallBkParam->lParam) & ( 1 << i) )
				{
					break;
				}
			}
			sMsgData.Format(IDS_STRING158, i + 1);
			break;
		}
	case LAUMSG_OUTPUTSTATUS:
//	    sMsgType = "�������״̬";
		sMsgType.LoadString(IDS_STRING159);
		break;
	case LAUMSG_CURSWITCHCHAN:
//		sMsgType = "��ǰ�л�ͨ��";
		sMsgType.LoadString(IDS_STRING160);
		break;
	case LAUMSG_HIDEALARM:
//		sMsgType = "ͼ���ڵ�����";
		sMsgType.LoadString(IDS_STRING161);
		break;
	case LAUMSG_CHRECORD:
//		sMsgType = "��ʼ��ֹͣ¼��";
		sMsgType.LoadString(IDS_STRING162);
		break;
	case LAUMSG_EACHVIDEOMOTION:
//		sMsgType = "�ֿ���Ƶ�ƶ�";
		sMsgType.LoadString(IDS_STRING163);
		break;
	case LAUMSG_INTERNAL:
//		sMsgType = "�ڲ���Ϣ";
		sMsgType.LoadString(IDS_STRING164);
		break;
	case LAUMSG_SERVERRECORD:
//		sMsgType = "������¼����Ϣ";
		sMsgType.LoadString(IDS_STRING165);
		break;
	case LAUMSG_STARTCLIENTRECORD:
//		sMsgType = "�ͻ���¼��������Ϣ";
		sMsgType.LoadString(IDS_STRING166);
		break;
	case LAUMSG_STOPCLIENTRECORD:
//		sMsgType = "�ͻ���¼��ֹͣ��Ϣ";
		sMsgType.LoadString(IDS_STRING167);
		break;
	case LAUMSG_CLIENTRECORDSTATUS:
//		sMsgType = "�ͻ���¼��״̬��Ϣ";
		sMsgType.LoadString(IDS_STRING168);
		break;
	case LAUMSG_EXTENDALARM_TEMPERATURE:
//		sMsgType = "�¶ȱ�����Ϣ";
		sMsgType.LoadString(IDS_STRING169);
		break;
	case LAUMSG_EXTENDALARM_HUMIDITY:
//		sMsgType = "ʪ�ȱ�����Ϣ";
		sMsgType.LoadString(IDS_STRING170);
		break;
	default:
//		sMsgType = "δ֪";
		sMsgType.LoadString(IDS_STRING109);
		break;
	}
	sprintf(chMsg, "%s%s", sMsgType, sMsgData);
}

