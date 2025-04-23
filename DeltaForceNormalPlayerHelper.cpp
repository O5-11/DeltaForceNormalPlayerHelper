
#include "c:\Head\Head.h"
#include "shellapi.h"
#include "vector"
#include <comdef.h>
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>
#include <audiopolicy.h>

#include <atlbase.h>

#pragma comment(lib,"winmm.lib")

using namespace std;

#define EXENAMELEN 100

bool Exit = false;

void StopServiceSimple(const char* serviceName) {
    std::string cmd = "net stop \"" + std::string(serviceName) + "\"";
    system(cmd.c_str());
}

void StopService(const wchar_t* serviceName) {
    std::wstring cmd = L"net stop \"" + std::wstring(serviceName) + L"\"";
    _wsystem(cmd.c_str());
}

class SwitchPanle {

public:
    void drawCloseButton()
    {
        circle(90, 80, 30);
        circle(270, 80, 30);

        line(90, 50, 270, 50);
        line(90, 110, 270, 110);

        setfillcolor(0x8080F0);
        solidrectangle(90, 50, 270, 110);
        solidcircle(270, 80, 30);

        setfillcolor(WHITE);
        solidcircle(90, 80, 30);
        setlinecolor(BLACK);
        circle(90, 80, 31);
    };

    void drawOpenButton()
    {
        circle(90, 80, 30);
        circle(270, 80, 30);
        line(90, 50, 270, 50);
        line(90, 110, 270, 110);

        setfillcolor(0xBFFF00);
        solidrectangle(90, 50, 270, 110);
        solidcircle(90, 80, 30);

        setfillcolor(WHITE);
        solidcircle(270, 80, 30);
        setlinecolor(BLACK);
        circle(270, 80, 31);
    };

    void ButtonMove(int per)
    {
        BeginBatchDraw();

        setfillcolor(0xBFFF00);
        solidrectangle(90, 50, 90 + per * 180 / 100, 110);
        solidcircle(90, 80, 30);
        setfillcolor(0x8080F0);
        solidrectangle(90 + per * 180 / 100, 50, 270, 110);
        solidcircle(270, 80, 30);

        setfillcolor(WHITE);
        solidcircle(90 + per * 180 / 100, 80, 30);
        setlinecolor(BLACK);
        circle(90 + per * 180 / 100, 80, 31);

        EndBatchDraw();
    }

    void SwitchButton(bool Open)
    {
        if (Open == false)
        {
            for (int i = 100; i >= 0; i--)
            {
                ButtonMove(i);
                if (i % 20 == 0)Sleep(1);
            }
        }
        else
        {
            for (int i = 1; i <= 100; i++)
            {
                ButtonMove(i);
                if (i % 20 == 0)Sleep(1);
            }
        }
    };

    bool ForgedTargetWindow(string TargetWindowName)
    {
        HWND hwnd = GetForegroundWindow();
        char* name = NewStr(256);
        GetWindowTextA(hwnd, name, 256);

        if (strcmp(name, TargetWindowName.c_str()) == 0)
        {
            delete[]name;
            return true;
        }
        else
        {
            delete[]name;
            return false;
        }
    };
};

typedef struct Data
{
    char Name[EXENAMELEN];
    char* ExePath;
    ULONG pID;
    bool FindState;
    ISimpleAudioVolume* Controler;
    HBITMAP hBitmap;
}Data;

vector<Data>List;

HMENU hMenu = NULL;
HMENU childhMenu = NULL;
HMENU settinghMenu = NULL;
POINT pt;
bool SINGLE = false;
bool ReleaseAllWindow = true;
bool KeepOriginVolume = true;
HANDLE hMutex = NULL;
string StaticFilePath;

SimpleThread ST;
HANDLE hThreads[3] = { NULL,NULL,NULL };
mutex MTX;

class SingleVolume
{
private:
    bool Initialized = false;

    HRESULT hr = S_OK;
    IMMDeviceCollection* pMultiDevice = nullptr;
    IMMDevice* pDevice = nullptr;
    IMMDeviceEnumerator* m_pEnumerator = nullptr;
    IAudioSessionEnumerator* pSessionEnum = nullptr;
    IAudioSessionManager2* pASManager = nullptr;
    IAudioSessionControl* pASControl = nullptr;
    IAudioSessionControl2* pASControl2 = nullptr;

    IPropertyStore* pPropertyStore = nullptr;
    PROPVARIANT pv;

