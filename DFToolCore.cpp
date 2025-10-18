#pragma warning(disable:4996)

#include "io.h"
#include "SCManager.h"
#include "MuteWindow.h"
#include "Json.h"
#include "SimpleThread.h"
#include "tlhelp32.h"

typedef struct {
    ULONG PID;
    HWND hwnd;
}WinInfo;

bool Exit = false;
bool Reload = false;
bool ExitTh = false;
const wchar_t* serviceName = L"AntiCheatExpert Service";
const char* WindowClass = "UnrealWindow";
const char* LauncherClass = "TWINCONTROL";
const char* WindowName = "三角洲行动  ";
const char* LauncherName = "三角洲行动";
UserConfig UC;

void PressKey(UINT Key) {
    keybd_event(Key, 0, 0, 0);
    Sleep(1);
    keybd_event(Key, 0, 2, 0);
}

DWORD FindProcessId(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return processId;
}

bool TerminateTargetProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        std::cerr << "无法打开进程，错误代码: " << GetLastError() << std::endl;
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    if (!result) {
        std::cerr << "终止进程失败，错误代码: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

void ConfigThread(void* args)
{
    HWND hwnd = NULL;
    int res = 0;
    bool showing = false;

    while (!ExitTh)
    {
        if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0 && showing == false)
        {
            if ((GetAsyncKeyState(VK_NUMPAD0) & 0x8000) != 0 && showing == false)
            {
                showing = true;

                res = MessageBoxA(NULL, "希望退出整个程序 选择【是】\t 希望更新配置 选择【否】\n打开配置文件选择【 取消 】", "选项", MB_YESNOCANCEL | MB_SYSTEMMODAL);

                hwnd = FindWindowA("#32770 (对话框)","选项");
                while (hwnd != NULL)
                    Sleep(100);

                switch (res)
                {
                case IDYES:
                    Exit = true;
                    GetAsyncKeyState(VK_LCONTROL);
                    GetAsyncKeyState(VK_NUMPAD0);
                    showing = false;
                    return;
                case IDCANCEL:
                    ShellExecuteA(NULL, "open", ".\\UserConfig.json", NULL, NULL, SW_SHOWNORMAL);
                    GetAsyncKeyState(VK_LCONTROL);
                    GetAsyncKeyState(VK_NUMPAD0);
                    showing = false;
                    break;
                case IDNO:
                    Reload = true;
                    GetAsyncKeyState(VK_LCONTROL);
                    GetAsyncKeyState(VK_NUMPAD0);
                    showing = false;
                    return;
                }
            }
        }

        Sleep(100);
    }

    return;
}

void MuteWindow(void* args) {

    WinInfo* WI = (WinInfo*)args;
    ISimpleAudioVolume* SV = nullptr;
    SingleVolume pSingV;
    HWND hw = NULL;

    while (!ExitTh)
    {
        while (SV == nullptr && !ExitTh)
        {
            SV = pSingV.GetContorl(WI->PID);
            Sleep(100);
        }

        hw = GetForegroundWindow();
        if (hw == WI->hwnd)
        {
            if (pSingV.IsMuted(SV))
                pSingV.UnMute(SV);
        }
        else
        {
            if (!pSingV.IsMuted(SV))
                pSingV.SetMute(SV);
        }
    }

    SV->Release();
    SV = nullptr;
    return;
};
 
void CloseAntiCheat(void* args)
{
    Wait(10);
    StopWindowsService(serviceName);
    return;
};

void AutoKnife(void* args)
{
    WinInfo* WI = (WinInfo*)args;
    char key = '1';
    if (UC.MainWeapon == false)
        key = '4';

    while (!ExitTh)
    {
        if (GetForegroundWindow() == WI->hwnd)
        {
            while (GetKeyState(VK_XBUTTON1) < 0)
            {
                PressKey(VK_OEM_3);
                Sleep(UC.KnifeTime + rand()%15 - 10);
                PressKey(key);
            }

            Sleep(100);
        }

        Sleep(100);
    }

    return;
}

void StopBreatheWhileFire(void* args)
{
    WinInfo* WI = (WinInfo*)args;
    bool bShiftPressed = false;

    while (!ExitTh)
    {
        if (GetForegroundWindow() == WI->hwnd)
        {
            if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
            {
                bool bLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

                if (bLeftDown && !bShiftPressed)
                {
                    keybd_event(VK_OEM_1, 0, 0, 0);
                    bShiftPressed = true;
                }
                else if (!bLeftDown && bShiftPressed)
                {
                    keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                    bShiftPressed = false;
                }
            }
            else if (bShiftPressed)
            {
                keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                bShiftPressed = false;
            }
        }
        else if (bShiftPressed)
        {
            keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
            bShiftPressed = false;
        }
        Sleep(100);
    }

    return;
}

void Slide(void* args) {

    WinInfo* WI = (WinInfo*)args;
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


    while (!ExitTh)
    {
        if (GetForegroundWindow() == WI->hwnd)
        {
            if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
                if (!bNumPad0Pressed) {

                    SendInput(1, &numDown, sizeof(INPUT));
                    bNumPad0Pressed = true;
                }

                SendInput(1, &keyDown, sizeof(INPUT));
                Sleep(50);
                SendInput(1, &keyUp, sizeof(INPUT));
                Sleep(30);
            }
            else {
                if (bNumPad0Pressed) {
                    SendInput(1, &numUp, sizeof(INPUT));
                    bNumPad0Pressed = false;
                }
            }
        }
        Sleep(10);
    }

    return;
}

