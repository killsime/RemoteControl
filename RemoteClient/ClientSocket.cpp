#include "pch.h"
#include "ClientSocket.h"

ClientSocket* ClientSocket::m_instance = NULL;
ClientSocket::CHelper ClientSocket::m_helper;

ClientSocket* pService = ClientSocket::getInstance();

std::string getErrInfo(int wsaErrorCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}