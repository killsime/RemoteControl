#pragma once
#include <map>
#include <string>
#include <direct.h>
#include "Tool.h"
#include <io.h>
#include <atlimage.h>
#include "LockInfoDialog.h"
#include "Resource.h"
#include "Packet.h"
#include <list>


using namespace std;

#define SEND_DRIVER 1
#define SEND_DRICTORY 2
#define SEND_RUN_FILE 3
#define SEND_DOWNLOAD_FILE 4
#define CMD_MOUSE_EVENT 5
#define CMD_SEND_SCREEN 6
#define CMD_LOCKMACHINE 7
#define CMD_UNLOCK_MACHINE 8
#define CMD_DELETE_FILE 9


class CCommand
{
public:
	CCommand();
	~CCommand();
	int ExcuteEommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPack);
	static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPack) {
		CCommand* thiz = (CCommand*)arg;
		if (status)
		{
			int ret = thiz->ExcuteEommand(status, lstPacket, inPack);
			if (ret != 0)
			{
				TRACE("执行命令失败:%d  ret=%d\r\n", status, ret);
			}
		}
		else
		{
			MessageBox(NULL, _T("无法接入用户,自动重试"), _T(""), MB_OK | MB_ICONERROR);
		}
	}

protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>&, CPacket& inPack);//成员函数指针
	std::map<int, CMDFUNC> m_mapFunction;//从命令号到功能的映射
	CLockInfoDialog dlg;
	unsigned threadid;



	static unsigned __stdcall threadLockDlg(void* arg)
	{
		CCommand* thiz = (CCommand*)arg;
		thiz->threadLockDlgMain();
		_endthreadex(0);
		return 0;
	}

	void threadLockDlgMain()
	{
		dlg.Create(IDD_DIALOG_INFO, NULL);
		dlg.ShowWindow(SW_SHOW);
		dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rect.right *= 1.1;
		rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		rect.bottom *= 1.05;
		dlg.MoveWindow(rect);
		CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
		if (pText)
		{
			CRect rtText;
			pText->GetWindowRect(rtText);
			int nWidth = rtText.Width();
			int x = (rect.right - nWidth) / 2;
			int nHeight = rtText.Height();
			int y = (rtText.bottom - nHeight) / 2;
			pText->MoveWindow(x, y, rtText.Width(), rtText.Height());

		}
		ShowCursor(false);
		::ShowWindow(::FindWindow(_T("shell_TrayWnd"), NULL), SW_SHOW);//隐藏任务栏
		dlg.GetWindowRect(rect);
		ClipCursor(rect);//限制鼠标范围
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN)
			{
				TRACE("msg:%08X wparam:%08x lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
				if (msg.wParam == 0x1B)
				{
					break;
				}
				//break;
			}
		}
		dlg.DestroyWindow();
		ShowCursor(true);
		_endthreadex(0);
		return;
	}

	int makeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		string result;
		for (size_t i = 1; i <= 26; i++)
		{
			if (_chdrive(i) == 0)
			{
				if (result.size() >= 1)
				{
					result += ",";
				}
				result += 'a' + i - 1;
			}
		}
		lstPacket.push_back(CPacket(SEND_DRIVER, (BYTE*)result.c_str(), result.size()));
		return 0;
	}

	int makeDrictoryInfo(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		std::string strPath = inPack.strData;

		if (_chdir(strPath.c_str()) != 0)
		{
			FILEINFO finfo;
			finfo.hashNext = FALSE;
			memcpy(finfo.szFileName, strPath.c_str(), strlen(strPath.c_str()));
			lstPacket.push_back(CPacket(SEND_DRICTORY, (BYTE*)&finfo, sizeof(finfo)));
			OutputDebugString(_T("没有权限访问目录"));
			return -2;
		}
		_finddata_t fdata;
		int hfind = 0;
		if ((hfind = _findfirst("*", &fdata)) == -1)
		{
			OutputDebugString(_T("没有找到任何文件"));
			FILEINFO finfo;
			finfo.hashNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			return -3;
		}
		int count = 0;
		do
		{
			FILEINFO finfo;
			finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			count++;
		} while (!_findnext(hfind, &fdata));
		TRACE("发送总数: %d\r\n", count);
		FILEINFO finfo;
		finfo.hashNext = FALSE;
		lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		return 0;
	}

	int runFile(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		std::string strPath = inPack.strData;

		ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		lstPacket.push_back(CPacket(3, NULL, 0));

		return 0;
	}

	int downloadFile(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		std::string strPath = inPack.strData;

		FILE* pFile;
		errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
		if (err != 0)
		{
			lstPacket.push_back(CPacket(SEND_DOWNLOAD_FILE, NULL, 0));
			return -1;
		}
		if (pFile != NULL)
		{
			fseek(pFile, 0, SEEK_END);
			long long data = _ftelli64(pFile);
			CPacket head();
			fseek(pFile, 0, SEEK_SET);
			lstPacket.push_back(CPacket(SEND_DOWNLOAD_FILE, (BYTE*)&data, 8));
			char buffer[1024] = "";
			size_t rlen = 0;
			do
			{
				rlen = fread(buffer, 1, 1024, pFile);
				lstPacket.push_back(CPacket(SEND_DOWNLOAD_FILE, (BYTE*)buffer, rlen));

			} while (rlen >= 1024);
			fclose(pFile);
		}
		lstPacket.push_back(CPacket(SEND_DOWNLOAD_FILE, NULL, 0));
		return 0;
	}

	int mouseEvent(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		MOUSEEV mouse;
		memcpy(&mouse, inPack.strData.c_str(), sizeof(MOUSEEV));


		SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		DWORD nFlags = 9999;
		switch (mouse.nButton)
		{
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中键
			nFlags = 4;
			break;
		case 4:
			nFlags = 8;
			break;
		default:
			break;
		}
		if (nFlags != 8)
		{
			SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		}

		switch (mouse.nAction)
		{
		case 0:
			nFlags |= 0x10;
			break;
		case 1:
			nFlags |= 0x20;
			break;
		case 2:
			nFlags |= 0x40;
			break;
		case 3:
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse_event : %08X  x:%d   y:%d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags)
		{
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x8:
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		default:
			break;
		}
		lstPacket.push_back(CPacket(CMD_MOUSE_EVENT, NULL, 0));

		return 0;
	}

	int sendScreen(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		CImage screen;//GDI
		HDC hScreen = ::GetDC(NULL);
		int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//24 ARGB8888  32BIT RGB888  24BIT RGB565  RGB444
		int nWidth = GetDeviceCaps(hScreen, HORZRES);
		int nHeight = GetDeviceCaps(hScreen, VERTRES);
		screen.Create(nWidth, nHeight, nBitPerPixel);
		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hScreen);

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL)return -1;
		IStream* pStream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);

		if (S_OK == ret)
		{
			screen.Save(pStream, Gdiplus::ImageFormatPNG);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			BYTE* pData = (BYTE*)GlobalLock(hMem);
			size_t nSize = GlobalSize(hMem);
			lstPacket.push_back(CPacket(CMD_SEND_SCREEN, pData, nSize));

			GlobalUnlock(hMem);
		}

		pStream->Release();
		GlobalFree(hMem);
		screen.ReleaseDC();

		return 0;
	}

	int lockMachine(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE)
		{
			_beginthreadex(NULL, 0, threadLockDlg, this, 0, &threadid);
		}
		lstPacket.push_back(CPacket(CMD_LOCKMACHINE, NULL, 0));
		return 0;
	}

	int unLockMachine(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		//::SendMessage(dlg.m_hWnd,WM_KEYDOWN,0x41,0x01E0001);
		PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
		lstPacket.push_back(CPacket(CMD_LOCKMACHINE, NULL, 0));
		return 0;
	}

	int DeleteLoaclFile(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		std::string strPath = inPack.strData;

		DeleteFile((LPCWSTR)CStringW(strPath.c_str()));
		lstPacket.push_back(CPacket(7, NULL, 0));
		return 0;
	}

	int testConnect(std::list<CPacket>& lstPacket, CPacket& inPack)
	{
		lstPacket.push_back(CPacket(7, NULL, 0));
		return 0;
	}


};

