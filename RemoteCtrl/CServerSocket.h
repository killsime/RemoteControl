#pragma once
#include "pch.h"
#include "framework.h"
class CServerSocket
{
public:
	CServerSocket() {
		if (FALSE == InitSocketEnv()) {
			MessageBox(NULL,_T("�޷���ʼ���׽���,������������"),_T("��ʼ������"),MB_OK|MB_ICONERROR);
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

