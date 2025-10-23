#pragma once
#pragma warning(disable:4996)

#include "windows.h"
#include "iostream"
#include "graphics.h"
#include "process.h"
#include "string"

void MBX(const char* str);
void MBX(std::string str);

void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg);

LPWSTR pCharToLPWSTR(char* old);

void PressKey(UINT Key);