    const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
    const IID IID_IAudioSessionControl2 = __uuidof(IAudioSessionControl2);
    GUID m_guidMyContext;
    ISimpleAudioVolume* pSimplevol = nullptr;

    wchar_t* wszDeviceName;

public:

    bool Init()
    {
        //环境
        CoInitialize(nullptr);

        wszDeviceName = new wchar_t[MAX_PATH]; memset(wszDeviceName, 0, MAX_PATH * sizeof(wchar_t));
        PropVariantInit(&pv);

        return TRUE;
    }

    SingleVolume()
    {
        Initialized = Init();
    };

    ~SingleVolume()
    {
        if (wszDeviceName != NULL)
            delete[]wszDeviceName;

        if (m_pEnumerator != nullptr)
            m_pEnumerator->Release();

        if (pPropertyStore != nullptr)
            pPropertyStore->Release();

        PropVariantClear(&pv);

        CoUninitialize();
    };

    bool GetDeviceName()
    {
        if (!Initialized)
        {
            MBX("环境初始化失败");
            return false;
        }

        //获取当前激活设备名
        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        try {
            HRESULT hrest = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
            if (SUCCEEDED(hrest)) {
                hrest = pPropertyStore->GetValue(PKEY_DeviceInterface_FriendlyName, &pv);
                if (SUCCEEDED(hrest) && VT_EMPTY != pv.vt) {
                    wcscat(wszDeviceName, pv.pwszVal);
                }
            }
        }
        catch (std::exception&) {
        }

        m_pEnumerator->Release();
    };

