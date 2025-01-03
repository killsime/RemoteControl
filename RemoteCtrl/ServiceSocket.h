#pragma once
#include "pch.h"
#include "framework.h"
#include <list>
#include "Packet.h"
//void dump(BYTE* pData, size_t nSize);



typedef void(*SOCKET_CALLBACK)(void* arg, int status, std::list<CPacket>& lstPackets, CPacket& p);


class ServiceSocket
{
public:
	static ServiceSocket* getInstance()//单例模式
	{
		if (NULL == m_instance)
		{
			m_instance = new ServiceSocket();
		}
		return m_instance;

	}

	bool initSocket(short port)//初始化socket
	{
		if (m_sock == -1) { return false; }
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);

		if (-1 == bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr))) { return false; }
		if (listen(m_sock, 1) == -1) { return false; };

		return true;
	}


	int run(SOCKET_CALLBACK callback, void* arg, short port = 9527)
	{
		bool ret = initSocket(port);
		if (ret == false) { return -1; }
		std::list<CPacket> listPackets;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true)
		{
			if (acceptClient() == false)
			{
				if (count >= 3)
				{
					return -2;
				}
			}
			int ret = dealCommand();
			if (ret > 0)
			{
				m_callback(m_arg, ret, listPackets, m_packet);
				int sendCout = 0;
				while (listPackets.size() > 0)
				{
					int r = sendData(listPackets.front());
					listPackets.pop_front();
					sendCout++;
					Sleep(1);
				}
				TRACE("发送数据包: %d\r\n", sendCout);
			}
			closeClient();

		}


		return 0;
	}

	bool acceptClient()//接收客户端连接
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		//TRACE("m_client = %d \r\n", m_client);
		if (-1 == m_client)
		{
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int dealCommand()//处理命令
	{
		if (m_client == -1)return -1;
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;//创建接收的数据
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);//给 buff 接收数据

			//TRACE("接收到的数据: %2X ", (BYTE) buffer);

			if (len <= 0)
			{
				delete[] buffer;
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				//TRACE("dealCommadnd: 接收的命令 %d", m_packet.sCmd);
				delete[] buffer;
				return m_packet.sCmd;
			}
		}
		delete[] buffer;
	}



	bool sendData(const char* pData, int nSize)
	{
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0);
	}

	bool sendData(CPacket& pack)
	{
		if (m_client == -1)return false;
		//dump((BYTE*)pack.data(), pack.Size());
		//TRACE("发送数据长度: %d \r\n", pack.Size());
		return send(m_client, pack.data(), pack.Size(), 0);
	}


	void closeClient()
	{
		if (m_client != INVALID_SOCKET)
		{
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}

private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;



	ServiceSocket& operator=(const ServiceSocket& ss) {}
	ServiceSocket(const ServiceSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}

	ServiceSocket() {
		m_sock = INVALID_SOCKET;
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("无法初始化套接字,请检查网络"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}

	~ServiceSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	bool InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	static ServiceSocket* m_instance;

	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			ServiceSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class  CHelper
	{
	public:
		CHelper() {
			ServiceSocket::getInstance();
		}
		~CHelper() {
			ServiceSocket::releaseInstance();
		}
	};
	static CHelper m_helper;

};

extern ServiceSocket server;

