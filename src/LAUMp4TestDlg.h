// LAUMp4TestDlg.h : header file
#include "luserparamx.h"
//{{AFX_INCLUDES()
#include "luserparamx.h"
//}}AFX_INCLUDES
//

#if !defined(AFX_LAUMP4TESTDLG_H__8109FF41_7950_4188_B68C_84F8410CE982__INCLUDED_)
#define AFX_LAUMP4TESTDLG_H__8109FF41_7950_4188_B68C_84F8410CE982__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShowWnd.h"
/////////////////////////////////////////////////////////////////////////////
// CLAUMp4TestDlg dialog
typedef struct
{
	BOOL		m_bLocalRecing;			//����¼���־
	int			m_iPTZActStatus;		//��̨�˶���־��0��ֹ��0x01000��ת��0x00100��ת��0x00010��ת��0x00001��ת��0x100000��ת
	BOOL		m_bViewing;				//Ԥ����־
	BOOL		m_bRedirecting;			//ת����־
	CString		m_sServerName;			//��������
	CString		m_sServerUrl;			//������URL
	int			m_iPort;				//�˿ں�
	CString		m_sUserName;			//�û���
	CString		m_sPsw;					//����
	int			m_iChlSn;				//ͨ����ţ���4��ͨ���еĵڼ���
	int			m_iTransType;			//0,UDP; 1,�ಥ; 2,TCP
	LONG		m_hLnk;					//���Ӿ��
	CString		m_sMultiCastURL;		//�ಥ��ַ
	int			m_iMultiCastPort;		//�ಥ�˿�
	int			m_iMultiCastLocalPort;	//�ಥ���ض˿�
	int			m_iWndSn;				//��������ĸ�����
}VIDEOCHLINFO;

typedef struct
{
//	int			m_iCtrlID;				//�ؼ�ID������IDC_STATIC_VIDEO1
	CShowWnd*	m_pShowWnd;				//��Ƶ��ʾ�ؼ�����
	int			m_iChlSn;				//��Ӧ��Ƶͨ����ţ���0,1,2,3,4,5...�봰�������1 : n (n >=0 )��ϵ
}VIDEOWNDINFO;

typedef struct
{
	int			m_iSelWndSn;			//��ǰѡ�д������
}GLOBEENVI;

typedef struct
{
	LONG hHandle;
	int wParam;
	int lParam;
	void *context;			
}MSGCALLBKPARAM;  //��Ϣ��ͨ����������û��ͨ���ֻ࣬�����������н��ܣ�Ϊ���ڴ������б����ͨ������������Ӿ�����͹�ȥ


class CLAUMp4TestDlg : public CDialog
{
// Construction
public:
	CLAUMp4TestDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CLAUMp4TestDlg)
	enum { IDD = IDD_LAUMP4TEST_DIALOG };
	CShowWnd	m_video14;
	CShowWnd	m_video16;
	CShowWnd	m_video15;
	CShowWnd	m_video13;
	CShowWnd	m_video12;
	CShowWnd	m_video11;
	CShowWnd	m_video10;
	CShowWnd	m_video9;
	CShowWnd	m_video8;
	CShowWnd	m_video7;
	CShowWnd	m_video6;
	CShowWnd	m_video5;
	CListCtrl	m_listalarm;
	CSliderCtrl	m_VolumeCtrl;
	CShowWnd	m_video4;
	CShowWnd	m_video3;
	CShowWnd	m_video2;
	CShowWnd	m_video1;
	CString	m_sername;
	CString	m_url;
	CString	m_username;
	CString	m_password;
	int		m_channel;
	int		m_trantype;
	int		m_port;
	CString	m_tranmultiurl;
	int		m_tranmultiport;
	int		m_tranlocalport;
	int		m_CB_iMode;
	CLUSerParamX	m_paramctrl;
	//}}AFX_DATA

	VIDEOWNDINFO	g_VideoWndInfo[MAX_VIDEOWND];	//������Ϣ
	VIDEOCHLINFO	g_VideoChlInfo[MAX_VIDEOWND];	//ͨ����Ϣ
	GLOBEENVI		g_GlobeEnvi;					//ȫ����Ϣ
	
	LONG		    m_hTalkHandle;   //�Խ����

	void	OnClkVideoWnd(int Num, int Id);
	void	SetWndParkMode(int SingleLineMode);
	void	GetWndRect(int CurWndSn, int SingleLineMode, CRect* m_rstRect);
	void	PlayStopSwitch();
	int		iGet_SelChlNo();
	VIDEOCHLINFO* pGet_SelChl();
	VIDEOCHLINFO* pHandleToChl(LONG mhHandle);
	void	PTZActSafeCtrl(int type, int val);
	void	TranslateMsg(MSGCALLBKPARAM* pMsgCallBkParam, char* chMsg);
	void	StopAllChlActs();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLAUMp4TestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CLAUMp4TestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStaticVideo1();
	afx_msg void OnStaticVideo2();
	afx_msg void OnStaticVideo3();
	afx_msg void OnStaticVideo4();
	afx_msg void OnDbStatic1();
	afx_msg void OnDbStatic2();
    afx_msg void OnDbStatic3();
    afx_msg void OnDbStatic4();
	afx_msg void PlayChl();
	afx_msg void StopChlActs();
	afx_msg void OnDestroy();
	afx_msg void OnBtnUpdate();
	virtual void OnCancel();
	afx_msg void OnBtnConfig();
	afx_msg void OnBtnReset();
	afx_msg void OnBtnUserip();
	afx_msg void OnBtnTransmit();
	afx_msg void TransmitStart();
	afx_msg void TransmitStop();
	afx_msg void OnBtnTalk();
	afx_msg void LocalRecStart();
	afx_msg void LocalRecStop();
	afx_msg void OnBtnLocalRec();
	afx_msg void OnBtnUarttran();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnOutofmemorySliderVoice(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnVstransuser();
	afx_msg void OnBtnSysparam();
	afx_msg void OnCheckView();
	afx_msg void OnBtnClear();
	afx_msg void OnBtnBradcast();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnBtnCapture();
	afx_msg void OnButton4();
	afx_msg void OnStaticVideo5();
	afx_msg void OnStaticVideo6();
	afx_msg void OnStaticVideo7();
	afx_msg void OnStaticVideo8();
	afx_msg void OnStaticVideo9();
	afx_msg void OnStaticVideo10();
	afx_msg void OnStaticVideo11();
	afx_msg void OnStaticVideo12();
	afx_msg void OnStaticVideo13();
	afx_msg void OnStaticVideo14();
	afx_msg void OnStaticVideo15();
	afx_msg void OnStaticVideo16();
	afx_msg void OnSelchangeCOMBOMode();
	afx_msg void OnBtnPtzleft();
	afx_msg void OnBTNPTZRight();
	afx_msg void OnBtnPtzup();
	afx_msg void OnBtnPtzdown();
	afx_msg void OnBtnPtzauto();
	afx_msg void OnBtnBmpcapture();
	afx_msg void OnBtnServerjpgcapture();
	afx_msg LRESULT OnMsgSthHappened(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnPreset();
	afx_msg void OnBtnCall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetBtnStatus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUMP4TESTDLG_H__8109FF41_7950_4188_B68C_84F8410CE982__INCLUDED_)
