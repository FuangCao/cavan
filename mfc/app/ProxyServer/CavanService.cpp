// CavanService.cpp: implementation of the CCavanService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProxyServer.h"
#include "CavanService.h"
#include <process.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CCavanThread::ThreadHandler(void *data)
{
	CCavanThread *thread = (CCavanThread *)data;
	thread->MainLoop();
	_endthread();
}

void CCavanThread::MainLoop(void)
{
	mLock.Lock();
	mState = CAVAN_THREAD_STATE_RUNNING;

	while (mState != CAVAN_THREAD_STATE_STOPPING)
	{
		mLock.Unlock();

		if (Run() == false)
		{
			break;
		}

		mLock.Lock();
	}

	mState = CAVAN_THREAD_STATE_STOPED;
	mLock.Unlock();
}

bool CCavanThread::Start(void)
{
	CSingleLock lock(&mLock);

	if (mState == CAVAN_THREAD_STATE_STOPED)
	{
		mState = CAVAN_THREAD_STATE_IDLE;
		_beginthread(ThreadHandler, 0, this);
	}

	return true;
}

void CCavanThread::Stop(void)
{
	mLock.Lock();

	if (mState != CAVAN_THREAD_STATE_STOPED)
	{
		mState = CAVAN_THREAD_STATE_STOPPING;

		while (mState != CAVAN_THREAD_STATE_STOPED)
		{
			mLock.Unlock();
			Sleep(100);
			mLock.Lock();
		}
	}

	mLock.Unlock();
}

// ================================================================================

bool CCavanService::Start(void)
{
	CSingleLock lock(&mLock);

	if (mThreadHead == NULL || mDaemonCount == 0)
	{
		return false;
	}

	for (CCavanThread *thread = mThreadHead; thread; thread = thread->next)
	{
		thread->Start();
	}

	return true;
}

void CCavanService::Stop(void)
{
	CSingleLock lock(&mLock);

	for (CCavanThread *thread = mThreadHead; thread; thread = thread->next)
	{
		thread->Stop();
	}
}
