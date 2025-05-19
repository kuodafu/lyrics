#pragma once
#include "CD2DRender.h"

NAMESPACE_D2D_BEGIN

typedef struct EX_IMAGELOCK
{
    UINT width;
    UINT height;
    UINT stride;
    void* pScan0;
    IWICBitmapLock* pLock;
}*PEX_IMAGELOCK, * LPEX_IMAGELOCK;

class CD2DImage
{
    IWICBitmapDecoder*  m_pDecoder;     // WICλͼ������
    IWICBitmap*         m_pFrame;       // λͼ֡
    ID2D1Bitmap1*       m_pBitmap;      // ����һ��, ����ʱ��Ҫ�ж��Ƿ�Ϊ��Ȼ������

;
public:
    CD2DImage(CD2DRender& d2dRender, const void* pData, size_t len);
    CD2DImage(CD2DRender& d2dRender, IStream* stream);
    ~CD2DImage();

public:
    ID2D1Bitmap1* GetBitmap(CD2DRender& d2dRender, HRESULT* phr = 0);

    // ����ͼƬ����
    // flags = 1=��, 2=д, 3=��д
    bool lock(UINT x, UINT y, UINT width, UINT height, DWORD flags, EX_IMAGELOCK* lockData);

    // ȡ������ͼƬ
    bool unlock(EX_IMAGELOCK* lockData);


private:
    void _img_create_fromstream(IStream* stream);

};


NAMESPACE_D2D_END
