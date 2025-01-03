#pragma once
#include "pch.h"
#include "framework.h"


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
		//TRACE("���ݰ�����: %d ,���ݰ�Ч��ֵ: %x \r\n", nSize + 4,sSum);
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
		nLength = *(WORD*)(pData + i); i += 4;//ǿת byte  ��ȡ ���ݳ���
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

