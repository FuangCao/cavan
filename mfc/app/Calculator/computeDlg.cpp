// computeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "compute.h"
#include "computeDlg.h"
#include "Multinomial.h"     //用于算术及多项式

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

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
	//}}AFx_VIRTUAL

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
// CComputeDlg dialog

CComputeDlg::CComputeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComputeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CComputeDlg)
	m_angle = 0;
	m_type = 0;
	m_MemorizerState = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	canmove=0;
	focus=0;
	computed=0;
	top1=top2=0;
	for(int i=0;i<MAX;i++)
		Formula[i]="\0";
	for(i=0;i<7;i++)
		Memorizer[i]="\0";
}

void CComputeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CComputeDlg)
	DDX_Control(pDX, IDC_EDIT_Time, m_time);
	DDX_Control(pDX, IDC_EDIT_result, m_result);
	DDX_Control(pDX, IDC_EDIT_compute, m_compute);
	DDX_Radio(pDX, IDC_RADIO_angle, m_angle);
	DDX_Radio(pDX, IDC_RADIO_compute1, m_type);
	DDX_Radio(pDX, IDC_RADIO_Write, m_MemorizerState);
	//}}AFX_DATA_MAP

}
BEGIN_MESSAGE_MAP(CComputeDlg, CDialog)
	//{{AFX_MSG_MAP(CComputeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_compute, OnBUTTONcompute)
	ON_BN_CLICKED(IDC_BUTTON_cls, OnBUTTONcls)
	ON_BN_CLICKED(IDC_BUTTON_Result, OnBUTTONResult)
	ON_BN_CLICKED(IDC_BUTTON_PreviousFormula, OnBUTTONPreviousFormula)
	ON_BN_CLICKED(IDC_BUTTON_NextFormula, OnBUTTONNextFormula)
	ON_BN_CLICKED(IDC_BUTTON_OFF, OnButtonOff)
	ON_BN_CLICKED(IDC_BUTTON_point, OnBUTTONpoint)
	ON_BN_CLICKED(IDC_BUTTON_KH1, OnButtonKh1)
	ON_BN_CLICKED(IDC_BUTTON_KH2, OnButtonKh2)
	ON_BN_CLICKED(IDC_BUTTON_back, OnBUTTONback)
	ON_BN_CLICKED(IDC_BUTTON_Pi, OnBUTTONPi)
	ON_BN_CLICKED(IDC_BUTTON_nCr, OnBUTTONnCr)
	ON_BN_CLICKED(IDC_BUTTON_nAr, OnBUTTONnAr)
	ON_BN_CLICKED(IDC_BUTTON_Up, OnBUTTONUp)
	ON_BN_CLICKED(IDC_BUTTON_Down, OnBUTTONDown)
	ON_BN_CLICKED(IDC_BUTTON_Left, OnBUTTONLeft)
	ON_BN_CLICKED(IDC_BUTTON_Right, OnBUTTONRight)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_RADIO_angle, OnRADIOangle)
	ON_BN_CLICKED(IDC_RADIO_radian, OnRADIOradian)
	ON_BN_CLICKED(IDC_RADIO_compute1, OnRADIOcompute1)
	ON_BN_CLICKED(IDC_RADIO_compute2, OnRADIOcompute2)
	ON_EN_CHANGE(IDC_EDIT_compute, OnChangeEDITcompute)
	ON_EN_SETFOCUS(IDC_EDIT_compute, OnSetfocusEDITcompute)
	ON_BN_CLICKED(IDC_RADIO_Write, OnRADIOWrite)
	ON_BN_CLICKED(IDC_RADIO_Read, OnRADIORead)
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_EN_SETFOCUS(IDC_EDIT_result, OnSetfocusEDITresult)
	ON_EN_SETFOCUS(IDC_EDIT_Time, OnSetfocusEDITTime)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDC_BUTTON_0,IDC_BUTTON_9,InputNumber)
	ON_COMMAND_RANGE(IDC_BUTTON_A,IDC_BUTTON_Z,InputWord)
	ON_COMMAND_RANGE(IDC_BUTTON_sin,IDC_BUTTON_arccot,Angle)
	ON_COMMAND_RANGE(IDC_BUTTON_memorizer1,IDC_BUTTON_memorizer6,Memory)
	ON_COMMAND_RANGE(IDC_BUTTON_Add,IDC_BUTTON_x3,Operator1)
	ON_COMMAND_RANGE(IDC_BUTTON_Ln,IDC_BUTTON_Sqrtn,Operator2)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CComputeDlg message handlers

