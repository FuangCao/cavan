// computeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "compute.h"
#include "computeDlg.h"
#include "Multinomial.h"     //��������������ʽ

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

//===============================�Զ��岿��====================================

//=============================================================================
//                               ��ʽ�����벿��
//=============================================================================
void CComputeDlg::GetResult(const CString &s)    //��ȡ���
{
	if(computed&&focus)
		m_compute.SetSel(0,-1);
	if(s[0]=='-'&&m_type==0)           //��Ϊ�����Ƕ���ʽ���������
	{
		m_compute.ReplaceSel("(");
		m_compute.ReplaceSel(s);
		m_compute.ReplaceSel(")");
	}
	else
		m_compute.ReplaceSel(s);
}

void CComputeDlg::InputNumber(UINT nID)//��������
{
	if(computed&&focus)
		OnBUTTONcls();
	CString number;
	number.Format("%d",nID-IDC_BUTTON_0);
	m_compute.ReplaceSel(number);
	m_compute.SetFocus();
}

void CComputeDlg::InputWord(UINT nID)  //������ĸ
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
		MessageBox("��ĸ���ܲ�����������!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::Memory(UINT nID)     //�洢����ȡ����
{
	if(m_MemorizerState)
	{
		if(Memorizer[nID-IDC_BUTTON_memorizer1+1]!="\0")
		{
			GetResult(Memorizer[nID-IDC_BUTTON_memorizer1+1]);
			m_compute.SetFocus();
		}
		else
			MessageBox("�ô洢���ǿյ�!","�޷���ȡ",MB_ICONASTERISK);
	}
	else
	{
		if(computed==0||Memorizer[0]=="\0")
			MessageBox("û�н���ɹ�����!","�޷�����",MB_ICONASTERISK);
		else
			Memorizer[nID-IDC_BUTTON_memorizer1+1]=Memorizer[0];
	}
}

void CComputeDlg::Angle(UINT nID)      //������������Ǻ��������
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
		MessageBox("����ʽ���ɽ������Ǻ�������!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::Operator1(UINT nID)  //�����������+��-��*��/��%��!��^��^2��^3��
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

void CComputeDlg::Operator2(UINT nID)  //�����������Ln��log��ex��sqr��sqt��Sqrt��
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
		MessageBox("����ʽ���ɽ��ж�������������!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONPi()    //���롰�С�
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONcls();
		m_compute.ReplaceSel("Pi");
	}
	else
		MessageBox("���С����ɽ��ж���ʽ����!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONpoint() //���롰.��
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

void CComputeDlg::OnButtonKh1()   //���롰(��
{
	// TODO: Add your control notification handler code here
	if(computed&&focus)
		OnBUTTONcls();
	m_compute.ReplaceSel("(");
	m_compute.SetFocus();
}

void CComputeDlg::OnButtonKh2()   //���롰)��
{
	// TODO: Add your control notification handler code here
	if(computed&&focus)
		OnBUTTONcls();
	m_compute.ReplaceSel(")");
	m_compute.SetFocus();
}


void CComputeDlg::OnBUTTONnCr()   //�����������C��
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONResult();
		m_compute.ReplaceSel("C");
	}
	else
		MessageBox("����ʽ���ɽ����������!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONnAr()   //�����������A��
{
	// TODO: Add your control notification handler code here
	if(m_type==0)
	{
		if(computed&&focus)
			OnBUTTONResult();
		m_compute.ReplaceSel("A");
	}
	else
		MessageBox("����ʽ���ɽ�����������!","�Ƿ�����",MB_ICONEXCLAMATION);
	m_compute.SetFocus();
}

//=============================================================================
//                               ��������
//=============================================================================

void CComputeDlg::OnOK()               //���غ���OnOK()
{
	OnBUTTONcompute();
}

void CComputeDlg::OnBUTTONcompute()              //���м���
{
	// TODO: Add your control notification handler code here
	if(computed)                  //���Ѿ������
		return;
	GetDlgItemText(IDC_EDIT_compute,Formula[top1]);//��ȡ�༭��m_compute������
	if(Formula[top1].IsEmpty())   //���༭��m_compute������Ϊ��
	{
		MessageBox("��������һ����ʽ!","û����ʽ",MB_ICONEXCLAMATION);
		return;
	}
	if(m_type==0||!IsDXS((string)Formula[top1])) //��������
	{
		long double d;
		if(compute1(d,(string)Formula[top1],m_angle))
		{
			if(!double_CString(Memorizer[0],d))
			{
				MessageBox("���ݵľ���ֵ̫�������!","�������",MB_ICONEXCLAMATION);
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
			MessageBox("��ʽ�﷨����!","��������",MB_ICONEXCLAMATION);
	}
	else                                         //����ʽ����
	{
		C a;
		if(compute2(a,(string)Formula[top1]))
		{
			if(!a.Multinomial_CString(Memorizer[0]))
			{
				MessageBox("���ݵľ���ֵ̫�������!","�������",MB_ICONEXCLAMATION);
				return;
			}
			if(Memorizer[0].GetLength()>41)
				MessageBox(Formula[top1]+"="+"\n"+Memorizer[0],"����ʽ������");
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
			MessageBox("��ʽ�﷨����!","��������",MB_ICONEXCLAMATION);
	}
}

void CComputeDlg::OnBUTTONback()  //�˸�
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

void CComputeDlg::OnBUTTONcls()   //�������
{
	// TODO: Add your control notification handler code here
	m_compute.SetSel(0,-1);
	m_compute.Clear();
	OnChangeEDITcompute();
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONResult()//��ȡ���һ�μ���Ľ��
{
	// TODO: Add your control notification handler code here
	if(Memorizer[0]=="\0")
		MessageBox("û�м�����!","��δ���м���",MB_ICONASTERISK);
	else
	{
		GetResult(Memorizer[0]);
		m_compute.SetFocus();
	}
}

void CComputeDlg::OnBUTTONPreviousFormula() //��ȡ��һ��ʽ
{
	// TODO: Add your control notification handler code here
	if(computed==1&&Formula[(top2+MAX-1)%MAX]!="\0")
		top2=(top2+MAX-1)%MAX;
	if(Formula[(top2+MAX-1)%MAX]=="\0"||top2==(top1+1)%MAX)
		top2=(top2+1)%MAX;
	top2=(top2+MAX-1)%MAX;
	if(Formula[top2]=="\0")
		MessageBox("�洢����û����ʽ","�޷���ȡ��ʽ",MB_ICONASTERISK);
	else
	{
		m_result.SetSel(0,-1);
		m_result.Clear();
		m_compute.SetSel(0,-1);
		m_compute.ReplaceSel(Formula[top2]);
	}
	m_compute.SetFocus();
}

void CComputeDlg::OnBUTTONNextFormula()//��ȡ��һ��ʽ
{
	// TODO: Add your control notification handler code here
	if(top1==(top2+1)%MAX||top1==top2)
		top2=(top1+MAX-2)%MAX;
	top2=(top2+1)%MAX;
	if(Formula[top2]=="\0")
		MessageBox("�洢����û����ʽ","�޷���ȡ��ʽ",MB_ICONASTERISK);
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

void CComputeDlg::OnButtonOff()   //�رճ���
{
	// TODO: Add your control notification handler code here
	int a=MessageBox("���Ҫ�˳�������?","�˳�����",MB_YESNO|MB_ICONQUESTION);
	if(a==IDYES)
		CDialog::OnCancel();      //�˳��Ի��򣬷Ż�ֵΪIDCANCEL
}

void CComputeDlg::OnBUTTONUp()    //���ַ�����
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

void CComputeDlg::OnBUTTONDown()       //���ַ�����
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	if(m_compute.LineFromChar()==m_compute.GetLineCount()-1)//��������Ϊ���һ��
		return;
	int Index,nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	Index=m_compute.LineIndex();
	nStart=nEnd-Index;
	Index=m_compute.LineIndex()+m_compute.LineLength()+2;//�������һ��
	m_compute.SetSel(Index,Index);
	if(m_compute.LineLength()>nStart)
		Index+=nStart;
	else
		Index+=m_compute.LineLength();
	m_compute.SetSel(Index,Index);
}

void CComputeDlg::OnBUTTONLeft()       //���ַ�����
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

void CComputeDlg::OnBUTTONRight()      //���ַ�����
{
	// TODO: Add your control notification handler code here
	m_compute.SetFocus();
	int nStart,nEnd;
	m_compute.GetSel(nStart,nEnd);
	if(nEnd==m_compute.LineIndex()+m_compute.LineLength()) //�����λ������
		nEnd+=2;
	else
		nEnd+=1;
	m_compute.SetSel(nEnd,nEnd);
}

void CComputeDlg::OnRADIOangle()                      //����Ϊ�Ƕ�
{
	// TODO: Add your control notification handler code here
	m_angle=0;
	computed=0;
	OnBUTTONcompute();  //���¼���
	UpdateData(false);
}

void CComputeDlg::OnRADIOradian()                     //����Ϊ����
{
	// TODO: Add your control notification handler code here
	m_angle=1;
	computed=0;
	OnBUTTONcompute();  //���¼���
	UpdateData(false);
}

void CComputeDlg::OnRADIOcompute1()                   //����Ϊ��������
{
	// TODO: Add your control notification handler code here
	if(m_type==0)       //������Ϊ��������
		return;
	GetDlgItem(IDC_RADIO_angle)->EnableWindow(true);  //����Ƕ���ѡ��
	GetDlgItem(IDC_RADIO_radian)->EnableWindow(true);
	m_type=0;
	UpdateData(false);
}

void CComputeDlg::OnRADIOcompute2()                   //����Ϊ����ʽ����
{
	// TODO: Add your control notification handler code here
	if(m_type==1)       //������Ϊ����ʽ����
		return;
	GetDlgItem(IDC_RADIO_angle)->EnableWindow(false); //���ýǶ���ѡ��
	GetDlgItem(IDC_RADIO_radian)->EnableWindow(false);
	m_type=1;
	UpdateData(false);
}

void CComputeDlg::OnChangeEDITcompute()     //���༭��compute�����ݸı����ж���ʽû�������
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

void CComputeDlg::OnRADIOWrite()  //��洢��Ϊд״̬
{
	// TODO: Add your control notification handler code here
	m_MemorizerState=0;
	UpdateData(false);
}

void CComputeDlg::OnRADIORead()   //��洢��Ϊ��״̬
{
	// TODO: Add your control notification handler code here
	m_MemorizerState=1;
	UpdateData(false);
}

void CComputeDlg::OnLButtonDown(UINT nFlags, CPoint point) //��갴�������Ӧ
{
	// TODO: Add your message handler code here and/or call default
	canmove=1;                    //ʹ�Ի�����ƶ�
	CDialog::OnLButtonDown(nFlags, point);
}

void CComputeDlg::OnLButtonUp(UINT nFlags, CPoint point)   //����ͷ������Ӧ
{
	// TODO: Add your message handler code here and/or call default
	canmove=0;                              //ʹ�Ի��򲻿��ƶ�
	CDialog::OnLButtonUp(nFlags, point);
}

void CComputeDlg::OnMouseMove(UINT nFlags, CPoint point)   //����ƶ���Ӧ
{
	// TODO: Add your message handler code here and/or call default
	if(canmove==1)                          //������������ƶ��Ի���
		SendMessage(WM_SYSCOMMAND,SC_MOVE|HTCLIENT,0);
	CDialog::OnMouseMove(nFlags, point);
}

void CComputeDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CAboutDlg about;
	about.DoModal();                        //�������ڶԻ���
	CDialog::OnRButtonDown(nFlags, point);
}

int CComputeDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	SetTimer(1,1000,NULL);                  //���ö�ʱ��
	return 0;
}

void CComputeDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent!=1)
		return;
	CTime ctime=CTime::GetCurrentTime();    //����Ϊ��ʱ��ת��Ϊ�����
	CString time;
	time.Format("%d��%02d��%02d�թ�",ctime.GetYear(),ctime.GetMonth(),ctime.GetDay());
	switch(ctime.GetDayOfWeek())
	{
	case 1:time+="������";break;
	case 2:time+="����һ";break;
	case 3:time+="���ڶ�";break;
	case 4:time+="������";break;
	case 5:time+="������";break;
	case 6:time+="������";break;
	case 7:time+="������";break;
	}
	time+=ctime.Format(" �� %H:%M:%S");//(CString)
	m_time.SetSel(0,-1);
	m_time.ReplaceSel(time);
	CDialog::OnTimer(nIDEvent);
}

BOOL CComputeDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)    //��̬�������ָ��
{
	// TODO: Add your message handler code here and/or call default
	CString sClassName;
	GetClassName(pWnd->GetSafeHwnd(),sClassName.GetBuffer(80),80);
	if(sClassName!="Edit")                                                //���Ǳ༭��
	{
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR1));
		return 1;
	}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
