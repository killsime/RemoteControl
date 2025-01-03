#pragma once
#include "pch.h"
#include "framework.h"


#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:


	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}//默认构造函数,初始化数据
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
		//TRACE("数据包长度: %d ,数据包效验值: %x \r\n", nSize + 4,sSum);
	}
	/*
	构造函数
	用于初始化数据包
	*/
	CPacket(const BYTE* pData, size_t& nSize)//
	{
		size_t i = 0;
		for (; i < nSize; i++)//这里是循环查找包头
		{
			if (*(WORD*)pData + i == 0xFEFF)//将传入的byte 数据转换成 word 判断是否为包头
			{
				sHead = *(WORD*)(pData + i);
				i += 2;//i+包头长度
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)//包数据补全,或者包头未全部接受 4=总长 2=命令长度 2=校验长度 
		{
			nSize = 0;
			return;
		}
		nLength = *(WORD*)(pData + i); i += 4;//强转 byte  获取 数据长度
		if (nLength + i > nSize)//包未完全接受到,判断数据包是否接收完全
		{
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;//读取命令包
		if (nLength > 4)//如果包头不是在数据头部
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

	WORD sHead;//包头 FEFF
	DWORD nLength;//长度
	WORD sCmd;//命令
	std::string strData;//数据
	WORD sSum;//和校验
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
	WORD nAction;//点击
	WORD nButton;//左右
	POINT ptXY;//坐标

}MOUSEEV, * PMOUSEEV;

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

