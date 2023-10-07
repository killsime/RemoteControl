#pragma once
#include "pch.h"
#include "framework.h"

void dump(BYTE* pData, size_t nSize);


#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:


	CPacket():sHead(0), nLength(0), sCmd(0), sSum(0){}//默认构造函数,初始化数据
	~CPacket() {}

	CPacket(const CPacket&pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}

	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		sHead = 0xFEFF;
		nLength = nSize+4;
		sCmd = nCmd;
		if (nSize >0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}

		sSum = 0;
		for (size_t i = 0; i < strData.size(); i++)
		{
			sSum += BYTE(strData[i]) & 0xFF;
		}
		//TRACE("数据包长度: %d ,数据包效验值: %x \r\n", nSize + 4,sSum);
	}
	/*
	构造函数
	用于初始化数据包
	*/
	CPacket(const BYTE * pData,size_t & nSize)//
	{
		size_t i = 0;
		for (; i < nSize; i++)//这里是循环查找包头
		{
			if (*(WORD*)pData+i  == 0xFEFF)//将传入的byte 数据转换成 word 判断是否为包头
			{
				sHead = *(WORD*)(pData + i);
				i += 2;//i+包头长度
				break;
			}
		}
		if (i+4+2+2 > nSize)//包数据补全,或者包头未全部接受 4=总长 2=命令长度 2=校验长度 
		{
			nSize = 0;
			return;
		}
		nLength = *(WORD*)(pData + i); i += 4;//强转 byte  获取 数据长度
		if (nLength+i >nSize)//包未完全接受到,判断数据包是否接收完全
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;//读取命令包
		if (nLength>4)//如果包头不是在数据头部
		{
			strData.resize(nLength - 2 - 2);//初始化strdata大小
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);//将数据复制进去
			i += nLength - 4;//i 到了校验位置
		}
		sSum = *(WORD*)(pData + i); //i到了尾部
		WORD sum = 0;
		for (size_t i = 0; i < strData.size(); i++)
		{
			sum += BYTE(strData[i]) & 0xFF;//???
		}

		if (sum == sSum)
		{
			nSize = nLength+2+4;
			return;
		}
		nSize = 0;
	}

	CPacket& operator=(const CPacket& pack)
	{
		if (this != &pack)
		{
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}

	int Size()
	{
		return nLength + 6;
	}

	const char* data()
	{
		strOut.resize(nLength+6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	WORD sHead;//包头 FEFF
	DWORD nLength;//长度
	WORD sCmd;//命令
	std::string strData;//数据
	WORD sSum;//和校验
	std::string strOut;

private:
};
#pragma pack(pop)

typedef struct MousrEvent{
	MousrEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击
	WORD nButton;//左右
	POINT ptXY;//坐标

}MOUSEEV,*PMOUSEEV;

typedef struct file_info {
	file_info() {
		isInvalid = FALSE;
		isDirectory = -1;
		hashNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}

	BOOL isInvalid;//是否是有效的
	BOOL isDirectory;//是否是目录
	char szFileName[256];//文件名
	BOOL hashNext;//判断是否有下一个文件

}FILEINFO, * PFILEINFO;

class ServiceSocket
{
public:
	static ServiceSocket * getInstance()//单例模式
	{
		if (NULL == m_instance)
		{
			m_instance = new ServiceSocket();
		}
		return m_instance;
	
	}

	bool initSocket()//初始化socket
	{
		if (m_sock == -1){	return false;}
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		
		if (-1 == bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr))) { return false; }
		if (listen(m_sock, 1) == -1) { return false; };
		return true;
	}

	bool acceptClient()//接收客户端连接
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock,(sockaddr*)&client_adr,&cli_sz);
		//TRACE("m_client = %d \r\n", m_client);
		if (-1 == m_client)
		{
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int dealCommadnd()//处理命令
	{
		if (m_client == -1)return -1;
		char * buffer = new char[BUFFER_SIZE];
		memset(buffer,0, BUFFER_SIZE);
		size_t index = 0;//创建接收的数据
		while (true)
		{
			size_t len = recv(m_client,buffer+index, BUFFER_SIZE -index,0);//给 buff 接收数据

			//TRACE("接收到的数据: %2X ", (BYTE) buffer);

			if (len <=0)
			{
				delete[] buffer;
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer,buffer+len,BUFFER_SIZE-len);
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
		return send(m_client,pData,nSize,0);
	}

	bool sendData(CPacket& pack)
	{
		if (m_client == -1)return false;
		//dump((BYTE*)pack.data(), pack.Size());
		//TRACE("发送数据长度: %d \r\n", pack.Size());
		return send(m_client, pack.data(), pack.Size(), 0);
	}

	bool getFilePath(std::string & strPath)
	{
		if (m_packet.sCmd ==2 || m_packet.sCmd == 3 || m_packet.sCmd == 4 || m_packet.sCmd == 9)
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool getMouseEvent(MOUSEEV &mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse,m_packet.strData.c_str(),sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& getPacket()
	{
		return m_packet;
	}

	void closeClient()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

private:
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;


	ServiceSocket& operator=(const ServiceSocket & ss) {}
	ServiceSocket(const ServiceSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}

	ServiceSocket() {
		m_sock = INVALID_SOCKET;
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL,_T("无法初始化套接字,请检查网络"),_T("初始化错误!"),MB_OK|MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET,SOCK_STREAM,0);
	}

	~ServiceSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	bool InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1,1),&data ) != 0)
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