    void GetpIDList()
    {
        if (!Initialized)
        {
            MBX("环境初始化失败");
            return;
        }

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMultiDevice);
        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pASManager);
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);
        pASManager->GetAudioSessionControl(nullptr, 0, &pASControl);

        int num = 0;
        TCHAR* processName = new TCHAR[MAX_PATH]; memset(processName, 0, sizeof(TCHAR) * MAX_PATH);

        pSessionEnum->GetCount(&num);

        for (int i = 0; i < num; i++)
        {
            pSessionEnum->GetSession(i, &pASControl);
            hr = pASControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pASControl2);

            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                pASControl2->GetProcessId(&processId);

                if (processId != 0)
                {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                    if (hProcess) {
                        if (GetModuleBaseName(hProcess, NULL, processName, MAX_PATH)) {

                            char* processCharName = NULL;
                            processCharName = LPWSTRTopChar(processName);

                            for (int i = 0; i < List.size(); i++)
                            {
                                if (strcmp(List[i].Name, processCharName) == 0)
                                {
                                    hr = pASControl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&pSimplevol);
                                    if (FAILED(hr))
                                    {
                                        delete[]processCharName;
                                        continue;
                                    }

                                    List[i].FindState = true;
                                    List[i].pID = processId;
                                    List[i].Controler = pSimplevol;
                                }
                            }

                            if (processCharName != NULL)
                                delete[]processCharName;
                        }

                        CloseHandle(hProcess);
                    }
                }
            }

            pASControl2->Release();
        }

        pASControl->Release();
        pSessionEnum->Release();
        pASManager->Release();
        pDevice->Release();

        if (processName)
            delete[]processName;
    };

    void GetpIDList_COP()
    {
        if (!SUCCEEDED(CoInitialize(NULL)))
        {
            MBX("环境初始化失败");
            return;
        }

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        //m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMultiDevice);
        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pASManager);
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);
        //pASManager->GetAudioSessionControl(nullptr, 0, &pASControl);

        int num = 0;
        pSessionEnum->GetCount(&num);

        for (int i = 0; i < num && SINGLE; i++)
        {
            pSessionEnum->GetSession(i, &pASControl);
            hr = pASControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pASControl2);

            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                pASControl2->GetProcessId(&processId);

                if (processId != 0)
                {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                    if (hProcess) {
                        char* processName = NewStr(MAX_PATH);
                        if (GetModuleBaseNameA(hProcess, NULL, processName, MAX_PATH)) {

                            for (int j = 0; j < List.size() && SINGLE; j++)
                            {
                                if (strcmp(List[j].Name, processName) == 0 && List[j].FindState == false)
                                {
                                    hr = pASControl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&pSimplevol);
                                    if (SUCCEEDED(hr))
                                    {
                                        List[j].FindState = true;
                                        List[j].pID = processId;
                                        List[j].Controler = pSimplevol;
                                        List[j].ExePath = NewStr(EXENAMELEN);
                                        DWORD len = EXENAMELEN;
                                        if (!QueryFullProcessImageNameA(hProcess, 0, List[j].ExePath, &len)) {
                                            delete[] List[i].ExePath;
                                            List[i].ExePath = nullptr;
                                        }
                                    }
                                }
                            }
                        }
                        delete[] processName;
                        CloseHandle(hProcess);
                    }
                }
            }

            pASControl2->Release();
            pASControl2 = nullptr;
            pASControl->Release();
            pASControl = nullptr;
        }

        pSessionEnum->Release();
        pSessionEnum = nullptr;
        pASManager->Release();
        pASManager = nullptr;
        pDevice->Release();
        pDevice = nullptr;
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
        CoUninitialize();
        return;
    }

    void ShowAllVolum()
    {
        if (!SUCCEEDED(CoInitialize(NULL)))
        {
            MBX("环境初始化失败");
            return;
        }

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pASManager);
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);

        int num = 0;
        pSessionEnum->GetCount(&num);

        for (int i = 0; i < num; i++)
        {
            pSessionEnum->GetSession(i, &pASControl);
            hr = pASControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pASControl2);

            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                pASControl2->GetProcessId(&processId);

                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                if (hProcess) {
                    char* processName = NewStr(MAX_PATH);
                    DWORD size = MAX_PATH;
                    if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
                        wchar_t* WprocessName = pCharToLPWSTR(processName);
                        cout << "PID:" << processId << " --> ";
                        wcout << WprocessName << endl;
                        if(WprocessName)
                            delete[] WprocessName;
                    }
                    delete[] processName;
                    CloseHandle(hProcess);
                }
            }

            pASControl2->Release();
            pASControl2 = nullptr;
            pASControl->Release();
            pASControl = nullptr;
        }

        pSessionEnum->Release();
        pSessionEnum = nullptr;
        pASManager->Release();
        pASManager = nullptr;
        pDevice->Release();
        pDevice = nullptr;
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
        CoUninitialize();
    }

    ULONG SelectVolumContorlByFullName(char* FullName)
    {
        bool fined = false;
        ULONG PID = 0;

        if (!SUCCEEDED(CoInitialize(NULL)))
        {
            MBX("环境初始化失败");
            return 0;
        }

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pASManager);
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);

        int num = 0;
        pSessionEnum->GetCount(&num);

        for (int i = 0; i < num; i++)
        {
            pSessionEnum->GetSession(i, &pASControl);
            hr = pASControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pASControl2);

            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                pASControl2->GetProcessId(&processId);

                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                if (hProcess) {
                    char* processName = NewStr(MAX_PATH);
                    DWORD size = MAX_PATH;
                    if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
                        wchar_t* WprocessName = pCharToLPWSTR(processName);
                        if (strcmp(processName, FullName) == 0)
                        {
                            fined = true;
                            PID = processId;
                        }
                        cout << "PID:" << processId << " --> ";
                        wcout << WprocessName << endl;
                        if (WprocessName)
                            delete[] WprocessName;
                    }
                    delete[] processName;
                    CloseHandle(hProcess);
                }
            }

            pASControl2->Release();
            pASControl2 = nullptr;
            pASControl->Release();
            pASControl = nullptr;
            if (fined)break;
        }

        pSessionEnum->Release();
        pSessionEnum = nullptr;
        pASManager->Release();
        pASManager = nullptr;
        pDevice->Release();
        pDevice = nullptr;
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
        CoUninitialize();
        return PID;
    }

    ISimpleAudioVolume* GetContorl(ULONG PID)
    {
        bool Find = false;

        if (!SUCCEEDED(CoInitialize(NULL)))
        {
            MBX("环境初始化失败");
            return nullptr;
        }

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

        pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&pASManager);
        hr = pASManager->GetSessionEnumerator(&pSessionEnum);

        int num = 0;
        pSessionEnum->GetCount(&num);

        for (int i = 0; i < num && !Find; i++)
        {
            pSessionEnum->GetSession(i, &pASControl);
            hr = pASControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pASControl2);

            if (SUCCEEDED(hr)) {
                DWORD processId = 0;
                pASControl2->GetProcessId(&processId);

                if (processId == PID)
                {
                    hr = pASControl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&pSimplevol);
                    Find = true;
                }
            }

            pASControl2->Release();
            pASControl2 = nullptr;
            pASControl->Release();
            pASControl = nullptr;
        }

        pSessionEnum->Release();
        pSessionEnum = nullptr;
        pASManager->Release();
        pASManager = nullptr;
        pDevice->Release();
        pDevice = nullptr;
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
        CoUninitialize();
        return pSimplevol;
    }

    bool CheckProcessAlive(DWORD processId)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (hProcess)
        {
            CloseHandle(hProcess);
            return true;
        }
        return false;
    }

    //弃用
    ISimpleAudioVolume* GetTargetProcessVolumeControl(ULONG TargetpId)
    {
        /*hr = CoCreateGuid(&m_guidMyContext);
        if (FAILED(hr))
            return FALSE;*/
            // Get enumerator for audio endpoint devices.  


            /*if (IsMixer)
            {
                hr = m_pEnumerator->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE, &pMultiDevice);
            }
            else
            {
                hr = m_pEnumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE, &pMultiDevice);
            } */
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);

        hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMultiDevice);
        if (FAILED(hr))
            return FALSE;

        UINT deviceCount = 0;
        hr = pMultiDevice->GetCount(&deviceCount);
        if (FAILED(hr))
            return FALSE;

        for (UINT ii = 0; ii < deviceCount; ii++)
        {
            pDevice = NULL;
            hr = pMultiDevice->Item(ii, &pDevice);
            if (FAILED(hr))
                return FALSE;
            hr = pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pASManager);

            if (FAILED(hr))
                return FALSE;
            hr = pASManager->GetSessionEnumerator(&pSessionEnum);
            if (FAILED(hr))
                return FALSE;
            int nCount;
            hr = pSessionEnum->GetCount(&nCount);

            for (int i = 0; i < nCount; i++)
            {
                IAudioSessionControl* pSessionCtrl;
                hr = pSessionEnum->GetSession(i, &pSessionCtrl);
                if (FAILED(hr))
                    continue;
                IAudioSessionControl2* pSessionCtrl2;
                hr = pSessionCtrl->QueryInterface(IID_IAudioSessionControl2, (void**)&pSessionCtrl2);
                if (FAILED(hr))
                    continue;
                ULONG pid;
                hr = pSessionCtrl2->GetProcessId(&pid);
                if (FAILED(hr))
                    continue;

                hr = pSessionCtrl2->QueryInterface(IID_ISimpleAudioVolume, (void**)&pSimplevol);
                if (FAILED(hr))
                    continue;

                if (pid == TargetpId && TargetpId != 0 && pid != 0)
                {
                    m_pEnumerator->Release();
                    return pSimplevol;
                    /*pSimplevol->SetMasterVolume((float)dwVolume/100, NULL);

                    if (dwVolume == 0)
                        pSimplevol->SetMute(true, NULL);*/
                }
            }
        }

        m_pEnumerator->Release();
        return NULL;
    }

    bool IsMuted(ISimpleAudioVolume* ISAV)
    {
        BOOL state = false;
        if (ISAV != nullptr)
            ISAV->GetMute(&state);
        return state;
    };

    bool SetMute(ISimpleAudioVolume* ISAV)
    {
        if (ISAV != nullptr)
        {
            if (KeepOriginVolume == false)
                ISAV->SetMasterVolume(0.0f, NULL);
            ISAV->SetMute(true, NULL);
            return true;
        }
        return false;
    }

    bool UnMute(ISimpleAudioVolume* ISAV)
    {
        if (ISAV != nullptr)
        {
            if (KeepOriginVolume == false)
                ISAV->SetMasterVolume(1.0f, NULL);
            ISAV->SetMute(false, NULL);
            return true;
        }
        return false;
    }
};

