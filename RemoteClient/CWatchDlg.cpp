// CWatchDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDlg.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// CWatchDlg 对话框

IMPLEMENT_DYNAMIC(CWatchDlg, CDialog)

CWatchDlg::CWatchDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

}

CWatchDlg::~CWatchDlg()
{
}

void CWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDlg::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDlg::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDlg::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDlg 消息处理程序


CPoint CWatchDlg::UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint cur = point;
		CRect clientRect;
		if (isScreen)ScreenToClient(&point);//全局客户区域

		m_picture.GetWindowRect(clientRect);
		float x = m_nObjWidth / clientRect.Width();
		return CPoint(point.x * m_nObjWidth / clientRect.Width(), point.y * m_nObjHeight / clientRect.Height());
	}
}

BOOL CWatchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0,45,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nIDEvent ==0 )
	{
		CRemoteClientDlg* pParment = (CRemoteClientDlg *)GetParent();
		if (pParment->m_isFull)
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			if (m_nObjWidth == -1)m_nObjWidth=pParment->m_image->GetWidth();
			if (m_nObjHeight == -1)m_nObjHeight =pParment->m_image->GetHeight();
			pParment->m_image->StretchBlt(m_picture.GetDC()->GetSafeHdc(),0,0,rect.Width(),rect.Height(),SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParment->m_image->Destroy();
			pParment->m_isFull = false;
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//左键
		even.nAction = 1;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//左键
		even.nAction = 2;//按下
		TRACE(" 1 ---- x=%d  y=%d \r\n", point.x, point.y);

		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//左键
		even.nAction = 3;//弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 1;//右键
		even.nAction = 1;//双击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//右键
		even.nAction = 2;//按下//TODO:服务端做对应修改
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//右键
		even.nAction = 3;//弹起
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//if (m_nObjWidth != -1 && m_nObjHeight != -1)
	//{
	//	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//	MOUSEEV even;
	//	even.ptXY = remote;
	//	even.nButton = 8;//没有按键
	//	even.nAction = 0;//移动
	//	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	//	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	//}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDlg::OnStnClickedWatch()
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint point;
		GetCursorPos(&point);
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		MOUSEEV even;
		even.ptXY = remote;
		even.nButton = 0;//左键
		even.nAction = 0;//单击
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&even);
	}
}


void CWatchDlg::OnBnClickedBtnLock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDlg::OnBnClickedBtnUnlock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
