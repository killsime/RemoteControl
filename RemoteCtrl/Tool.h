#pragma once
#include <string>
using namespace std;
class CTool
{
public:
    /*
��־��ӡ
*/
    static void dump(BYTE* pData, size_t nSize)
    {
        string strOut;
        strOut += "Dump : ";
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))
            {
                strOut += "\n";
            }
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }

    static bool IsAdmin() {//����ԱȨ���ж�
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))//GetCurrentProcess��ȡ��ǰ�߳̾��  OpenProcessToken�����Ʒ�������������
        {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {//��ȡ����token��Ϣ
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {
            return eve.TokenIsElevated;
        }
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }

    static bool Init()
    {//���ڴ�mfc��������Ŀ��ʼ����ͨ�ã�
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr) {
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }


    static BOOL WriteStartupDir(const CString& strPath)//д�ļ�������Ŀ¼
    {//ͨ���޸Ŀ��������ļ�����ʵ�ֿ�������
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return CopyFile(sPath, strPath, FALSE);
        //fopen CFile system(copy) CopyFile OpenFile 
    }

    static bool WriteRegisterTable(const CString& strPath)
    {//ͨ���޸�ע�����ʵ�ֿ�������
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CopyFile(sPath, strPath, FALSE);
        //fopen CFile system(copy) CopyFile OpenFile 
        if (ret == FALSE) {
            MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    static bool RunAsAdmin()
    {//TODO:��ȡ����ԱȨ�ޡ�ʹ�ø�Ȩ�޴�������
        //���ز����� ����Administrator�˻�  ��ֹ������ֻ�ܵ�¼���ؿ���̨
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);//�������ص�½�߳�
        if (!ret) {
            ShowError();//TODO:ȥ��������Ϣ
            MessageBox(NULL, sPath, _T("��������ʧ��"), 0);//TODO:ȥ��������Ϣ
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static void ShowError()
    {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);//��׼C������
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        MessageBox(NULL, lpMessageBuf, _T("��������"), 0);
        LocalFree(lpMessageBuf);
    }
};

