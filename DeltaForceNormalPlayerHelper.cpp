
#include "global.h"
#include "SimpleThread.h"
#include "SCManager.h"
#include "MuteWindow.h"
#include "tlhelp32.h"

#pragma region 全局变量

HWND hwnd;

int Page = 0;
int Page_last = 0;
int Page_count = 5;
bool FuncSw[6] = { 0,0,0,0,0,0 };
bool Drawed[7] = { 1,1,1,1,1,1,1 };
bool GameState = false;
bool GameState_last = false;
HANDLE Threads[6] = { NULL };
int t_knife = 165;
bool b_MainWeapon = true;
int t_flash = 5;
IMAGE i_switch_off;
IMAGE i_switch_on;
IMAGE i_bar_func;
IMAGE i_bar_page;
IMAGE i_bar_input;
IMAGE i_icon_mainconfig;
IMAGE i_icon_autoknife;
IMAGE i_icon_flash;
IMAGE i_icon_save;
IMAGE i_icon_info;
IMAGE i_icon_work;
IMAGE i_icon_sample;
IMAGE i_icon_online;
IMAGE i_icon_offline;
IMAGE i_pic_keysetting;
IMAGE i_pic_gamesetting;

typedef struct Data {
    bool SW[7];
    int tk;
    int tf;
}DATA;

RECT SwitchsPos[7] = {
    //主页面功能开关
    {860,100,920,130},
    {860,200,920,230},
    {860,300,920,330},
    {860,400,920,430},
    {860,500,920,530},
    {860,600,920,630},
    //取消刀武器模式
    {860,100,920,130}
};
RECT PagePos[5] = {
    {20,80,220,130},
    {20,140,220,190},
    {20,200,220,250},
    {20,260,220,310},
    {20,320,220,370}
};
RECT InputPos[1] = {
    {280,200,930,250}
};
RECT ButtonPos[2] = {
    {40,640,70,670},
    {135,638,165,668}
};

const wchar_t* serviceName = L"AntiCheatExpert Service";
const char* WindowClass = "UnrealWindow";
const char* LauncherClass = "TWINCONTROL";
const char* WindowName = "三角洲行动  ";
const char* LauncherName = "三角洲行动";

DWORD g_pid = 0;
HWND g_hwnd = NULL;

#pragma endregion

void SaveConfig();
void ReadConfig();
bool IsPressed(int Key);
bool IsPointInRect(POINT& pos, RECT& rec);
void ReDraw();
void DrawSwitch(RECT& rec, IMAGE& img);
void MainUIListen(void* args);
DWORD FindProcessId(const std::wstring& processName);
bool TerminateTargetProcess(DWORD processId);

unsigned int __stdcall MuteWindow(void* args);
unsigned int __stdcall CloseAntiCheat(void* args);
unsigned int __stdcall AutoKnife(void* args);
unsigned int __stdcall StopBreatheWhileFire(void* args);
unsigned int __stdcall Slide(void* args);
unsigned int __stdcall UnlimitedFlash(void* args);

