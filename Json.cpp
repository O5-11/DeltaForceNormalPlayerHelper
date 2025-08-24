

#include <iostream>
#include <fstream>
#include "Json.h"

void ConfigInit(UserConfig* UC)
{
    UC->FlashStopTime = 5;
    UC->KnifeTime = 170;
    UC->FuncSwitch.AutoKnife = true;
    UC->FuncSwitch.CloseAce = true;
    UC->FuncSwitch.Flash = true;
    UC->FuncSwitch.MuteWindow = true;
    UC->FuncSwitch.Slide = true;
    UC->FuncSwitch.StopBreath = true;
};

void WriteDefault() {

    const char* data = "{\n\t\"FlashStopTime\": 5,\n\t\"KnifeTime\" : 170,\n\t\"FuncSwitch\" : {\n\t\t\"AutoKnife\": true,\n\t\t\"CloseAce\" : true,\n\t\t\"Flash\" : true,\n\t\t\"MuteWindow\" : true,\n\t\t\"Slide\" : true,\n\t\t\"StopBreath\" : true\n\t}\n}\n";

    std::fstream f("UserConfig.json", std::ios::out | std::ios::binary);
    f.write(data, 189);
    f.close();
}

char* read_file(const char* filename) {

    std::fstream f(filename, std::ios::in | std::ios::binary);

    if (!f.is_open()) {
        return nullptr;
    };

    char* str = new char[256];
    memset(str, 0, 256);
    f.read(str, 255);
    f.close();

    return str;
}

int parse_user_config(const char* filename, UserConfig* config) {

    char* json_string = read_file(filename);
    if (!json_string)
        return 0;

    cJSON* root = cJSON_Parse(json_string);
    if (!root) {
        delete[] json_string;
        return 0;
    }

    cJSON* flashStopTime = cJSON_GetObjectItemCaseSensitive(root, "FlashStopTime");
    if (cJSON_IsNumber(flashStopTime)) {
        config->FlashStopTime = flashStopTime->valueint;
    }

    cJSON* knifeStopTime = cJSON_GetObjectItemCaseSensitive(root, "KnifeTime");
    if (cJSON_IsNumber(knifeStopTime)) {
        config->KnifeTime = knifeStopTime->valueint;
    }

    cJSON* funcSwitch = cJSON_GetObjectItemCaseSensitive(root, "FuncSwitch");
    if (funcSwitch) {
        cJSON* closeAce = cJSON_GetObjectItemCaseSensitive(funcSwitch, "CloseAce");
        if (cJSON_IsBool(closeAce)) config->FuncSwitch.CloseAce = cJSON_IsTrue(closeAce);

        cJSON* muteWindow = cJSON_GetObjectItemCaseSensitive(funcSwitch, "MuteWindow");
        if (cJSON_IsBool(muteWindow)) config->FuncSwitch.MuteWindow = cJSON_IsTrue(muteWindow);

        cJSON* autoKnife = cJSON_GetObjectItemCaseSensitive(funcSwitch, "AutoKnife");
        if (cJSON_IsBool(autoKnife)) config->FuncSwitch.AutoKnife = cJSON_IsTrue(autoKnife);

        cJSON* stopBreath = cJSON_GetObjectItemCaseSensitive(funcSwitch, "StopBreath");
        if (cJSON_IsBool(stopBreath)) config->FuncSwitch.StopBreath = cJSON_IsTrue(stopBreath);

        cJSON* slide = cJSON_GetObjectItemCaseSensitive(funcSwitch, "Slide");
        if (cJSON_IsBool(slide)) config->FuncSwitch.Slide = cJSON_IsTrue(slide);

        cJSON* flash = cJSON_GetObjectItemCaseSensitive(funcSwitch, "Flash");
        if (cJSON_IsBool(flash)) config->FuncSwitch.Flash = cJSON_IsTrue(flash);

    }

    cJSON_Delete(root);
    delete[] json_string;

    return 1;
}