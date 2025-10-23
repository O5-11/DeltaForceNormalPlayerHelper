
#include "global.h"

void MBX(const char* str)
{
    MessageBoxA(NULL, str, "提示", MB_OK);
};

void MBX(std::string str)
{
    MessageBoxA(NULL, str.c_str(), "提示", MB_OK);
};

void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{

    // 变量初始化
    DWORD* dst = GetImageBuffer(dstimg);
    DWORD* src = GetImageBuffer(srcimg);
    int src_width = srcimg->getwidth();
    int src_height = srcimg->getheight();
    int dst_width = (dstimg == NULL ? getwidth() : dstimg->getwidth());
    int dst_height = (dstimg == NULL ? getheight() : dstimg->getheight());

    // 计算贴图的实际长宽
    int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;		// 处理超出右边界
    int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;	// 处理超出下边界
    if (x < 0) { src += -x;				iwidth -= -x;	x = 0; }				// 处理超出左边界
    if (y < 0) { src += src_width * -y;	iheight -= -y;	y = 0; }				// 处理超出上边界

    // 修正贴图起始位置
    dst += dst_width * y + x;

    // 实现透明贴图

    for (int iy = 0; iy < iheight; ++iy)
    {
        for (int i = 0; i < iwidth; ++i)

        {
            int sa = ((src[i] & 0xff000000) >> 24);//获取阿尔法值
            if (sa != 0)//假如是完全透明就不处理
                if (sa == 255)//假如完全不透明则直接拷贝
                    dst[i] = src[i];
                else//真正需要阿尔法混合计算的图像边界才进行混合
                    dst[i] = ((((src[i] & 0xff0000) >> 16) + ((dst[i] & 0xff0000) >> 16) * (255 - sa) / 255) << 16) | ((((src[i] & 0xff00) >> 8) + ((dst[i] & 0xff00) >> 8) * (255 - sa) / 255) << 8) | ((src[i] & 0xff) + (dst[i] & 0xff) * (255 - sa) / 255);
        }
        dst += dst_width;
        src += src_width;
    }
}

LPWSTR pCharToLPWSTR(char* old)
{
    int len = strlen(old) + 4;
    TCHAR* nold = new TCHAR[len]; memset(nold, 0, len);
    int num = MultiByteToWideChar(0, 0, old, -1, NULL, 0);
    MultiByteToWideChar(CP_ACP, 0, old, -1, nold, num);
    return nold;
};

void PressKey(UINT Key) {
    keybd_event(Key, 0, 0, 0);
    Sleep(1);
    keybd_event(Key, 0, 2, 0);
}