void Flasher(void* args)
{
    bool* open = (bool*)args;
    while (*open)
    {
        PressKey('Y');
    }
}

void UnlimitedFlash(void* args)
{
    SimpleThread ST;
    WinInfo* WI = (WinInfo*)args;
    bool Flash = false;
    while (!ExitTh)
    {
        if (GetForegroundWindow() == WI->hwnd)
        {
            if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0)
            {
                Flash = true;
                ST.StartThread(Flasher, &Flash);
                Wait(UC.FlashStopTime);
                Flash = false;
            }
            Sleep(10);
        }
        Sleep(100);
    }

    return;
}

int main(int argc,char* argv[])
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    HANDLE mutex;
    if (CheckSelfExists(mutex, "DFTOOLCORE"))
    {
        MBX("已存在相同进程");
        return 0;
    }

    if (_access("UserConfig.json", 0) != 0)
    {
        WriteDefault();
        MBX("检查到无配置文件，已使用默认配置，若需要修改请自行修改同文件夹下生成的 UserConfig.json 文件");
    }
    
    SimpleThread ST;
    WinInfo WI;
    HWND hw;
    HANDLE Threads[7] = { NULL };
    int res;

BEGIN:

    res = 0;
    Exit = false;
    Reload = false;
    ExitTh = false;
    WI.hwnd = NULL; WI.PID = NULL;
    ConfigInit(&UC);

    if (!parse_user_config("UserConfig.json", &UC))
        MBX("解析Json文件失败，正在使用默认配置");

    Threads[0] = ST.StartThread(ConfigThread, NULL);

    if (UC.ShowConfig)
    {
        std::string configinfo = "当前设置：";
        configinfo += "\n启动时显示当前配置："; if (UC.ShowConfig)configinfo += "激活中";
        configinfo += "\n取消刀："; if (UC.FuncSwitch.AutoKnife)configinfo += "激活中"; else configinfo += "未激活";
        if (UC.FuncSwitch.AutoKnife) {
            char* buff = new char[16]; memset(buff, 0, 16);
            configinfo += "\n取消刀切换武器："; if (UC.MainWeapon)configinfo += "主武器"; else configinfo += "手枪";
            configinfo += "\n取消刀动画时长： "; 
            itoa(UC.KnifeTime, buff, 10);
            configinfo += buff;
            configinfo += " 毫秒";
            delete[] buff;
            buff = nullptr;
        }
        configinfo += "\n关闭ACE："; if (UC.FuncSwitch.CloseAce)configinfo += "激活中"; else configinfo += "未激活";
        configinfo += "\n威慑手电："; if (UC.FuncSwitch.Flash)configinfo += "激活中"; else configinfo += "未激活";
        if (UC.FuncSwitch.AutoKnife) {
            char* buff = new char[16]; memset(buff, 0, 16);
            configinfo += "\n威慑手电持续时长： ";
            itoa(UC.FlashStopTime, buff, 10);
            configinfo += buff;
            configinfo += " 秒";
            delete[] buff;
            buff = nullptr;
        }
        configinfo += "\n切窗口游戏静音："; if (UC.FuncSwitch.MuteWindow)configinfo += "激活中"; else configinfo += "未激活";
        configinfo += "\n滑步："; if (UC.FuncSwitch.Slide)configinfo += "激活中"; else configinfo += "未激活";
        configinfo += "\n自动屏息："; if (UC.FuncSwitch.StopBreath)configinfo += "激活中"; else configinfo += "未激活";

        MBX(configinfo.c_str());
    }

    while (WI.hwnd == NULL)
    {
        if (Reload || Exit)
            goto END;

        WI.hwnd = FindWindowA(WindowClass, WindowName);
        Sleep(1000);
    }
    GetWindowThreadProcessId(WI.hwnd, &WI.PID);
    SendMessageA(FindWindowA(LauncherClass, LauncherName), WM_CLOSE, 0, 0);

    if(UC.FuncSwitch.MuteWindow)
        Threads[1] = ST.StartThread(MuteWindow,(void*)&WI);
    if (UC.FuncSwitch.CloseAce)
        Threads[2] = ST.StartThread(CloseAntiCheat, NULL);
    if(UC.FuncSwitch.AutoKnife)
        Threads[3] = ST.StartThread(AutoKnife, (void*)&WI);
    if (UC.FuncSwitch.Flash)
        Threads[4] = ST.StartThread(UnlimitedFlash, (void*)&WI);
    if (UC.FuncSwitch.Slide)
        Threads[5] = ST.StartThread(Slide,(void*)&WI);
    if (UC.FuncSwitch.StopBreath)
        Threads[6] = ST.StartThread(StopBreatheWhileFire,(void*)&WI);
    

    hw = WI.hwnd;
    while(hw != NULL)
    {
        if (Reload || Exit)
            goto END;

        hw = FindWindowA(WindowClass, WindowName);
        Sleep(1000);
    }

END:

    ExitTh = true;
    DWORD single = WaitForMultipleObjects(7, Threads, true, INFINITE);
    if ((single == WAIT_OBJECT_0) || (single == WAIT_ABANDONED_0)){
        for (int i = 0; i < 7; i++)
            CloseHandle(Threads[i]);
    }
    else {
        for (int i = 0; i < 7; i++)
        {
            TerminateThread(Threads[i], 0);
            CloseHandle(Threads[i]);
        }
    }

    if (!Reload)
    {
        hw = FindWindowA(WindowClass, WindowName);
        if(hw == NULL)
            TerminateTargetProcess(FindProcessId(L"delta_force_launcher.exe"));
    }

    if(!Exit)
        goto BEGIN;

    return 0;
}
