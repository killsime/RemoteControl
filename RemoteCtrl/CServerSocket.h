#pragma once
#include "pch.h"
#include "framework.h"
class CServerSocket
{
public:
	CServerSocket() {
		if (FALSE == InitSocketEnv()) {
			MessageBox(NULL,_T("无法初始化套接字,请检查网络设置"),_T("初始化错误"),MB_OK|MB_ICONERROR);
		}
	};
	~CServerSocket() {
		WSACleanup();
	};

	bool InitSocketEnv() {
		WSADATA data;
		if (0 != WSAStartup(MAKEWORD(1, 1), &data)) {
			return FALSE;
		}
		return TRUE;
	}
};