void SimulateShiftKey(bool press)
{
    INPUT input[2] = { 0 };

    // 构造按下/释放事件
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_LSHIFT;
    input[0].ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP;

    // 更安全的发送方式（同时处理可能的消息队列溢出）
    UINT sent = SendInput(1, input, sizeof(INPUT));
    if (sent != 1) {
        // 处理发送失败的情况（可选）
        // GetLastError() 可获取错误代码
    }
}


void CloseAntiCheat(void* args) {
    Wait(30);
    StopServiceSimple("AntiCheatExpert Service");
    return;
};

void PressKey(UINT Key) {
    keybd_event(Key, 0, 0, 0);
    Sleep(1);
    keybd_event(Key, 0, 2, 0);
}

void AutoKnife(void* args)
{
    HWND hwnd = *((HWND*)args);
    while (Exit == false)
    {
        if (GetForegroundWindow() == hwnd)
        {
            while (GetKeyState(VK_XBUTTON1) < 0)
            {
                PressKey(VK_OEM_3);
                Sleep(140);
                PressKey('1');
                Sleep(1);
            }
        }
    }
}

void StopBreatheWhileFire(void* args)
{
    HWND hwnd = *((HWND*)args);
    bool bShiftPressed = false;

    while (!Exit)
    {
        if (GetForegroundWindow() == hwnd)
        {
            // 检查右键是否按下
            if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
            {
                // 检查左键状态
                bool bLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

                if (bLeftDown && !bShiftPressed)
                {
                    // 按下左Shift键
                    keybd_event(VK_OEM_1, 0, 0, 0);
                    bShiftPressed = true;
                }
                else if (!bLeftDown && bShiftPressed)
                {
                    // 松开左Shift键
                    keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                    bShiftPressed = false;
                }
            }
            else if (bShiftPressed)
            {
                // 如果右键松开但Shift仍处于按下状态，则释放
                keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                bShiftPressed = false;
            }
        }
        else if (bShiftPressed)
        {
            // 窗口失去焦点时释放Shift
            keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
            bShiftPressed = false;
        }

        Sleep(10); // 适当降低CPU占用
    }
}

