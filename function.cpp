
#include "global.h"

void MBX(const char* str)
{
    MessageBoxA(NULL, str, "��ʾ", MB_OK);
};

void MBX(std::string str)
{
    MessageBoxA(NULL, str.c_str(), "��ʾ", MB_OK);
};

void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{

    // ������ʼ��
    DWORD* dst = GetImageBuffer(dstimg);
    DWORD* src = GetImageBuffer(srcimg);
    int src_width = srcimg->getwidth();
    int src_height = srcimg->getheight();
    int dst_width = (dstimg == NULL ? getwidth() : dstimg->getwidth());
    int dst_height = (dstimg == NULL ? getheight() : dstimg->getheight());

    // ������ͼ��ʵ�ʳ���
    int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;		// �������ұ߽�
    int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;	// �������±߽�
    if (x < 0) { src += -x;				iwidth -= -x;	x = 0; }				// ��������߽�
    if (y < 0) { src += src_width * -y;	iheight -= -y;	y = 0; }				// �������ϱ߽�

    // ������ͼ��ʼλ��
    dst += dst_width * y + x;

    // ʵ��͸����ͼ

    for (int iy = 0; iy < iheight; ++iy)
    {
        for (int i = 0; i < iwidth; ++i)

        {
            int sa = ((src[i] & 0xff000000) >> 24);//��ȡ������ֵ
            if (sa != 0)//��������ȫ͸���Ͳ�����
                if (sa == 255)//������ȫ��͸����ֱ�ӿ���
                    dst[i] = src[i];
                else//������Ҫ��������ϼ����ͼ��߽�Ž��л��
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