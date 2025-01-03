
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CWatchDlg.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nport(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nport);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();
	ClientSocket* pClient = ClientSocket::getInstance();
	bool ret = pClient->initSocket(m_server_address, _ttoi(m_nport));//TODO: 处理返回信息
	if (!ret)
	{
		AfxMessageBox(_T("初始化ip失败"));
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	pClient->sendData(pack);
	//pClient->getPacket();
	pClient->dealCommadnd();
	//TRACE("ack:%d\r\n", pClient->getPacket().sCmd);
	if (bAutoClose)
	{
		pClient->closeSocket();
	}
	return pClient->getPacket().sCmd;
}



BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CRemoteClientDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_FILEINFO, &CRemoteClientDlg::OnBnClickedButtonFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_OPEN_FILE, &CRemoteClientDlg::OnOpenFile)
	ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::onSendPacket)
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_nport = _T("9527");
	m_server_address = 0x7f000001;//127.0.0.1
	UpdateData(FALSE);

	m_dlgStatus.Create(IDD_DIALOG_STATUS,this);
	m_dlgStatus.ShowWindow(SW_HIDE);

	m_isFull = false;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteClientDlg::OnBnClickedButtonTest()
{
	SendCommandPacket(1981);
}


void CRemoteClientDlg::OnBnClickedButtonFileinfo()
{
	int ret = SendCommandPacket(1);
	if (-1 == ret)
	{
		AfxMessageBox(L"命令处理失败");
	}
	ClientSocket* pClient = ClientSocket::getInstance();

	m_tree.DeleteAllItems();
	std::string drivers = pClient->getPacket().strData;
	std::string dr;
	drivers += ",";
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTemp = m_tree.InsertItem((LPCTSTR) CStringW(dr.c_str()),TVI_ROOT,TVI_LAST);
			m_tree.InsertItem(NULL,hTemp,TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}

	//dr += ":";
	//m_tree.InsertItem((LPCTSTR)CStringW(dr.c_str()), TVI_ROOT, TVI_LAST);
}

void CRemoteClientDlg::threadEntryForWatchData(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadWatchData();
	_endthread();
}

void CRemoteClientDlg::threadWatchData()
{//可能存在异步问题,导致崩溃
	ClientSocket* pClient = NULL;
	do
	{
		pClient = ClientSocket::getInstance();

	} while (pClient == NULL);

	ULONGLONG tick = GetTickCount64();
	while (!m_isClosed)//等价于  while(true)
	{
		if (m_isFull == false)
		{
			int ret = SendMessage(WM_SEND_PACKET,6<<1|1);
			if (ret>0)
			{
				std::string strData = pClient->getPacket().strData;
				BYTE * pData =(BYTE*)strData.c_str();
				//TODO:存入CImage
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,0);
				if (hMem == NULL)
				{
					TRACE("内存不足!");
					Sleep(10);
					continue;
				}
				IStream* pStream = NULL;
				HRESULT hRet = CreateStreamOnHGlobal(hMem,true,&pStream);
				if (hRet == S_OK)
				{
					ULONG length = 0;
					pStream->Write(pData,strData.size(),&length);
					LARGE_INTEGER bg = { 0 };
					pStream->Seek(bg,STREAM_SEEK_SET,NULL);
					if ((HBITMAP)*m_image !=NULL)
					{
						m_image->Destroy();
					}
					m_image->Load(pStream);
					m_isFull = true;
				}
			}
			else Sleep(10);
		}
		else Sleep(10);
	}
}

void CRemoteClientDlg::threadEntryForDownFile(void* arg)
{
	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
	thiz->threadDownFile();
	_endthread();
}

