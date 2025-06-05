#pragma once
#include "CD2DRender.h"

KUODAFU_NAMESPACE_BEGIN

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
    IWICBitmapDecoder*  m_pDecoder;     // WIC位图解码器
    IWICBitmap*         m_pFrame;       // 位图帧
    ID2D1Bitmap1*       m_pBitmap;      // 创建一次, 销毁时需要判断是否为空然后销毁

public:
    CD2DImage(CD2DRender& d2dRender, const void* pData, size_t len);
    CD2DImage(CD2DRender& d2dRender, IStream* stream);
    ~CD2DImage();

public:
    ID2D1Bitmap1* GetBitmap(CD2DRender& d2dRender, HRESULT* phr = nullptr);

    // 锁定图片数据
    // flags = 1=读, 2=写, 3=读写
    bool lock(UINT x, UINT y, UINT width, UINT height, DWORD flags, EX_IMAGELOCK* lockData);

    // 取消锁定图片
    bool unlock(EX_IMAGELOCK* lockData);


private:
    void _img_create_fromstream(IStream* stream);

};


KUODAFU_NAMESPACE_END
