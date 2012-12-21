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

CCavanThread::~CCavanThread()
{
	Stop();
}

void CCavanThread::ThreadHandler(void *data)
{
	CCavanThread *thread = (CCavanThread *)data;

	thread->mLock.Lock();
	thread->mState = CAVAN_THREAD_STATE_RUNNING;

	while (thread->mState != CAVAN_THREAD_STATE_STOPPING)
	{
		thread->mLock.Unlock();

		if (thread->Run() == false)
		{
			break;
		}

		thread->mLock.Lock();
	}

	thread->mState = CAVAN_THREAD_STATE_STOPED;
	thread->mLock.Unlock();

	_endthread();
}

bool CCavanThread::Start(int index)
{
	CSingleLock lock(&mLock);

	if (mState == CAVAN_THREAD_STATE_STOPED)
	{
		mIndex = index;
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

CCavanService::~CCavanService()
{
	Stop();
}

void CCavanService::DestoryThreads(void)
{
	CCavanThread *thread = mThreadHead;

	while (thread)
	{
		CCavanThread *next = thread->next;

		delete thread;
		thread = next;
	}

	mThreadHead = NULL;
	mDaemonCount = 0;
}

bool CCavanService::Start(void)
{
	CSingleLock lock(&mLock);

	if (mThreadHead == NULL || mDaemonCount == 0)
	{
		return false;
	}

	for (CCavanThread *thread = mThreadHead; thread; thread = thread->next)
	{
		thread->Start(thread - mThreadHead);
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