void CRemoteClientDlg::threadDownFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString cStrPath = m_List.GetItemText(nListSelected, 0);
	//设置下载地址
	CFileDialog dlg(FALSE, (LPCTSTR)L"*", m_List.GetItemText(nListSelected, 0), OFN_OVERWRITEPROMPT, (LPCTSTR)L"", this, 0, true);
	if (dlg.DoModal() == IDOK)
	{
		HTREEITEM hSelected = m_tree.GetSelectedItem();
		cStrPath = GetPath(hSelected) + cStrPath;
		TRACE("%s\r\n", cStrPath);
		USES_CONVERSION;
		std::string strPath(W2A(cStrPath));

		FILE* pFile = fopen(W2A(dlg.GetPathName()), "wb+");
		if (pFile == NULL)
		{
			AfxMessageBox(L"本地无权限,文件无法创建");
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}

		int ret = SendMessage(WM_SEND_PACKET,4 << 1|0,(LPARAM) strPath.c_str());
		if (ret < 0)
		{
			AfxMessageBox(L"执行下载命令失败!!!");
			TRACE("执行下载命令失败  %d\r\n", ret);
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		ClientSocket* pClient = ClientSocket::getInstance();
		long long nLength = *(long long*)pClient->getPacket().strData.c_str();
		if (nLength == 0)
		{
			AfxMessageBox(L"文件长度为0,或无法读取");
			pClient->closeSocket();
			m_dlgStatus.ShowWindow(SW_HIDE);
			EndWaitCursor();
			return;
		}
		long long nCount = 0;
		long long nCountOld = 0;
		while (nCount < nLength)
		{
			ret = pClient->dealCommadnd();
			if (ret < 0)
			{
				AfxMessageBox(L"传输失败!!!");
				TRACE("传输失败  %d\r\n", ret);
				pClient->closeSocket();
				m_dlgStatus.ShowWindow(SW_HIDE);
				EndWaitCursor();
				return;
			}
			fwrite(pClient->getPacket().strData.c_str(), 1, pClient->getPacket().strData.size(), pFile);
			nCount += pClient->getPacket().strData.size();
			//进度显示
			int MB = 1024 * 1024;
			if (nCount - nCountOld > MB)
			{
				CString Progress;
				Progress.Format(L"下载进度 : %lld MB / %lld MB", nCount / MB, nLength / MB);
				m_dlgStatus.m_info.SetWindowTextW(Progress);
				nCountOld = nCount;
			}
		}
		fclose(pFile);
		pClient->closeSocket();
	}

	m_dlgStatus.ShowWindow(SW_HIDE);
	EndWaitCursor();
	AfxMessageBox(L"传输完成");

	return;
}

void CRemoteClientDlg::loadFileCurrent()
{
	HTREEITEM hTreeSelected = m_tree.GetSelectedItem();
	CString strPath = GetPath(hTreeSelected);
	m_List.DeleteAllItems();
	USES_CONVERSION;
	std::string str(W2A(strPath));
	int nCmd = SendCommandPacket(2, false, (BYTE*)str.c_str(), str.length());
	PFILEINFO pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
	ClientSocket* pClient = ClientSocket::getInstance();

	while (pInfo->hashNext)
	{
		TRACE("[%s] sidir  %d \r\n", pInfo->szFileName, pInfo->isDirectory);
		if (pInfo->isDirectory)
		{
			if (CStringW(pInfo->szFileName) == L"." || CStringW(pInfo->szFileName) == L"..")
			{
				int cmd = pClient->dealCommadnd();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0) { break; }
				pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_tree.InsertItem(CStringW(pInfo->szFileName), hTreeSelected, TVI_LAST);
			m_tree.InsertItem(NULL, hTemp, TVI_LAST);
		}
		else
		{
			m_List.InsertItem(0, CStringW(pInfo->szFileName));
		}

		int cmd = pClient->dealCommadnd();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)
		{
			break;
		}
		pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
	}


	pClient->closeSocket();
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_tree.ScreenToClient(&ptMouse);

	HTREEITEM hTreeSelected = m_tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL)
		return;
	if (NULL == m_tree.GetChildItem(hTreeSelected))
		return;

	DeleteTreeChildrenItem(hTreeSelected);

	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	USES_CONVERSION;
	std::string str(W2A(strPath));
	int nCmd = SendCommandPacket(2, false, (BYTE*)str.c_str(), str.length());
	PFILEINFO pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
	ClientSocket* pClient = ClientSocket::getInstance();
	int count = 0;
	while (pInfo->hashNext)
	{
		//TRACE("[%s] sidir  %d \r\n", pInfo->szFileName, pInfo->isDirectory);
		if (pInfo->isDirectory)
		{
			if (CStringW(pInfo->szFileName) == L"." || CStringW(pInfo->szFileName) == L"..")
			{
				int cmd = pClient->dealCommadnd();
				if (cmd < 0) { break; }
				pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_tree.InsertItem(CStringW(pInfo->szFileName), hTreeSelected, TVI_LAST);
			m_tree.InsertItem(NULL, hTemp, TVI_LAST);
		}
		else
		{
			m_List.InsertItem(0,CStringW(pInfo->szFileName));
		}
		
		int cmd = pClient->dealCommadnd();
		//TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)
		{
			break;
		}
		pInfo = (PFILEINFO)ClientSocket::getInstance()->getPacket().strData.c_str();
		count++;
	}
	TRACE("接收到: %d", count);

	pClient->closeSocket();
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do
	{
		hSub = m_tree.GetChildItem(hTree);
		if (hSub != NULL)
		{
			m_tree.DeleteItem(hSub);
		}
	} while (hSub != NULL);
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do
	{
		strTmp = m_tree.GetItemText(hTree);
		strRet = strTmp + L"\\" + strRet;
		hTree = m_tree.GetParentItem(hTree);
	} while (hTree != NULL);

	return strRet;
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();

}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int listSelected = m_List.HitTest(ptList);

	if (0 == listSelected){return;}

	CMenu menu;

	menu.LoadMenuW(IDR_MENU_RCLICK);
	CMenu * pPopup = menu.GetSubMenu(0);

	if (pPopup != NULL)
	{
		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y,this);
	}

}