void StopBreatheWhileFire_SI(void* args)
{
    HWND hwnd = *((HWND*)args);
    bool bShiftPressed = false;
    ULONGLONG lastCheckTime = 0;

    while (!Exit)
    {
        ULONGLONG currentTime = GetTickCount64();

        // 每10ms检测一次（降低CPU占用）
        if (currentTime - lastCheckTime >= 10)
        {
            lastCheckTime = currentTime;

            if (GetForegroundWindow() == hwnd)
            {
                bool bRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
                bool bLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

                if (bRightDown)
                {
                    if (bLeftDown && !bShiftPressed)
                    {
                        SimulateShiftKey(true);
                        bShiftPressed = true;
                    }
                    else if (!bLeftDown && bShiftPressed)
                    {
                        SimulateShiftKey(false);
                        bShiftPressed = false;
                    }
                }
                else if (bShiftPressed) // 右键松开时强制释放
                {
                    SimulateShiftKey(false);
                    bShiftPressed = false;
                }
            }
            else if (bShiftPressed) // 窗口失去焦点时释放
            {
                SimulateShiftKey(false);
                bShiftPressed = false;
            }
        }
        else
        {
            // 精确控制等待时间
            DWORD remaining = (DWORD)(10 - (currentTime - lastCheckTime));
            Sleep(remaining > 10 ? 10 : remaining);
        }
    }

    // 退出前确保释放Shift
    if (bShiftPressed) {
        SimulateShiftKey(false);
    }
}

int main()
{
    HANDLE mutex;
    if (CheckSelfExists(mutex, "DFTOOL"))
    {
        MBX("已存在相同进程");
        return 0;
    }

    SimpleThread ST;
    HWND self = initgraph(360, 160), hwnd = NULL, hw = NULL;
    SetWindowTextA(self,"八宝粥绿玩四合一");
    SwitchPanle SP;
    DWORD PID;
    POINT pos;

    ISimpleAudioVolume* SV = nullptr;
    SingleVolume pSingV;

    SP.drawCloseButton();
    FlushBatchDraw();

begin:

    SV = nullptr;
    hwnd = NULL;
    hw = NULL;
    Exit = false;

    while (hwnd == NULL)
    {
        hwnd = FindWindowA("UnrealWindow", "三角洲行动  ");
        Sleep(1000);
    }

    GetWindowThreadProcessId(hwnd, &PID);
 
    ST.StartThread(CloseAntiCheat, NULL);
    ST.StartThread(AutoKnife, &hwnd);
    ST.StartThread(StopBreatheWhileFire, &hwnd);

    while (SV == nullptr)
    {
        SV = pSingV.GetContorl(PID);
        Sleep(100);
    }

    SP.SwitchButton(true);
    FlushBatchDraw();
    ShowWindow(self,SW_HIDE);

    while (hwnd != NULL)
    {
        hw = GetForegroundWindow();
        if (hw == hwnd)
        {
            if(pSingV.IsMuted(SV))
                pSingV.UnMute(SV);
        }
        else
        {
            if (!pSingV.IsMuted(SV))
                pSingV.SetMute(SV);
        }

        hwnd = FindWindowA("UnrealWindow", "三角洲行动  ");
        if (hwnd == NULL)
        {
            SP.SwitchButton(false);
            FlushBatchDraw();
            SV->Release();
            SV = nullptr;
            ShowWindow(self, SW_NORMAL);
            Exit = true;
            goto begin;
        }

        Sleep(10);
    }
}
