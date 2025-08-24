#include "SCManager.h"

bool StopWindowsService(const wchar_t* serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(
        NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCManager == NULL) {
        MessageBoxA(NULL, "�򿪷��������ʧ��", "����", MB_OK | MB_SYSTEMMODAL);
        return false;
    }

    SC_HANDLE hService = OpenService(
        hSCManager, serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS);

    if (hService == NULL) {
        MessageBoxA(NULL, "�򿪷���ʧ��", "����", MB_OK | MB_SYSTEMMODAL);
        CloseServiceHandle(hSCManager);
        return false;
    }

    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwBytesNeeded;

    if (!ControlService(
        hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssStatus)) {

        MessageBoxA(NULL, "�������ʧ��", "����", MB_OK | MB_SYSTEMMODAL);
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    while (ssStatus.dwCurrentState != SERVICE_STOPPED) {
        Sleep(ssStatus.dwWaitHint);
        if (!QueryServiceStatusEx(
            hService, SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {

            std::cerr << "\n���·���״̬ʧ�ܣ�������룺" << GetLastError() << std::endl;
            break;
        }

        if (ssStatus.dwCurrentState == SERVICE_STOPPED)
            break;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return ssStatus.dwCurrentState == SERVICE_STOPPED;
}