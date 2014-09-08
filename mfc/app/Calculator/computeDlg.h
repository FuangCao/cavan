// computeDlg.h : header file
//

#if !defined(AFX_COMPUTEDLG_H__B7458E43_3A78_4056_93B8_E2ECCC7353B7__INCLUDED_)
#define AFX_COMPUTEDLG_H__B7458E43_3A78_4056_93B8_E2ECCC7353B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CComputeDlg dialog
const int MAX=100;          //设定算式存储器的容量
class CComputeDlg : public CDialog
{
private:
	CString	Memorizer[7];    //运算结果存储器
	CString Formula[MAX];    //算式存储栈，的内存空间
	int canmove;             //对话框移动控制
	int computed;            //标志算式是否已经计算过
	int top1;                //算式存储栈的栈顶指针
	int top2;                //算式存储栈
	int focus;               //标志焦点所在位置,0：表示在算式编辑框，1：表示在结果或时间编辑框
	int time;
	// Construction
public:
	CComputeDlg(CWnd* pParent = NULL); // standard constructor
	virtual void OnOK();               //重载OnOK()函数
	void GetResult(const CString &s);  //获取存储器的内容
// Dialog Data
	//{{AFX_DATA(CComputeDlg)
	enum { IDD = IDD_COMPUTE_DIALOG };
	CEdit	m_time;
	CEdit	m_result;
	CEdit	m_compute;
	int		m_angle;
	int		m_type;
	int		m_MemorizerState;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComputeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	//{{AFX_MSG(CComputeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBUTTONcompute();
	afx_msg void OnBUTTONcls();
	afx_msg void OnBUTTONResult();
	afx_msg void OnBUTTONPreviousFormula();
	afx_msg void OnBUTTONNextFormula();
	afx_msg void OnButtonOff();
	afx_msg void OnBUTTONpoint();
	afx_msg void OnButtonKh1();
	afx_msg void OnButtonKh2();
	afx_msg void OnBUTTONback();
	afx_msg void OnBUTTONPi();
	afx_msg void OnBUTTONnCr();
	afx_msg void OnBUTTONnAr();
	afx_msg void OnBUTTONUp();
	afx_msg void OnBUTTONDown();
	afx_msg void OnBUTTONLeft();
	afx_msg void OnBUTTONRight();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRADIOangle();
	afx_msg void OnRADIOradian();
	afx_msg void OnRADIOcompute1();
	afx_msg void OnRADIOcompute2();
	afx_msg void OnChangeEDITcompute();
	afx_msg void OnSetfocusEDITcompute();
	afx_msg void OnRADIOWrite();
	afx_msg void OnRADIORead();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetfocusEDITresult();
	afx_msg void OnSetfocusEDITTime();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	afx_msg void InputNumber(UINT nID);
	afx_msg void InputWord(UINT nID);
	afx_msg void Angle(UINT nID);
	afx_msg void Memory(UINT nID);
	afx_msg void Operator1(UINT nID);
	afx_msg void Operator2(UINT nID);
	DECLARE_MESSAGE_MAP()
};
/*
	afx_msg void InputNumber(UINT nID);
	afx_msg void InputWord(UINT nID);
	afx_msg void Angle(UINT nID);
	afx_msg void Memory(UINT nID);
	afx_msg void Operator1(UINT nID);
	afx_msg void Operator2(UINT nID);
*/
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPUTEDLG_H__B7458E43_3A78_4056_93B8_E2ECCC7353B7__INCLUDED_)
