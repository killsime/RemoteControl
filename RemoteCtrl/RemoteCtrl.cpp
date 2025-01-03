// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "ServiceSocket.h"

#include "LockInfoDialog.h"
CLockInfoDialog dlg;

#include"Tool.h"
#include"Command.h"
using namespace std;


// 唯一的应用程序对象

CWinApp theApp;


int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            CCommand cmd;
            ServiceSocket* pserver = ServiceSocket::getInstance();
            int ret = pserver->run(&CCommand::RunCommand, &cmd);
            switch (ret)
            {
            case -1:
				MessageBox(NULL, _T("网络初始化异常异常"), _T(""), MB_OK | MB_ICONERROR);
                exit(0);
                break;
			case -2:
				MessageBox(NULL, _T("多次无法接入,结束程序"), _T(""), MB_OK | MB_ICONERROR);
				exit(0);
				break;
            default:
                break;
            }
        }
    }
    else
    {
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
