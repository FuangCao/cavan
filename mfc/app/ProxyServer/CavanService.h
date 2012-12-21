// CavanService.h: interface for the CCavanService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVANSERVICE_H__90B39A5C_16F7_42DF_916C_B65D3FA3ABBB__INCLUDED_)
#define AFX_CAVANSERVICE_H__90B39A5C_16F7_42DF_916C_B65D3FA3ABBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <AfxMt.h>

class CCavanTransport;
enum CProxyProcotolType;

enum CCavanThreadState
{
	CAVAN_THREAD_STATE_IDLE,
	CAVAN_THREAD_STATE_RUNNING,
	CAVAN_THREAD_STATE_STOPPING,
	CAVAN_THREAD_STATE_STOPED
};

class CCavanThread
{
private:
	CMutex mLock;
	CCavanThreadState mState;

protected:
	int mIndex;

public:
	CCavanThread *next;

protected:
	virtual bool Run(void) = 0;

public:
	CCavanThread(void) : next(NULL), mState(CAVAN_THREAD_STATE_STOPED) {}
	~CCavanThread(void);
	virtual void Prepare(CCavanTransport *trspService, WORD wProxyPort, CProxyProcotolType nProxyProtocol, DWORD dwProxyIP) = 0;
	virtual bool Start(int index);
	virtual void Stop(void);

private:
	static void ThreadHandler(void *data);
};

class CCavanService
{
private:
	CMutex mLock;

protected:
	int mDaemonCount;
	CCavanThread *mThreadHead;

public:
	CCavanService(void) : mThreadHead(NULL), mDaemonCount(0) {};
	virtual ~CCavanService(void);

	void DestoryThreads(void);
	virtual bool Start(void);
	virtual void Stop(void);
};

#endif // !defined(AFX_CAVANSERVICE_H__90B39A5C_16F7_42DF_916C_B65D3FA3ABBB__INCLUDED_)