BOOL CComputeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here
	CFont *font=new CFont;
	font->CreatePointFont(275,"");
	m_result.SetFont(font);
	font=new CFont;
	font->CreateFont(23,9,0,0,500,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH|FF_SCRIPT,_T("Times New Roman"));
	m_time.SetFont(font);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CComputeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CComputeDlg::OnPaint()
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
HCURSOR CComputeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//===============================自定义部分====================================

//=============================================================================
//                               算式的输入部分
//=============================================================================
void CComputeDlg::GetResult(const CString &s)    //获取结果
{
	if(computed&&focus)
		m_compute.SetSel(0,-1);
	if(s[0]=='-'&&m_type==0)           //若为负数非多项式则加括号且
	{
		m_compute.ReplaceSel("(");
		m_compute.ReplaceSel(s);
		m_compute.ReplaceSel(")");
	}
	else
		m_compute.ReplaceSel(s);
}

void CComputeDlg::InputNumber(UINT nID)//输入数字
{
	if(computed&&focus)
		OnBUTTONcls();
	CString number;
	number.Format("%d",nID-IDC_BUTTON_0);
	m_compute.ReplaceSel(number);
	m_compute.SetFocus();
}

void CComputeDlg::InputWord(UINT nID)  //输入字母
{
	if(m_type)
	{
		if(computed&&focus)
			OnBUTTONcls();
		CString word;
		word.Format("%c",nID-IDC_BUTTON_A+'A');
		m_compute.ReplaceSel(word);
	}
	else
		MessageBox("字母不能参与算术运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::Memory(UINT nID)     //存储器存取操作
{
	if(m_MemorizerState)
	{
		if(Memorizer[nID-IDC_BUTTON_memorizer1+1]!="\0")
		{
			GetResult(Memorizer[nID-IDC_BUTTON_memorizer1+1]);
			m_compute.SetFocus();
		}
		else
			MessageBox("该存储器是空的!","无法读取",MB_ICONASTERISK);
	}
	else
	{
		if(computed==0||Memorizer[0]=="\0")
			MessageBox("没有结果可供保存!","无法保存",MB_ICONASTERISK);
		else
			Memorizer[nID-IDC_BUTTON_memorizer1+1]=Memorizer[0];
	}
}

void CComputeDlg::Angle(UINT nID)      //插入运算符三角函数运算符
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONcls();
		switch(nID-IDC_BUTTON_sin)
		{
		case 0:m_compute.ReplaceSel("sin");break;
		case 1:m_compute.ReplaceSel("cos");break;
		case 2:m_compute.ReplaceSel("tan");break;
		case 3:m_compute.ReplaceSel("cot");break;
		case 4:m_compute.ReplaceSel("arcsin");break;
		case 5:m_compute.ReplaceSel("arccos");break;
		case 6:m_compute.ReplaceSel("arctan");break;
		case 7:m_compute.ReplaceSel("arccot");break;
		}
	}
	else
		MessageBox("多项式不可进行三角函数运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::Operator1(UINT nID)  //插入运算符“+、-、*、/、%、!、^、^2、^3”
{
	if(computed&&focus)
		OnBUTTONResult();
	switch(nID-IDC_BUTTON_Add)
		{
		case 0:m_compute.ReplaceSel("+");break;
		case 1:m_compute.ReplaceSel("-");break;
		case 2:m_compute.ReplaceSel("*");break;
		case 3:m_compute.ReplaceSel("/");break;
		case 4:m_compute.ReplaceSel("%");break;
		case 5:m_compute.ReplaceSel("!");break;
		case 6:m_compute.ReplaceSel("^");break;
		case 7:m_compute.ReplaceSel("^2");break;
		case 8:m_compute.ReplaceSel("^3");break;
		}
	m_compute.SetFocus();
}

void CComputeDlg::Operator2(UINT nID)  //插入运算符“Ln、log、ex、sqr、sqt、Sqrt”
{
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONcls();
		switch(nID-IDC_BUTTON_Ln)
		{
		case 0:m_compute.ReplaceSel("Ln");break;
		case 1:m_compute.ReplaceSel("log");break;
		case 2:m_compute.ReplaceSel("e^");break;
		case 3:m_compute.ReplaceSel("sqr");break;
		case 4:m_compute.ReplaceSel("sqt");break;
		case 5:
			m_compute.ReplaceSel("Sqrt(,)");
			OnBUTTONLeft();
			OnBUTTONLeft();
			break;
		}
	}
	else
		MessageBox("多项式不可进行对数、开方运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONPi()    //输入“π”
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONcls();
		m_compute.ReplaceSel("Pi");
	}
	else
		MessageBox("‘π’不可进行多项式运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONpoint() //输入“.”
{
	// TODO: Add your control notification handler code here
	if(computed&&focus)
		OnBUTTONcls();
	GetDlgItemText(IDC_EDIT_compute,Formula[top1]);
	int nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	if(nStart==0||Formula[top1][nStart-1]<'0'||Formula[top1][nStart-1]>'9')
		m_compute.ReplaceSel("0");
	m_compute.ReplaceSel(".");
	m_compute.SetFocus();
}

void CComputeDlg::OnButtonKh1()   //输入“(”
{
	// TODO: Add your control notification handler code here
	if(computed&&focus)
		OnBUTTONcls();
	m_compute.ReplaceSel("(");
	m_compute.SetFocus();
}

void CComputeDlg::OnButtonKh2()   //输入“)”
{
	// TODO: Add your control notification handler code here
	if(computed&&focus)
		OnBUTTONcls();
	m_compute.ReplaceSel(")");
	m_compute.SetFocus();
}


void CComputeDlg::OnBUTTONnCr()   //插入运算符“C”
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONResult();
		m_compute.ReplaceSel("C");
	}
	else
		MessageBox("多项式不可进行组合运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONnAr()   //插入运算符“A”
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONResult();
		m_compute.ReplaceSel("A");
	}
	else
		MessageBox("多项式不可进行排列运算!","非法输入",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

//=============================================================================
//                               操作部分
//=============================================================================

void CComputeDlg::OnOK()               //重载函数OnOK()
{
	OnBUTTONcompute();
}

void CComputeDlg::OnBUTTONcompute()              //进行计算
{
	// TODO: Add your control notification handler code here
	if(computed)                  //若已经计算过
		return;
	GetDlgItemText(IDC_EDIT_compute,Formula[top1]);//获取编辑框m_compute的内容
	if(Formula[top1].IsEmpty())   //若编辑框m_compute的内容为空
	{
		MessageBox("请先输入一个算式!","没有算式",MB_ICONEXCLAMATION);
		return;
	}
	if(m_type==0||!IsDXS((string)Formula[top1])) //算术运算
	{
		long double d;
		if(compute1(d,(string)Formula[top1],m_angle))
		{
			if(!double_CString(Memorizer[0],d))
			{
				MessageBox("数据的绝对值太大，已溢出!","数据溢出",MB_ICONEXCLAMATION);
				return;
			}
			m_result.SetSel(0,-1);
			m_result.ReplaceSel("=");
			m_result.ReplaceSel(Memorizer[0]);
			if(Formula[(top1+MAX-1)%MAX]!=Formula[top1])
				top2=top1=(top1+1)%MAX;
			computed=1;
			focus=1;
		}
		else
			MessageBox("算式语法错误!","输入有误",MB_ICONEXCLAMATION);
	}
	else                                         //多项式运算
	{
		C a;
		if(compute2(a,(string)Formula[top1]))
		{
			if(!a.Multinomial_CString(Memorizer[0]))
			{
				MessageBox("数据的绝对值太大，已溢出!","数据溢出",MB_ICONEXCLAMATION);
				return;
			}
			if(Memorizer[0].GetLength()>41)
				MessageBox(Formula[top1]+"="+"\n"+Memorizer[0],"多项式运算结果");
			else
			{
				m_result.SetSel(0,-1);
				m_result.ReplaceSel("=");
				m_result.ReplaceSel(Memorizer[0]);
			}
			if(Formula[(top1+MAX-1)%MAX]!=Formula[top1])
				top2=top1=(top1+1)%MAX;
			computed=1;
			focus=1;
		}
		else
			MessageBox("算式语法错误!","输入有误",MB_ICONEXCLAMATION);
	}
}

void CComputeDlg::OnBUTTONback()  //退格
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	int nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	if(nEnd==0)
		return;
	if(nStart==nEnd)
	{
		if(m_compute.LineIndex()==nEnd)
			m_compute.SetSel(nEnd-2,nEnd);
		else
			m_compute.SetSel(nEnd-1,nEnd);
	}
	m_compute.Clear();
}

void CComputeDlg::OnBUTTONcls()   //清除所有
{
	// TODO: Add your control notification handler code here
	m_compute.SetSel(0,-1);
	m_compute.Clear();
	OnChangeEDITcompute();
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONResult()//获取最近一次计算的结果
{
	// TODO: Add your control notification handler code here
	if(Memorizer[0]=="\0")
		MessageBox("没有计算结果!","尚未进行计算",MB_ICONASTERISK);
	else
	{
		GetResult(Memorizer[0]);
		m_compute.SetFocus();
	}
}

void CComputeDlg::OnBUTTONPreviousFormula() //获取上一算式
{
	// TODO: Add your control notification handler code here
	if(computed==1&&Formula[(top2+MAX-1)%MAX]!="\0")
		top2=(top2+MAX-1)%MAX;
	if(Formula[(top2+MAX-1)%MAX]=="\0"||top2==(top1+1)%MAX)
		top2=(top2+1)%MAX;
	top2=(top2+MAX-1)%MAX;
	if(Formula[top2]=="\0")
		MessageBox("存储器中没有算式","无法获取算式",MB_ICONASTERISK);
	else
	{
		m_result.SetSel(0,-1);
		m_result.Clear();
		m_compute.SetSel(0,-1);
		m_compute.ReplaceSel(Formula[top2]);
	}
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONNextFormula()//获取下一算式
{
	// TODO: Add your control notification handler code here
	if(top1==(top2+1)%MAX||top1==top2)
		top2=(top1+MAX-2)%MAX;
	top2=(top2+1)%MAX;
	if(Formula[top2]=="\0")
		MessageBox("存储器中没有算式","无法获取算式",MB_ICONASTERISK);
	else
	{
		m_result.SetSel(0,-1);
		m_result.Clear();
		m_compute.SetSel(0,-1);
		m_result.SetSel(0,-1);
		m_result.Clear();
		m_compute.ReplaceSel(Formula[top2]);
	}
	m_compute.SetFocus();
}

void CComputeDlg::OnButtonOff()   //关闭程序
{
	// TODO: Add your control notification handler code here
	int a=MessageBox("真的要退出程序吗?","退出程序",MB_YESNO|MB_ICONQUESTION);
	if(a==IDYES)
		CDialog::OnCancel();      //退出对话框，放回值为IDCANCEL
}

void CComputeDlg::OnBUTTONUp()    //插字符上移
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	if(m_compute.LineFromChar()==0)
		return;
	int Index,nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	Index=m_compute.LineIndex();
	nStart=nEnd-Index;
	m_compute.SetSel(Index-2,Index-2);
	if(m_compute.LineLength()>nStart)
	{
		Index=m_compute.LineIndex();
		Index+=nStart;
		m_compute.SetSel(Index,Index);
	}
}

void CComputeDlg::OnBUTTONDown()       //插字符下移
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	if(m_compute.LineFromChar()==m_compute.GetLineCount()-1)//若所在行为最后一行
		return;
	int Index,nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	Index=m_compute.LineIndex();
	nStart=nEnd-Index;
	Index=m_compute.LineIndex()+m_compute.LineLength()+2;//光标下移一行
	m_compute.SetSel(Index,Index);
	if(m_compute.LineLength()>nStart)
		Index+=nStart;
	else
		Index+=m_compute.LineLength();
	m_compute.SetSel(Index,Index);
}

void CComputeDlg::OnBUTTONLeft()       //插字符左移
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	int nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	if(nEnd==0)
		return;
	if(nEnd==m_compute.LineIndex())
		nEnd-=2;
	else
		nEnd-=1;
	m_compute.SetSel(nEnd,nEnd);
}

void CComputeDlg::OnBUTTONRight()      //插字符右移
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	int nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	if(nEnd==m_compute.LineIndex()+m_compute.LineLength()) //若光标位于行首
		nEnd+=2;
	else
		nEnd+=1;
	m_compute.SetSel(nEnd,nEnd);
}

