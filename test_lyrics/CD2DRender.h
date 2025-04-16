#pragma once
#include "d2d.h"
#include "CD2DFont.h"
#include "CD2DBrush.h"

NAMESPACE_D2D_BEGIN

class CD2DRender
{
    ID2D1Bitmap1* m_pBitmap;  // d2dλͼ
    ID2D1GdiInteropRenderTarget* m_pGDIInterop;   // d2d����gdi����, ͨ����������ȡHDC
    ID2D1DeviceContext* m_pRenderTarget;  // d2d�豸������, d2dʹ��������л滭, ��������� pBitmap ���λͼ��

public:
    CD2DRender(int width, int height);
    ~CD2DRender();

    ID2D1DeviceContext* operator->() const { return m_pRenderTarget; }
    operator ID2D1GdiInteropRenderTarget*() const { return m_pGDIInterop; }
    operator ID2D1Bitmap1*() const { return m_pBitmap; }
    operator ID2D1DeviceContext*() const { return m_pRenderTarget; }


    bool resize(int width, int height);

    bool calc_text(CD2DFont* font, LPCWSTR text, size_t textLen,
                   DWORD textFormat, LPDRAWTEXTPARAMS lParam, float layoutWidth, float layoutHeight,
                   float* pWidth, float* pHeight, IDWriteTextLayout** ppDWriteTextLayout);
};


NAMESPACE_D2D_END