void CRemoteClientDlg::OnDownloadFile()
{
	// TODO: 在此添加命令处理程序代码
	_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	Sleep(50);
	BeginWaitCursor();
	m_dlgStatus.m_info.SetWindowTextW(L"命令执行中");
	m_dlgStatus.ShowWindow(SW_SHOW);
	m_dlgStatus.CenterWindow(this);
	m_dlgStatus.SetActiveWindow();

}


void CRemoteClientDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString cStrPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = cStrPath + strFile;

	USES_CONVERSION;
	std::string strPath(W2A(strFile));
	int ret = SendCommandPacket(9, true, (BYTE*)strPath.c_str(), strPath.size());
	if (ret < 0)
	{
		AfxMessageBox(L"删除文件失败!!!");
	}
	loadFileCurrent();

}


void CRemoteClientDlg::OnOpenFile()
{
	// TODO: 在此添加命令处理程序代码
	HTREEITEM hSelected = m_tree.GetSelectedItem();
	CString cStrPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected,0);
	strFile = cStrPath + strFile;

	USES_CONVERSION;
	std::string strPath(W2A(strFile));
	int ret = SendCommandPacket(3,true,(BYTE*)strPath.c_str(), strPath.size());
	if (ret < 0)
	{
		AfxMessageBox(L"打开文件失败!!!");
	}


}

LRESULT CRemoteClientDlg::onSendPacket(WPARAM wParam, LPARAM lParam)
{
	int cmd = wParam >> 1;
	int ret = 0;
	
	switch (cmd) 
	{
	case 4:
		ret = SendCommandPacket(wParam >> 1, wParam & 1, (BYTE*)std::string((LPCSTR)lParam).c_str(), std::string((LPCSTR)lParam).size());
		break;
	case 5://鼠标操作
		 ret = SendCommandPacket(cmd, wParam & 1,(BYTE *)lParam,sizeof(MOUSEEV));
		break;
	case 6:
	case 7:
	case 8:
		ret = SendCommandPacket(wParam >> 1, wParam & 1);
		break;
	default :
		ret = -1;
	}
	return ret;
}


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	m_isClosed = false;
	CWatchDlg dlg(this);
	HANDLE hThread =(HANDLE) _beginthread(CRemoteClientDlg::threadEntryForWatchData,0,this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(hThread,500);
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}
