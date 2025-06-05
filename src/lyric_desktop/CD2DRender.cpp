#include "CD2DRender.h"
#include <atlbase.h>

KUODAFU_NAMESPACE_BEGIN
LPWSTR prefixstring(LPCWSTR text, DWORD format, size_t*& nPreFix, int& count);

struct BitmapData
{
    BYTE* pixels = nullptr;
    UINT width = 0;
    UINT height = 0;
    UINT stride = 0;
};

BitmapData* ExtractPixelsFromD2D(ID2D1DeviceContext* pRenderTarget)
{
    if (!pRenderTarget)
        return nullptr;

    CComPtr<ID2D1Image> pImage;
    pRenderTarget->GetTarget(&pImage);
    if (!pImage)
        return nullptr;

    CComPtr<ID2D1Bitmap1> pBitmap;
    HRESULT hr = pImage->QueryInterface(&pBitmap);
    if (FAILED(hr) || !pBitmap)
        return nullptr;

    D2D1_SIZE_U size = pBitmap->GetPixelSize();

    CComPtr<IDXGISurface> dxgiSurface;
    hr = pBitmap->GetSurface(&dxgiSurface);
    if (FAILED(hr))
        return nullptr;

    CComPtr<ID3D11Texture2D> d3dTexture;
    hr = dxgiSurface->QueryInterface(&d3dTexture);
    if (FAILED(hr))
        return nullptr;

    CComPtr<ID3D11Device> d3dDevice;
    d3dTexture->GetDevice(&d3dDevice);

    CComPtr<ID3D11DeviceContext> d3dContext;
    d3dDevice->GetImmediateContext(&d3dContext);

    D3D11_TEXTURE2D_DESC desc;
    d3dTexture->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    CComPtr<ID3D11Texture2D> stagingTexture;
    hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(hr))
        return nullptr;

    d3dContext->CopyResource(stagingTexture, d3dTexture);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = d3dContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr))
        return nullptr;

    UINT width = desc.Width;
    UINT height = desc.Height;
    UINT stride = width * 4;

    BitmapData* bmpData = new BitmapData;
    bmpData->pixels = new BYTE[stride * height];
    bmpData->width = width;
    bmpData->height = height;
    bmpData->stride = stride;

    BYTE* pSrc = (BYTE*)mapped.pData;
    BYTE* pDst = bmpData->pixels;

    for (UINT y = 0; y < height; ++y)
    {
        memcpy(pDst + y * stride, pSrc + y * mapped.RowPitch, stride);
    }

    d3dContext->Unmap(stagingTexture, 0);

    return bmpData;
}

HDC CreateHDCFromD2D(ID2D1DeviceContext* pRenderTarget)
{
    BitmapData* bmpData = ExtractPixelsFromD2D(pRenderTarget);
    if (!bmpData)
        return nullptr;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    ReleaseDC(nullptr, hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmpData->width;
    bmi.bmiHeader.biHeight = -(LONG)bmpData->height; // top-down bitmap
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    if (!hBitmap)
    {
        delete[] bmpData->pixels;
        delete bmpData;
        DeleteDC(hdcMem);
        return nullptr;
    }

    SelectObject(hdcMem, hBitmap);
    memcpy(pBits, bmpData->pixels, bmpData->stride * bmpData->height);

    delete[] bmpData->pixels;
    delete bmpData;

    return hdcMem;
}

bool CopyD2DRenderTargetToHDC(ID2D1DeviceContext* pRenderTarget, HDC hdc)
{
    if (!hdc)
        return false;

    BitmapData* bmpData = ExtractPixelsFromD2D(pRenderTarget);
    if (!bmpData)
        return false;

    HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
    if (!hBitmap)
    {
        delete[] bmpData->pixels;
        delete bmpData;
        return false;
    }

    BITMAP bmpInfo;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmpInfo) == 0)
    {
        delete[] bmpData->pixels;
        delete bmpData;
        return false;
    }

    if ((UINT)bmpInfo.bmWidth != bmpData->width || (UINT)abs(bmpInfo.bmHeight) != bmpData->height)
    {
        delete[] bmpData->pixels;
        delete bmpData;
        return false;
    }

    int rowBytes = bmpData->width * 4;
    int heightAbs = abs(bmpInfo.bmHeight);

    if (bmpInfo.bmHeight > 0)
    {
        // bottom-up bitmap
        BYTE* pDst = new BYTE[rowBytes * heightAbs];
        for (int y = 0; y < heightAbs; ++y)
        {
            memcpy(pDst + (heightAbs - 1 - y) * rowBytes, bmpData->pixels + y * rowBytes, rowBytes);
        }

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = bmpInfo.bmWidth;
        bmi.bmiHeader.biHeight = bmpInfo.bmHeight;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        BOOL bRet = SetDIBits(hdc, hBitmap, 0, heightAbs, pDst, &bmi, DIB_RGB_COLORS);
        delete[] pDst;
        delete[] bmpData->pixels;
        delete bmpData;
        return bRet != FALSE;
    }
    else
    {
        // top-down bitmap
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = bmpInfo.bmWidth;
        bmi.bmiHeader.biHeight = bmpInfo.bmHeight;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        BOOL bRet = SetDIBits(hdc, hBitmap, 0, heightAbs, bmpData->pixels, &bmi, DIB_RGB_COLORS);

        delete[] bmpData->pixels;
        delete bmpData;
        return bRet != FALSE;
    }
}



