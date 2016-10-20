//NetHostLib.h
#ifndef __NETHOSTLIB_H__
#define __NETHOSTLIB_H__

#ifndef CNET_APIIMPORT
#define CNET_APIIMPORT  extern "C"__declspec(dllexport)
#endif//CNET_APIIMPORT


typedef struct
{
	char *IpAddr;			/* ��������ַ */
	WORD  wPort;			/* �������˿ں� */
	BYTE *pMac;				/* MAC��ַ */
	char *pSerName;			/* ���������� */
	char *pSerialNo;		/* ���������к� */
	int   nAlarmChannel;	/* ����ͨ�� */
	int   nAlarmType;		/* �������� */
}SERVERALARMINFO;

typedef struct
{
	char *IpAddr;			/* ��������ַ */
	WORD  wPort;			/* �������˿� */
	BYTE *pMac;				/* MAC��ַ */
	char *pSerName;			/* ���������� */
	char *pSerialNo;		/* ���к� */
	int   nChannel;			/* ͨ���� */
	int   nyear;			/* �ļ�ʱ�� */
	BYTE  nmonth;
	BYTE  nday;
	BYTE  nhour;
	BYTE  nminute;
	BYTE  nsecond;

	int   nFileType;		/* 1��MPEG4������2��JPEGͼ�� */
	int   nRecvType;		/* ¼������ */
}SERVERRECVHEADINFO;

typedef void (WINAPI *pSerAlarmInfoCallback)(SERVERALARMINFO *pAlarmInfo,void *pUserData);
typedef void*(WINAPI *pSerRecvHeadCallback) (SERVERRECVHEADINFO *pRecvHead,void *pUserData);
typedef int  (WINAPI *pSerRecvDataCallback) (char *pBuffer,int size,void *context,void *pUserData);

CNET_APIIMPORT BOOL  __stdcall NetHostLibInit();
CNET_APIIMPORT BOOL  __stdcall NetHostLibFree();
CNET_APIIMPORT BOOL  __stdcall NetHostLibRegCallBack(pSerAlarmInfoCallback pAlarmCallback,void *pAlarmData,
													 pSerRecvHeadCallback pRecvHeadCallback,void *pHeadData,
													 pSerRecvDataCallback pRecvDataCallback,void *pDataData);

CNET_APIIMPORT BOOL  __stdcall NetHostAlarm_Start(char *m_plocalip,WORD m_wPort);
CNET_APIIMPORT BOOL  __stdcall NetHostAlarm_Stop();

#endif//__NETHOSTLIB_H__
