#pragma once

#include "cJSON.h"

typedef struct FunctionSwitch {
    bool CloseAce;
    bool MuteWindow;
    bool AutoKnife;
    bool StopBreath;
    bool Slide;
    bool Flash;
} FuncSw;

typedef struct UserConfig {
    FuncSw FuncSwitch;
    int FlashStopTime;
    int KnifeTime;
} UserConfig;

void WriteDefault();

char* read_file(const char* filename);

int parse_user_config(const char* filename, UserConfig* config);

void ConfigInit(UserConfig* UC);