int main()
{
    HANDLE mutex;
    if (CheckSelfExists(mutex, "DeltaForceNormalPlayerHelper"))
    {
        return 0;
    }

    SimpleThread ST;

    ReadConfig();

    hwnd = initgraph(960, 720);
    SetWindowTextA(hwnd,"我真是八宝粥绿玩");
    HICON hIcon = (HICON)LoadImage(NULL, L"PicRes\\O5-11.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    ST.StartThread(MainUIListen, (void*)&FuncSw);

    loadimage(&i_switch_off, L"PicRes\\switch-off.png", 60, 30, true);
    loadimage(&i_switch_on, L"PicRes\\switch-on.png", 60, 30, true);
    loadimage(&i_bar_func, L"PicRes\\func-bar.png", 660, 80, true);
    loadimage(&i_bar_page, L"PicRes\\page-bar.png", 200, 50, true);
    loadimage(&i_bar_input, L"PicRes\\input-bar.png", 650, 50, true);
    loadimage(&i_icon_mainconfig, L"PicRes\\config.png", 20, 20, true);
    loadimage(&i_icon_autoknife, L"PicRes\\sword.png", 20, 20, true);
    loadimage(&i_icon_flash, L"PicRes\\flash.png", 20, 20, true);
    loadimage(&i_icon_work, L"PicRes\\Working.png", 20, 20, true);
    loadimage(&i_icon_sample, L"PicRes\\sample.png", 20, 20, true);
    loadimage(&i_icon_save, L"PicRes\\save.png", 30, 30, true);
    loadimage(&i_icon_info, L"PicRes\\info.png", 30, 30, true);
    loadimage(&i_icon_online, L"PicRes\\online.png", 30, 30, true);
    loadimage(&i_icon_offline, L"PicRes\\offline.png", 30, 30, true);
    loadimage(&i_pic_keysetting, L"PicRes\\keysetting.png");
    loadimage(&i_pic_gamesetting, L"PicRes\\gamesetting.png");

    g_hwnd = FindWindowA(WindowClass, WindowName);
    if (g_hwnd != NULL)
        GameState = true;

    ReDraw();

    while (1) {

        HWND new_hwnd = FindWindowA(WindowClass, WindowName);
        bool new_GameState = (new_hwnd != NULL);
        if (new_GameState != GameState) {
            GameState = new_GameState;
            g_hwnd = new_hwnd;
            ReDraw();
            GameState_last = GameState;
        }

        for (int i = 0; i < 6; i++) {
            if (FuncSw[i] != Drawed[i] && Page == 0) {
                DrawSwitch(SwitchsPos[i], FuncSw[i] ? i_switch_on : i_switch_off);
                Drawed[i] = FuncSw[i];
            }
        }

        if (b_MainWeapon != Drawed[6] && Page == 1) {
            DrawSwitch(SwitchsPos[6], b_MainWeapon ? i_switch_on : i_switch_off);
            Drawed[6] = b_MainWeapon;
        }

        if (Page != Page_last) {
            ReDraw();
            Page_last = Page;
        }

        if (GameState) {
            for (int i = 0; i < 6; i++) {
                DWORD exitCode = STILL_ACTIVE;
                bool isThreadActive = false;

                if (Threads[i] != NULL) {
                    if (GetExitCodeThread(Threads[i], &exitCode)) {
                        isThreadActive = (exitCode == STILL_ACTIVE);
                    }
                    else {
                        CloseHandle(Threads[i]);
                        Threads[i] = NULL;
                    }
                }

                if (!FuncSw[i] && Threads[i] != NULL) {
                    if (isThreadActive) {
                        if (WaitForSingleObject(Threads[i], 100) == WAIT_OBJECT_0) {
                            CloseHandle(Threads[i]);
                            Threads[i] = NULL;
                        }
                    }
                    else {
                        CloseHandle(Threads[i]);
                        Threads[i] = NULL;
                    }
                    continue;
                }

                if (FuncSw[i] && (Threads[i] == NULL || !isThreadActive)) {
                    if (Threads[i] != NULL) {
                        CloseHandle(Threads[i]);
                        Threads[i] = NULL;
                    }
                    switch (i) {
                    case 0: Threads[i] = ST.StartThreadEx(CloseAntiCheat, NULL); break;
                    case 1: Threads[i] = ST.StartThreadEx(MuteWindow, NULL); break;
                    case 2: Threads[i] = ST.StartThreadEx(AutoKnife, NULL); break;
                    case 3: Threads[i] = ST.StartThreadEx(StopBreatheWhileFire, NULL); break;
                    case 4: Threads[i] = ST.StartThreadEx(Slide, NULL); break;
                    case 5: Threads[i] = ST.StartThreadEx(UnlimitedFlash, NULL); break;
                    }
                }
            }
        }

        Sleep(10);
    }

    closegraph();
    return 0;
}

void ReDraw() {

    LOGFONT f;
    gettextstyle(&f);
    f.lfWeight = FW_BOLD;
    _tcscpy(f.lfFaceName, _T("宋体"));
    f.lfQuality = PROOF_QUALITY;
    f.lfHeight = 26;
    settextstyle(&f);

    BeginBatchDraw();
    cleardevice();
    setfillcolor(RGB(17, 24, 39));
    solidrectangle(0, 0, 250, 720);
    setfillcolor(RGB(3, 7, 28));
    solidrectangle(251, 0, 960, 720);

    setbkmode(TRANSPARENT);

    outtextxy(30, 20, L"八宝粥绿玩辅助");

    if (Page == 0)
    {
        transparentimage(NULL, 20, 80, &i_bar_page);

        for (int i = 0; i < 6; i++)
        {
            transparentimage(NULL, 280, 75 + 100 * i, &i_bar_func);
            transparentimage(NULL, SwitchsPos[i].left, SwitchsPos[i].top, &(FuncSw[i] ? i_switch_on : i_switch_off));
        }

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 30, L"功能总开关");

        f.lfHeight = 18;
        settextstyle(&f);
        outtextxy(320, 95, L"自动关闭ACE");
        outtextxy(320, 195, L"切屏静音");
        outtextxy(320, 295, L"取消刀法");
        outtextxy(320, 395, L"自动屏息");
        outtextxy(320, 495, L"无声滑步");
        outtextxy(320, 595, L"威慑爆闪");

        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextcolor(RGB(92, 94, 102));
        settextstyle(&f);
        outtextxy(320, 120, L"游戏启动时自动关闭扫盘掉帧的ACE");
        outtextxy(320, 220, L"游戏窗口不在最前时将其静音，给你一段纯净的听觉体验");
        outtextxy(320, 320, L"刀人机一等一的好用，谨慎刀人，吃举报有封号可能");
        outtextxy(320, 420, L"开镜的时候开枪才有用，且效果与枪的据枪属性强相关");
        outtextxy(320, 520, L"旧版本伪装人机步还行，现在一般当做节省体力的移动手段");
        outtextxy(320, 620, L"爆闪大拉专克高手，菜鸡不会上当");
    }
    else if (Page == 1)
    {
        transparentimage(NULL, 280, 75, &i_bar_func);
        transparentimage(NULL, SwitchsPos[6].left, SwitchsPos[6].top, &(b_MainWeapon ? i_switch_on : i_switch_off));
        transparentimage(NULL, 20, 140, &i_bar_page);
        transparentimage(NULL, 280, 200, &i_bar_input);

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 30, L"取消刀动画时长");

        f.lfHeight = 18;
        settextstyle(&f);
        outtextxy(320, 95, L"当局主武器不为手枪");
        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextcolor(RGB(92, 94, 102));
        settextstyle(&f);
        outtextxy(320, 120, L"只带G18夺舍时也能用出取消刀了");

        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextstyle(&f);
        settextcolor(WHITE);
        outtextxy(280, 170, L"设置自定义时长(ms):");
        char* buff = new char[10]; memset(buff, 0, 10);
        itoa(t_knife, buff, 10);
        TCHAR* show = pCharToLPWSTR(buff);
        f.lfHeight = 18;
        settextstyle(&f);
        outtextxy(300, 215, show);
        delete[] show;
        delete[] buff;
    }
    else if (Page == 2)
    {
        transparentimage(NULL, 20, 200, &i_bar_page);
        transparentimage(NULL, 280, 200, &i_bar_input);

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 30, L"威慑爆闪时长");

        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextstyle(&f);
        outtextxy(280, 170, L"设置自定义时长(s):");
        char* buff = new char[10]; memset(buff, 0, 10);
        itoa(t_flash, buff, 10);
        TCHAR* show = pCharToLPWSTR(buff);
        f.lfHeight = 18;
        settextstyle(&f);
        outtextxy(300, 215, show);
        delete[] show;
        delete[] buff;
    }
    else if (Page == 3) {
        transparentimage(NULL, 20, 260, &i_bar_page);

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 30, L"改键图示");

        transparentimage(NULL, 280, 75, &i_pic_keysetting);
        transparentimage(NULL, 280, 585, &i_pic_gamesetting);

        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextstyle(&f);

        outtextxy(320, 140, L"取消刀法");
        outtextxy(320, 160, L"游戏内设置 快速近战:");
        outtextxy(540, 160, L"功能激活键:");
        outtextxy(320, 195, L"无声滑步");
        outtextxy(320, 215, L"游戏内设置 奔跑/快游:");
        outtextxy(540, 215, L"功能激活键:");
        outtextxy(320, 250, L"自动屏息");
        outtextxy(320, 270, L"游戏内设置 屏息:");
        outtextxy(540, 270, L"功能激活: 开镜(鼠标右键)后射击(鼠标左键)");
    }
    else if (Page == 4)
    {
        transparentimage(NULL, 20, 320, &i_bar_page);

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 30, L"后续开发计划");

        f.lfHeight = 14;
        f.lfWeight = FW_NORMAL;
        settextstyle(&f);
        outtextxy(280, 170, L"一键从裤裆里掏出子弹到口袋的功能");
        outtextxy(280, 200, L"自定义游戏内/功能键位：比如取消刀不再是游戏内绑～键，也不再是鼠标侧键激活，按你自己习惯的来");
        outtextxy(280, 230, L"检查版本更新的功能......");
        outtextxy(280, 290, L"但是谁又知道这游戏还能活多久，又还有多少人需要我这东西呢.....");
        outtextxy(280, 320, L"人心若是烧没了，修好一座破庙又有什么用呢");

        f.lfHeight = 26;
        settextstyle(&f);
        outtextxy(280, 400, L"总之关注");
        outtextxy(280, 440, L"Github:O5-11 Bilbili UID:9427514");
        outtextxy(280, 480, L"谢谢喵～");
    }

    transparentimage(NULL, 40, 95, &i_icon_mainconfig);
    transparentimage(NULL, 40, 155, &i_icon_autoknife);
    transparentimage(NULL, 40, 215, &i_icon_flash);
    transparentimage(NULL, 40, 275, &i_icon_sample);
    transparentimage(NULL, 40, 335, &i_icon_work);

    transparentimage(NULL, 40, 640, &i_icon_save);
    transparentimage(NULL, 135, 638, &i_icon_info);

    f.lfHeight = 18;
    f.lfWeight = FW_NORMAL;
    settextstyle(&f);
    settextcolor(WHITE);
    outtextxy(100, 95, L"功能开关");
    outtextxy(100, 155, L"取消刀法");
    outtextxy(100, 215, L"威慑爆闪");
    outtextxy(100, 275, L"改键示例");
    outtextxy(100, 335, L"更多功能");

    if (GameState)
    {
        transparentimage(NULL, 40, 560, &i_icon_online);
        outtextxy(100, 565, L"游戏已开");
    }
    else
    {
        transparentimage(NULL, 40, 560, &i_icon_offline);
        outtextxy(100, 565, L"游戏未开");
    }
    


    EndBatchDraw();
}

