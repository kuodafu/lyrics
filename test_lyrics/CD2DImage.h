#pragma once
#include "d2d.h"

NAMESPACE_D2D_BEGIN

class CD2DImage
{
    IWICBitmapDecoder*  m_pDecoder;     // WICλͼ������
    IWICBitmap*         m_pFrame;       // λͼ֡
    ID2D1Bitmap1*       m_pBitmap;      // ����һ��, ����ʱ��Ҫ�ж��Ƿ�Ϊ��Ȼ������

;
public:
    CD2DImage(const void* pData, size_t len);
    CD2DImage(IStream* stream);
    ~CD2DImage();

public:
    ID2D1Bitmap1* GetBitmap(HRESULT* phr = 0);


private:
    void _img_create_fromstream(IStream* stream);

};


NAMESPACE_D2D_END
