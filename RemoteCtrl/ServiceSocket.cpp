#include "pch.h"
#include "ServiceSocket.h"

ServiceSocket* ServiceSocket::m_instance = NULL;
ServiceSocket::CHelper ServiceSocket::m_helper;

ServiceSocket* pService = ServiceSocket::getInstance();