#include "pch.h"
#include "Command.h"

CCommand::CCommand() :threadid(0)
{
	struct
	{
		int nCmd;
		CMDFUNC func;
	}data[] = {
		{1,&CCommand::makeDriverInfo},
		{2,&CCommand::makeDrictoryInfo},
		{3,&CCommand::runFile},
		{4,&CCommand::downloadFile},
		{5,&CCommand::mouseEvent},
		{6,&CCommand::sendScreen},
		{7,&CCommand::lockMachine},
		{8,&CCommand::unLockMachine},
		{9,&CCommand::DeleteLoaclFile},
		{1981,&CCommand::testConnect},
		{-1,NULL},
	};

	for (size_t i = 0; data[i].nCmd != -1; i++)
	{
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}

}

CCommand::~CCommand()
{
}

int CCommand::ExcuteEommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPack)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);

	if (it == m_mapFunction.end())
	{
		return -1;
	}

	return (this->*it->second)(lstPacket, inPack);
}