void DrawSwitch(RECT& rec, IMAGE& img)
{
    BeginBatchDraw();
    transparentimage(NULL, rec.left, rec.top, &img);
    EndBatchDraw();
};

void MainUIListen(void* args)
{
    bool (*FuncUISw)[3] = (bool(*)[3])args;
    POINT pos;
    bool last_clicked = false;
    while (1)
    {
        if (GetForegroundWindow() == hwnd)
        {
            GetCursorPos(&pos);
            ScreenToClient(hwnd, &pos);
            bool current_clicked = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

            // 仅在点击状态从松开变为按下时触发（上升沿检测）
            if (current_clicked && !last_clicked) {

                //切换页面
                for (int i = 0; i < Page_count; i++)
                {
                    if (IsPointInRect(pos, PagePos[i])) {
                        Page = i;
                    }
                }

                //保存和更多按钮
                for (int i = 0; i < 2; i++)
                {
                    if (IsPointInRect(pos, ButtonPos[i])) {
                        switch (i)
                        {
                        case 0:
                            //保存
                            SaveConfig();
                            break;
                        case 1:
                            //更多
                            ShellExecuteA(NULL, "open", "https://space.bilibili.com/9427514", 0, 0, 0);
                            break;
                        }
                    }
                }

                if (Page == 0)
                {
                    for (int i = 0; i < 6; i++)
                    {
                        if (IsPointInRect(pos, SwitchsPos[i])) {
                            (*FuncUISw)[i] = !(*FuncUISw)[i];
                        }
                    }
                }
                else if (Page == 1)
                {
                    if (IsPointInRect(pos, SwitchsPos[6])) {
                        b_MainWeapon = !b_MainWeapon;
                    }
                    if (IsPointInRect(pos, InputPos[0])) {
                        wchar_t num[10];
                        InputBox(num, 10, L"建议时长 150ms 到 190ms，电锯大约在 400ms", L"取消刀动画时长");
                        int input = _wtoi(num);
                        if (input == 0)
                        {
                            t_knife = 165;
                            MessageBoxA(NULL, "输入中有无法解析的字符，当前数值重置为165ms", "警告", MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
                        }
                        else if (input > 1000 || input < 0)
                        {
                            t_knife = 165;
                            MessageBoxA(NULL, "输入数值超过阈值，当前数值重置为165ms", "警告", MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
                        }
                        else
                        {
                            t_knife = input;
                        }
                        ReDraw();
                    }
                }
                else if (Page == 2)
                {
                    if (IsPointInRect(pos, InputPos[0])) {
                        wchar_t num[10];
                        InputBox(num, 10, L"建议时长 3s 到 5ms，太长自己也受不了", L"爆闪手电时长");
                        int input = _wtoi(num);
                        if (input == 0)
                        {
                            t_flash = 5;
                            MessageBoxA(NULL, "输入中有无法解析的字符，当前数值重置为5s", "警告", MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
                        }
                        if (input < 0 || input > 10)
                        {
                            t_flash = 5;
                            MessageBoxA(NULL, "输入超过阈值，当前数值重置为5s", "警告", MB_OK | MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
                        }
                        else
                        {
                            t_flash = input;
                        }
                        ReDraw();
                    }
                }

            }
            last_clicked = current_clicked;

            Sleep(10);
        }
        Sleep(10);
    }
}

void SaveConfig()
{
    FILE* f = fopen("DFUserConfig.dat", "wb+");

    DATA dat;

    for (int i = 0; i < 6; i++)
    {
        dat.SW[i] = FuncSw[i];
    }
    dat.SW[6] = b_MainWeapon;
    dat.tf = t_flash;
    dat.tk = t_knife;

    fwrite(&dat, sizeof(DATA), 1, f);
    fclose(f);

    MBX("已保存当前配置文件");
};

void ReadConfig()
{

    FILE* f = fopen("DFUserConfig.dat", "rb+");
    if (f == nullptr)
        return;

    DATA dat; memset(&dat, 0, sizeof(DATA));

    fread(&dat, sizeof(DATA), 1, f);

    for (int i = 0; i < 6; i++)
    {
        FuncSw[i] = dat.SW[i];
    }
    b_MainWeapon = dat.SW[6];
    t_flash = dat.tf;
    t_knife = dat.tk;

    fclose(f);
};

bool IsPressed(int Key) {
    return (GetAsyncKeyState(Key) & 0x8000) != 0;
};

bool IsPointInRect(POINT& pos, RECT& rec)
{
    if (pos.x >= rec.left && pos.x <= rec.right && pos.y >= rec.top && pos.y <= rec.bottom)
        return true;
    return false;
};

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
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    if (!result) {
        return false;
    }

    return true;
}

unsigned int __stdcall MuteWindow(void* args) {

    ISimpleAudioVolume* SV = nullptr;
    SingleVolume pSingV;
    HWND hw = NULL;

    GetWindowThreadProcessId(g_hwnd, &g_pid);

    while (FuncSw[1] && GameState)
    {
        while (SV == nullptr && FuncSw[1])
        {
            SV = pSingV.GetContorl(g_pid);
            Sleep(100);
        }

        hw = GetForegroundWindow();
        if (hw == g_hwnd)
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

    pSingV.UnMute(SV);
    if(SV != nullptr)
        SV->Release();
    SV = nullptr;
    return 0;
};

unsigned int __stdcall CloseAntiCheat(void* args)
{
    Wait(10);
    StopWindowsService(serviceName);
    return 0;
};

unsigned int __stdcall AutoKnife(void* args)
{
    while (FuncSw[2] && GameState) {
        if (GetForegroundWindow() == g_hwnd) {
            while (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) {
                PressKey(VK_OEM_3);
                Sleep(t_knife + (rand() % 21 - 10));
                if (b_MainWeapon)
                    PressKey('1');
                else
                    PressKey('4');

            }
            Sleep(100);
        }
        Sleep(100);
    }
    return 0;
}

unsigned int __stdcall StopBreatheWhileFire(void* args)
{
    bool bShiftPressed = false;

    while (FuncSw[3] && GameState)
    {
        if (GetForegroundWindow() == g_hwnd)
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
    return 0;
}

unsigned int __stdcall Slide(void* args) {

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


    while (FuncSw[4] && GameState)
    {
        if (GetForegroundWindow() == g_hwnd)
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
    return 0;
}

unsigned int __stdcall UnlimitedFlash(void* args)
{
    SimpleThread ST;
    clock_t beg = 0;
    clock_t now = 0;
    while (FuncSw[5] && GameState)
    {
        if (GetForegroundWindow() == g_hwnd)
        {
            if ((GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0)
            {
                beg = clock();
                now = clock();
                while (GetForegroundWindow() == g_hwnd && FuncSw[5] && (((now - beg) / CLOCKS_PER_SEC) < t_flash))
                {
                    PressKey('Y');
                    now = clock();
                }
            }
            Sleep(10);
        }
        Sleep(100);
    }
    return 0;
}