void CComputeDlg::OnRADIOangle()                      //设置为角度
{
	// TODO: Add your control notification handler code here
	m_angle=0;
	computed=0;
	OnBUTTONcompute();  //重新计算
	UpdateData(false);
}

void CComputeDlg::OnRADIOradian()                     //设置为弧度
{
	// TODO: Add your control notification handler code here
	m_angle=1;
	computed=0;
	OnBUTTONcompute();  //重新计算
	UpdateData(false);
}

void CComputeDlg::OnRADIOcompute1()                   //设置为算术运算
{
	// TODO: Add your control notification handler code here
	if(m_type==0)       //若已设为算术运算
		return;
	GetDlgItem(IDC_RADIO_angle)->EnableWindow(true);  //激活角度制选择
	GetDlgItem(IDC_RADIO_radian)->EnableWindow(true);
	m_type=0;
	UpdateData(false);
}

void CComputeDlg::OnRADIOcompute2()                   //设置为多项式运算
{
	// TODO: Add your control notification handler code here
	if(m_type==1)       //若已设为多项式运算
		return;
	GetDlgItem(IDC_RADIO_angle)->EnableWindow(false); //禁用角度制选择
	GetDlgItem(IDC_RADIO_radian)->EnableWindow(false);
	m_type=1;
	UpdateData(false);
}

