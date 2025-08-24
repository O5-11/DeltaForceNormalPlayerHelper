#include "MuteWindow.h"


bool SingleVolume::Init()
{
    CoInitialize(nullptr);

    wszDeviceName = new wchar_t[MAX_PATH]; memset(wszDeviceName, 0, MAX_PATH * sizeof(wchar_t));
    PropVariantInit(&pv);

    return TRUE;
}

SingleVolume::SingleVolume()
{
    Initialized = Init();
};

SingleVolume::~SingleVolume()
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

ISimpleAudioVolume* SingleVolume::GetContorl(ULONG PID)
{
    bool Find = false;

    if (!SUCCEEDED(CoInitialize(NULL)))
    {
        MessageBoxA(NULL,"Audio环境初始化失败","提示",MB_OK | MB_SYSTEMMODAL);
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

bool SingleVolume::IsMuted(ISimpleAudioVolume* ISAV)
{
    BOOL state = false;
    if (ISAV != nullptr)
        ISAV->GetMute(&state);
    return state;
};

bool SingleVolume::SetMute(ISimpleAudioVolume* ISAV)
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

bool SingleVolume::UnMute(ISimpleAudioVolume* ISAV)
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