static HRESULT _d2d_create_bitmap(ID2D1DeviceContext* pRenderTarget, int width, int height, ID2D1Bitmap1** ppBitmap)
{
    D2D1_BITMAP_PROPERTIES1 d2dbp = {};
    d2dbp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    d2dbp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    d2dbp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    d2dbp.dpiX = 96;
    d2dbp.dpiY = 96;

    D2D1_SIZE_U size = { (UINT)width, (UINT)height };
    return pRenderTarget->CreateBitmap(size, nullptr, 0, d2dbp, ppBitmap);
}

CD2DRender::CD2DRender(int width, int height)
    : m_pRenderTarget(nullptr)
    , m_pBitmap(nullptr)
    , m_pGDIInterop(nullptr)
{
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    auto& d2dInfo = d2d_get_info();
    HRESULT hr = 0;

    hr = d2dInfo.pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pRenderTarget);
    if (FAILED(hr))
        return;

    hr = m_pRenderTarget->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&m_pGDIInterop);
    if (FAILED(hr))
    {
        SafeRelease(m_pRenderTarget);
        return;
    }
    hr = _d2d_create_bitmap(m_pRenderTarget, width, height, &m_pBitmap);
    m_pRenderTarget->SetTarget(m_pBitmap);

}

CD2DRender::~CD2DRender()
{
    SafeRelease(m_pBitmap);
    SafeRelease(m_pGDIInterop);
    SafeRelease(m_pRenderTarget);
}


