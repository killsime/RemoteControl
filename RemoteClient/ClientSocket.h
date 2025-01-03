#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>


#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:


	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}//Ĭ�Ϲ��캯��,��ʼ������
	~CPacket() {}

	CPacket(const CPacket& pack)
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
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
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
	}
	/*
	���캯��
	���ڳ�ʼ�����ݰ�
	*/
	CPacket(const BYTE* pData, size_t& nSize)//
	{
		size_t i = 0;
		for (; i < nSize; i++)//������ѭ�����Ұ�ͷ
		{
			if (*(WORD*)pData + i == 0xFEFF)//�������byte ����ת���� word �ж��Ƿ�Ϊ��ͷ
			{
				sHead = *(WORD*)(pData + i);
				i += 2;//i+��ͷ����
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)//�����ݲ�ȫ,���߰�ͷδȫ������ 4=�ܳ� 2=����� 2=У�鳤�� 
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;//ǿת byte  ��ȡ ���ݳ���
		if (nLength + i > nSize)//��δ��ȫ���ܵ�,�ж����ݰ��Ƿ������ȫ
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;//��ȡ�����
		if (nLength > 4)//�����ͷ����������ͷ��
		{
			strData.resize(nLength - 2 - 2);//��ʼ��strdata��С
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);//�����ݸ��ƽ�ȥ
			i += nLength - 4;//i ����У��λ��
		}
		sSum = *(WORD*)(pData + i); //i����β��
		WORD sum = 0;
		for (size_t i = 0; i < strData.size(); i++)
		{
			sum += BYTE(strData[i]) & 0xFF;//???
		}

		if (sum == sSum)
		{
			nSize = nLength + 2 + 4;
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
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		//TRACE("��Ϻ������: %2X \r\n", (BYTE)strOut.c_str());

		return strOut.c_str();
	}

	WORD sHead;//��ͷ FEFF
	DWORD nLength;//����
	WORD sCmd;//����
	std::string strData;//����
	WORD sSum;//��У��
	std::string strOut;

private:
};
#pragma pack(pop)

typedef struct MousrEvent {
	MousrEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//���
	WORD nButton;//����
	POINT ptXY;//����

}MOUSEEV, * PMOUSEEV;


typedef struct file_info {
	file_info() {
		isInvalid = FALSE;
		isDirectory = -1;
		hashNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}

	BOOL isInvalid;//�Ƿ�����Ч��
	BOOL isDirectory;//�Ƿ���Ŀ¼
	char szFileName[256];//�ļ���
	BOOL hashNext;//�ж��Ƿ�����һ���ļ�

}FILEINFO, * PFILEINFO;

std::string getErrInfo(int wsaErrorCode);


class ClientSocket
{
public:
	static ClientSocket* getInstance()//����ģʽ
	{
		if (NULL == m_instance)
		{
			m_instance = new ClientSocket();
		}
		return m_instance;

	}

	bool initSocket(int nIP,int nPort)//��ʼ��socket
	{
		if (m_sock != INVALID_SOCKET)closeSocket();
		m_sock = socket(PF_INET,SOCK_STREAM,0);
		if (m_sock == -1) { return false; }
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr =htonl( nIP);
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox(_T("ip��ַ������"));
			return false;
		}
		int ret = connect(m_sock,(sockaddr*)&serv_adr,sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox(_T("����ʧ��"));
			TRACE("����ʧ��: %d  %s\r\n",WSAGetLastError(),getErrInfo(WSAGetLastError()).c_str());
		}

		return true;
	}

#define BUFFER_SIZE 4096000
	int dealCommadnd()//��������
	{
		if (m_sock == -1)return -1;
		char* buffer =m_buffer.data();
		if (buffer == NULL)
		{
			TRACE("�ڴ治��! \r\n");
			return -2;
		}
		static size_t index = 0;//�������յ�����
		while (true)
		{
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);//�� buff ��������
			if ((len <= 0) && (index ==0))
			{
				//
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				//delete[] buffer;
				return m_packet.sCmd;
			}
		}

		//delete[] buffer;

	}

	bool sendData(const char* pData, int nSize)
	{
		if (m_sock == -1)return false;
		return send(m_sock, pData, nSize, 0);
	}

	bool sendData(CPacket& pack)
	{
		if (m_sock == -1)return false;
		return send(m_sock, pack.data(), pack.Size(), 0);
	}

	bool getFilePath(std::string& strPath)
	{
		if (m_packet.sCmd == 2 || m_packet.sCmd == 3 || m_packet.sCmd == 4)
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool getMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& getPacket()
	{
		return m_packet;
	}

	void closeSocket()
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

private:
	std::vector<char> m_buffer;
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;


	ClientSocket& operator=(const ClientSocket& ss) {}
	ClientSocket(const ClientSocket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}

	ClientSocket() {
		m_sock = INVALID_SOCKET;
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == FALSE)
		{
			MessageBox(NULL, _T("�޷���ʼ���׽���,��������"), _T("��ʼ������!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
	}

	~ClientSocket() {
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
	static ClientSocket* m_instance;

	static void releaseInstance()
	{
		if (m_instance != NULL)
		{
			ClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	class  CHelper
	{
	public:
		CHelper() {
			ClientSocket::getInstance();
		}
		~CHelper() {
			ClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;

};

extern ClientSocket server;