void CComputeDlg::OnChangeEDITcompute()     //若编辑框compute的内容改变则判断算式没被运算过
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	// TODO: Add your control notification handler code here
	computed=0;
	m_result.SetSel(0,-1);
	m_result.Clear();
}

void CComputeDlg::OnSetfocusEDITcompute()
{
	// TODO: Add your control notification handler code here
	focus=0;
}

void CComputeDlg::OnSetfocusEDITresult()
{
	// TODO: Add your control notification handler code here
	focus=1;
}

void CComputeDlg::OnSetfocusEDITTime()
{
	// TODO: Add your control notification handler code here
	focus=1;
}

void CComputeDlg::OnRADIOWrite()  //设存储器为写状态
{
	// TODO: Add your control notification handler code here
	m_MemorizerState=0;
	UpdateData(false);
}

void CComputeDlg::OnRADIORead()   //设存储器为读状态
{
	// TODO: Add your control notification handler code here
	m_MemorizerState=1;
	UpdateData(false);
}

void CComputeDlg::OnLButtonDown(UINT nFlags, CPoint point) //鼠标按下左键响应
{
	// TODO: Add your message handler code here and/or call default
	canmove=1;                    //使对话框可移动
	CDialog::OnLButtonDown(nFlags, point);
}

void CComputeDlg::OnLButtonUp(UINT nFlags, CPoint point)   //鼠标释放左键响应
{
	// TODO: Add your message handler code here and/or call default
	canmove=0;                              //使对话框不可移动
	CDialog::OnLButtonUp(nFlags, point);
}