bool CD2DRender::getsize(int* width, int* height)
{
    if (!m_pBitmap)
        return false;
    D2D1_SIZE_U si = m_pBitmap->GetPixelSize();
    if (width) *width = si.width;
    if (height) *height = si.height;
    return true;
}
bool CD2DRender::resize(int width, int height)
{
    if (!m_pBitmap)
        return false;

    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    D2D1_SIZE_U si = m_pBitmap ? m_pBitmap->GetPixelSize() : D2D1::SizeU(0, 0);
    if (si.width == (UINT)width && si.height == (UINT)height)
        return true;

    m_pRenderTarget->SetTarget(nullptr);
    SafeRelease(m_pBitmap);

    HRESULT hr = _d2d_create_bitmap(m_pRenderTarget, width, height, &m_pBitmap);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        __debugbreak();
#endif
        return false;
    }
    m_pRenderTarget->SetTarget(m_pBitmap);
    return true;
}
//
//bool CD2DRender::calc_text(CD2DFont* pFont, LPCWSTR text, size_t textLen, DWORD textFormat, LPDRAWTEXTPARAMS lParam, float layoutWidth, float layoutHeight, float* pWidth, float* pHeight, IDWriteTextLayout** ppDWriteTextLayout)
//{
//    if (layoutWidth < 0)layoutWidth = 0;
//    if (layoutHeight < 0)layoutHeight = 0;
//
//    CD2DFont& font = *pFont;
//
//    size_t* nPreFix = 0;
//    int nPreFixCount = 0;
//    LPWSTR lpwzTextFix = prefixstring(text, textFormat, nPreFix, nPreFixCount);
//    HRESULT hr = 0;
//
//    IDWriteInlineObject* pWriteInlineObject = font;
//    IDWriteTextFormat* dxFormat = font;
//
//
//    auto& d2dInfo = d2d_get_info();
//    IDWriteTextLayout* pDWriteTextLayout = 0;
//    hr = d2dInfo.pDWriteFactory->CreateTextLayout(lpwzTextFix ? lpwzTextFix : text,
//                                                  (UINT32)textLen, dxFormat, layoutWidth, layoutHeight, &pDWriteTextLayout);
//    if (ppDWriteTextLayout)
//        *ppDWriteTextLayout = pDWriteTextLayout;
//
//    //CComPtr<IDWriteTextLayout1> pTextLayout1 = nullptr;
//    //hr = pDWriteTextLayout->QueryInterface(__uuidof(IDWriteTextLayout1), reinterpret_cast<void**>(&pTextLayout1));
//
//    //if (pTextLayout1)
//    //{
//    //    DWRITE_TEXT_RANGE range = { 0, (UINT32)textLen };
//    //    hr = pTextLayout1->SetCharacterSpacing(
//    //        10.0f,  // 前导间距 (字符前添加的空间)
//    //        5.0f,   // 尾随间距 (字符后添加的空间)
//    //        20.0f,  // 最小前进宽度 (字符的最小总宽度)
//    //        range);
//    //}
//
//    if (FAILED(hr))
//        return false;
//    LOGFONTW& lf = font;
//
//    if (lf.lfUnderline)
//    {
//        DWRITE_TEXT_RANGE tmp{};
//        tmp.length = (UINT32)textLen;
//        tmp.startPosition = 0;
//        pDWriteTextLayout->SetUnderline(lf.lfUnderline, tmp);
//    }
//    if (lf.lfStrikeOut)
//    {
//        DWRITE_TEXT_RANGE tmp{};
//        tmp.length = (UINT32)textLen;
//        tmp.startPosition = 0;
//        pDWriteTextLayout->SetStrikethrough(lf.lfStrikeOut, tmp);
//    }
//    pDWriteTextLayout->SetWordWrapping(
//        (textFormat & DT_SINGLELINE)
//        ? DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_NO_WRAP
//        : DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_WRAP);
//    if ((textFormat & DT_PATH_ELLIPSIS) || (textFormat & DT_WORD_ELLIPSIS) || (textFormat & DT_END_ELLIPSIS))
//    {
//        DWRITE_TRIMMING tmp{};
//        if ((textFormat & DT_END_ELLIPSIS))
//        {
//            tmp.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
//            tmp.delimiter = 0;
//            tmp.delimiterCount = 0;
//        }
//        else if ((textFormat & DT_WORD_ELLIPSIS))
//        {
//            tmp.granularity = DWRITE_TRIMMING_GRANULARITY_WORD;
//        }
//        if (SUCCEEDED(hr))
//        {
//            pDWriteTextLayout->SetTrimming(&tmp, pWriteInlineObject);
//        }
//    }
//    DWRITE_TEXT_METRICS t = { 0 };
//    pDWriteTextLayout->GetMetrics(&t);
//    if (pWidth)*pWidth = t.widthIncludingTrailingWhitespace;
//    if (pHeight)*pHeight = t.height;
//    DWRITE_TEXT_ALIGNMENT textAlignment;
//    if (textFormat & DT_CENTER)
//        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
//    else if (textFormat & DT_RIGHT)
//        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING;
//    else
//        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
//    pDWriteTextLayout->SetTextAlignment(textAlignment);
//    DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment;
//    if (textFormat & DT_VCENTER)
//        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
//    else if (textFormat & DT_BOTTOM)
//        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_FAR;
//    else
//        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
//    pDWriteTextLayout->SetParagraphAlignment(paragraphAlignment);
//    if (nPreFix)
//    {
//        DWRITE_TEXT_RANGE tmp{};
//        tmp.length = 1;
//        tmp.startPosition = (UINT32)nPreFix[0];
//        pDWriteTextLayout->SetUnderline(TRUE, tmp);
//    }
//    if (lpwzTextFix)
//    {
//        delete[] nPreFix;
//        delete[] lpwzTextFix;
//    }
//    if (!ppDWriteTextLayout)
//        pDWriteTextLayout->Release();
//    return SUCCEEDED(hr);
//}
//

LPWSTR prefixstring(LPCWSTR text, DWORD format, size_t*& nPreFix, int& count)
{
    wchar_t* ret = 0;
    if ((format & DT_NOPREFIX) == 0 && text)
    {
        LPCWSTR lpOffset = wcschr(text, '&');
        if (lpOffset)
        {
            size_t len = wcslen(text) + 1;
            size_t nOffset = lpOffset - text;

            ret = new wchar_t[len];
            memset(ret, 0, sizeof(wchar_t) * len);
            count = (int)len;
            nPreFix = new size_t[len];
            memset(nPreFix, 0, sizeof(size_t) * len);
            memcpy(ret, text, nOffset * sizeof(wchar_t));
            memcpy(ret + nOffset, lpOffset + 1, (len - nOffset - 2) * sizeof(wchar_t));
            nPreFix[0] = nOffset;
            return ret;
            count = 0;
            int i = 0, n = 0;
            while (text[n])
            {
                if (text[n] == '&' && count == 0)
                    nPreFix[count++] = n;
                else
                    ret[i++] = text[n];
                n++;
            }
        }

    }
    return (LPWSTR)ret;
}


KUODAFU_NAMESPACE_END

