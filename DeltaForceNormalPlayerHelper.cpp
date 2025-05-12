
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
    return;
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

            Sleep(100);
        }

        Sleep(100);
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

        Sleep(100); // 适当降低CPU占用
    }
}

void Slide(void* args) {

    HWND hwnd = *((HWND*)args);
    bool bNumPad0Pressed = false;

    INPUT keyDown = { 0 };
    keyDown.type = INPUT_KEYBOARD;
    keyDown.ki.wVk = 'W';

    INPUT keyUp = { 0 };
    keyUp.type = INPUT_KEYBOARD;
    keyUp.ki.wVk = 'W';
    keyUp.ki.dwFlags = KEYEVENTF_KEYUP;

    INPUT numDown = { 0 };
    numDown.type = INPUT_KEYBOARD;
    numDown.ki.wVk = VK_NUMPAD0;

    INPUT numUp = { 0 };
    numUp.type = INPUT_KEYBOARD;
    numUp.ki.wVk = VK_NUMPAD0;
    numUp.ki.dwFlags = KEYEVENTF_KEYUP;


    while (!Exit)
    {
        if (GetForegroundWindow() == hwnd)
        {
            // 检测侧键2是否按下
            if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
                if (!bNumPad0Pressed) {
                    
                    SendInput(1, &numDown, sizeof(INPUT));
                    bNumPad0Pressed = true;
                }

                // 按下W键
                SendInput(1, &keyDown, sizeof(INPUT));

                // 保持按下状态50毫秒
                Sleep(50);

                // 释放W键
                SendInput(1, &keyUp, sizeof(INPUT));

                // 控制操作频率（总间隔时间 = 保持时间 + 间隔时间）
                Sleep(30); // 可根据需要调整这个间隔
            }
            else {
                if (bNumPad0Pressed) {
                    SendInput(1, &numUp, sizeof(INPUT));
                    bNumPad0Pressed = false;
                }
            }
        }
        // 降低CPU占用
        Sleep(10);
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
    HWND self = initgraph(360, 160), hwnd = NULL, hw = NULL,launcher = NULL;
    SetWindowTextA(self,"八宝粥绿玩五合一");
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
    launcher = NULL;
    Exit = false;

    while (hwnd == NULL)
    {
        hwnd = FindWindowA("UnrealWindow", "三角洲行动  ");
        Sleep(1000);
    }

    GetWindowThreadProcessId(hwnd, &PID);
    launcher = FindWindowA("TWINCONTROL", "三角洲行动");
 
    ST.StartThread(CloseAntiCheat, NULL);
    ST.StartThread(AutoKnife, &hwnd);
    ST.StartThread(StopBreatheWhileFire, &hwnd);
    ST.StartThread(Slide, &hwnd);

    SendMessageA(launcher, WM_CLOSE, 0, 0);

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

        Sleep(100);
    }
}
