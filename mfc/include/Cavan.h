// Cavan.h: interface for the CCavan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAVAN_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_)
#define AFX_CAVAN_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <CavanDebug.h>
#include <CavanDevice.h>
#include <CavanText.h>
#include <CavanFile.h>

#define KB(a)					((a) << 10)
#define MB(a)					((a) << 20)
#define GB(a)					((a) << 30)
#define TB(a)					((a) << 40)
#define ARRAY_SIZE(a)			(sizeof(a) / sizeof(a[0]))
#define MAX(a, b)				((a) > (b) ? (a) : (b))
#define MIN(a, b)				((a) < (b) ? (a) : (b))
#define ABS(a)					((a) > 0 ? (a) : -(a))
#define COMBO_GET_TEXT(a, val)	(a.GetLBText(a.GetCurSel(), val))

#endif // !defined(AFX_CAVAN_H__E02C2E21_00AE_4C91_9F29_5AFF368BD5CB__INCLUDED_)
