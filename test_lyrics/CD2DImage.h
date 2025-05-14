#pragma once
#include "CD2DRender.h"

NAMESPACE_D2D_BEGIN

class CD2DImage
{
    IWICBitmapDecoder*  m_pDecoder;     // WIC位图解码器
    IWICBitmap*         m_pFrame;       // 位图帧
    ID2D1Bitmap1*       m_pBitmap;      // 创建一次, 销毁时需要判断是否为空然后销毁

;
public:
    CD2DImage(CD2DRender& d2dRender, const void* pData, size_t len);
    CD2DImage(CD2DRender& d2dRender, IStream* stream);
    ~CD2DImage();

public:
    ID2D1Bitmap1* GetBitmap(CD2DRender& d2dRender, HRESULT* phr = 0);


private:
    void _img_create_fromstream(IStream* stream);

};


NAMESPACE_D2D_END
