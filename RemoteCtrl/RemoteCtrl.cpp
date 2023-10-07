// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "ServiceSocket.h"
#include <io.h>
#include <list>
#include<atlimage.h>

#include "LockInfoDialog.h"
CLockInfoDialog dlg;

using namespace std;


// 唯一的应用程序对象

CWinApp theApp;

#define SEND_DRIVER 1
#define SEND_DRICTORY 2
#define SEND_RUN_FILE 3
#define SEND_DOWNLOAD_FILE 4
#define CMD_MOUSE_EVENT 5
#define CMD_SEND_SCREEN 6
#define CMD_LOCKMACHINE 7
#define CMD_UNLOCK_MACHINE 8
#define CMD_DELETE_FILE 9


/*
日志打印
*/
void dump(BYTE* pData,size_t nSize)
{
    string strOut;
    for (size_t i = 0; i < nSize; i++)
    {
        char buf[8] = "";
        if (i>0 && (i%16 ==0))
        {
            strOut += "\n";
        }
        snprintf(buf,sizeof(buf),"%02X ",pData[i]&0xFF);
        strOut += buf;
    }
    OutputDebugStringA(strOut.c_str());
}

int makeDriverInfo()
{
    string result;
    for (size_t i = 1; i <=26; i++)
    {
        if (_chdrive(i) == 0)
        {
            if (result.size()>=1)
            {
                result += ",";
            }
            result += 'a' + i - 1;
        }
    }
    CPacket pack(SEND_DRIVER, (BYTE*)result.c_str(), result.size());
    dump((BYTE*)pack.data(), pack.Size());
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}