void CComputeDlg::OnMouseMove(UINT nFlags, CPoint point)   //鼠标移动响应
{
	// TODO: Add your message handler code here and/or call default
	if(canmove==1)                          //若左键按下则移动对话框
		SendMessage(WM_SYSCOMMAND,SC_MOVE|HTCLIENT,0);
	CDialog::OnMouseMove(nFlags, point);
}

void CComputeDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CAboutDlg about;
	about.DoModal();                        //弹出关于对话框
	CDialog::OnRButtonDown(nFlags, point);
}

int CComputeDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	SetTimer(1,1000,NULL);                  //设置定时器
	return 0;
}

void CComputeDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent!=1)
		return;
	CTime ctime=CTime::GetCurrentTime();    //以下为把时间转换为所需格
	CString time;
	time.Format("%d年%02d月%02d日━",ctime.GetYear(),ctime.GetMonth(),ctime.GetDay());
	switch(ctime.GetDayOfWeek())
	{
	case 1:time+="星期日";break;
	case 2:time+="星期一";break;
	case 3:time+="星期二";break;
	case 4:time+="星期三";break;
	case 5:time+="星期四";break;
	case 6:time+="星期五";break;
	case 7:time+="星期六";break;
	}
	time+=ctime.Format(" ━ %H:%M:%S");//(CString)
	m_time.SetSel(0,-1);
	m_time.ReplaceSel(time);
	CDialog::OnTimer(nIDEvent);
}

BOOL CComputeDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)    //动态调整鼠标指针
{
	// TODO: Add your message handler code here and/or call default
	CString sClassName;
	GetClassName(pWnd->GetSafeHwnd(),sClassName.GetBuffer(80),80);
	if(sClassName!="Edit")                                                //若非编辑框
	{
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR1));
		return 1;
	}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
