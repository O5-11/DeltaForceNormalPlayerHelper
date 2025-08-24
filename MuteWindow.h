#pragma once

#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>
#include <audiopolicy.h>

class SingleVolume
{
private:
    bool Initialized = false;
    bool KeepOriginVolume = true;

    HRESULT hr = S_OK;
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
    ISimpleAudioVolume* pSimplevol = nullptr;

    wchar_t* wszDeviceName;

public:

    bool Init();
    SingleVolume();
    ~SingleVolume();
    ISimpleAudioVolume* GetContorl(ULONG PID);
    bool IsMuted(ISimpleAudioVolume* ISAV);
    bool SetMute(ISimpleAudioVolume* ISAV);
    bool UnMute(ISimpleAudioVolume* ISAV);
};