int makeDrictoryInfo()
{
    std::string strPath;
    //std::list<FILEINFO> lsFileinfos;
    if (false == ServiceSocket::getInstance()->getFilePath(strPath))
    {
        OutputDebugString(_T("当前命令,不是获取文件列表的,命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str())!=0)
    {
        FILEINFO finfo;
       // finfo.isInvalid = TRUE;
        //finfo.isDirectory = TRUE;
        finfo.hashNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strlen(strPath.c_str()));
        //lsFileinfos.push_back(finfo);
        CPacket pack(SEND_DRICTORY,(BYTE*)&finfo,sizeof(finfo));
        ServiceSocket::getInstance()->sendData(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ( (hfind= _findfirst("*", &fdata)) == -1)
    {
        OutputDebugString(_T("没有找到任何文件"));
        FILEINFO finfo;
        finfo.hashNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        ServiceSocket::getInstance()->sendData(pack);
        return -3;
    }
    int count = 0;
    do
    {
        FILEINFO finfo;
        finfo.isDirectory = (fdata.attrib & _A_SUBDIR) !=0;
        memcpy(finfo.szFileName,fdata.name,strlen(fdata.name));
        //lsFileinfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        ServiceSocket::getInstance()->sendData(pack);
        count++;
    } while (!_findnext(hfind,&fdata));
    TRACE("发送总数: %d",count);
    FILEINFO finfo;
    finfo.hashNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int runFile()
{
    std::string strPath;
    //std::list<FILEINFO> lsFileinfos;
    ServiceSocket::getInstance()->getFilePath(strPath);
    ShellExecuteA(NULL,NULL,strPath.c_str(),NULL,NULL,SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int downloadFile()
{
    std::string strPath;
    ServiceSocket::getInstance()->getFilePath(strPath);
    FILE* pFile;
    errno_t err = fopen_s(&pFile,strPath.c_str(), "rb");
    if (err != 0)
    {
        CPacket pack(SEND_DOWNLOAD_FILE, NULL, 0);
        ServiceSocket::getInstance()->sendData(pack);
        return -1;
    }
    if (pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        long long data = _ftelli64(pFile);
        CPacket head(SEND_DOWNLOAD_FILE, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);

        ServiceSocket::getInstance()->sendData(head);//这里教程没有,不过我觉得有必要加上

        char buffer[1024] = "";
        size_t rlen = 0;
        do
        {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(SEND_DOWNLOAD_FILE, (BYTE*)buffer, rlen);
            ServiceSocket::getInstance()->sendData(pack);

        } while (rlen >= 1024);
        fclose(pFile);
    }

    CPacket pack(SEND_DOWNLOAD_FILE, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int mouseEvent()
{
    MOUSEEV mouse;
    if (ServiceSocket::getInstance()->getMouseEvent(mouse))
    {
        SetCursorPos(mouse.ptXY.x,mouse.ptXY.y);
        DWORD nFlags=9999;
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
        if (nFlags!=8)
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
        TRACE("mouse_event : %08X  x:%d   y:%d\r\n",nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags)
        {
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,GetMessageExtraInfo());
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
        case 0x14://中建单击
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
        CPacket pack(CMD_MOUSE_EVENT, NULL, 0);
        ServiceSocket::getInstance()->sendData(pack);
    }
    else
    {
        OutputDebugString(_T("获取鼠标参数失败"));
        return -1;
    }

}

int sendScreen()
{
    CImage screen;//GDI
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen,BITSPIXEL);//24 ARGB8888  32BIT RGB888  24BIT RGB565  RGB444
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth,nHeight,nBitPerPixel);
    BitBlt(screen.GetDC(),0,0, nWidth, nHeight,hScreen,0,0,SRCCOPY);
    ReleaseDC(NULL,hScreen);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL)return -1;
    IStream* pStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem,TRUE,&pStream);
    
    if (S_OK ==  ret)
    {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        BYTE * pData = (BYTE *) GlobalLock(hMem);
        size_t nSize = GlobalSize(hMem);
        CPacket pack(CMD_SEND_SCREEN, pData, nSize);
        ServiceSocket::getInstance()->sendData(pack);
        GlobalUnlock(hMem);
    }
    
    // GetTickCount64(); 获取开机时间
    //screen.Save(_T("test2021.png"),Gdiplus::ImageFormatPNG);
    //screen.Save(_T("test2021.jpg"),Gdiplus::ImagerFormatPNG);
    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();

    return 0;
}
unsigned threadid = 0;
unsigned __stdcall  threadLockDlg(void* arg)
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
        pText->MoveWindow(x,y,rtText.Width(),rtText.Height());
             
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
    return NULL;
}

int lockMachine()
{
    if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE)
    {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL,0,threadLockDlg,NULL,0,&threadid);
    }
    CPacket pack(CMD_LOCKMACHINE, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int unLockMachine()
{
    //::SendMessage(dlg.m_hWnd,WM_KEYDOWN,0x41,0x01E0001);
    PostThreadMessage(threadid,WM_KEYDOWN,0x1B,0);
    CPacket pack(CMD_LOCKMACHINE, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int DeleteLoaclFile()
{
    string strPath;
    ServiceSocket::getInstance()->getFilePath(strPath);
    DeleteFile((LPCWSTR)CStringW(strPath.c_str()));
    CPacket pack(7, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int testConnect()
{
    CPacket pack(7, NULL, 0);
    ServiceSocket::getInstance()->sendData(pack);
    return 0;
}

int excuteEommand(int nCmd)
{
    int ret = 0;
    switch (nCmd)
    {
    case SEND_DRIVER:
        ret = makeDriverInfo();
        break;
    case SEND_DRICTORY:
        ret = makeDrictoryInfo();
        break;
    case SEND_RUN_FILE: //打开文件
        ret = runFile();
    case SEND_DOWNLOAD_FILE: //下载文件
        ret = downloadFile();
        break;
    case CMD_MOUSE_EVENT:
        ret = mouseEvent();
        break;
    case CMD_SEND_SCREEN://发送屏幕内容 ==> 发送屏幕截图
        ret = sendScreen();
        break;
    case CMD_LOCKMACHINE:
        ret = lockMachine();
        break;
    case CMD_UNLOCK_MACHINE:
        ret = unLockMachine();
        break;
    case CMD_DELETE_FILE:
        ret = DeleteLoaclFile();
        break;
    case 1981:
        ret = testConnect();
    default:
        break;
    }

    return ret;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            ServiceSocket* pserver = ServiceSocket::getInstance();
            int count = 0;
            if (pserver->initSocket() == false)
            {
                MessageBox(NULL,_T("网络初始化异常异常"),_T(""),MB_OK|MB_ICONERROR);
            }
            while (ServiceSocket::getInstance() != NULL)
            {
                if (pserver->acceptClient() ==false)
                {
                    if (count >= 3)
                    {
                        MessageBox(NULL, _T("多次无法接入,结束程序"), _T(""), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法接入用户,自动重试"), _T(""), MB_OK | MB_ICONERROR);
                    count++;
                }

                int ret = pserver->dealCommadnd();
               // TRACE("dealCommadnd 解析命令返回值 %d \r\n", ret);

                if (0 < ret)
                {
                    ret = excuteEommand(pserver->getPacket().sCmd);
                    if (0 != ret)
                    {
                        TRACE("执行命令失败:%d  ret=%d\r\n",pserver->getPacket().sCmd,ret);
                    }
                    pserver->closeClient();
                }
                
            }

        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
