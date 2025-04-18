#include "CD2DRender.h"
NAMESPACE_D2D_BEGIN
LPWSTR prefixstring(LPCWSTR text, DWORD format, size_t*& nPreFix, int& count);

CD2DRender::CD2DRender(int width, int height):
    m_pRenderTarget(nullptr),
    m_pBitmap(nullptr),
    m_pGDIInterop(nullptr)
{
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    auto& d2dInfo = d2d_get_info();
    HRESULT hr = 0;
    if (m_pRenderTarget == 0)
    {
        hr = d2dInfo.pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                                             &m_pRenderTarget);
        if (FAILED(hr))
            return;
        m_pRenderTarget->QueryInterface(IID_PPV_ARGS(&m_pGDIInterop));
    }

    d2dInfo.bp_proper.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    D2D1_SIZE_U size = { (UINT)width, (UINT)height };
    hr = m_pRenderTarget->CreateBitmap(size, 0, 0, &d2dInfo.bp_proper, &m_pBitmap);
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

    D2D1_SIZE_U si = m_pBitmap->GetPixelSize();
    if (si.width == (UINT)width && si.height == (UINT)height)
        return true;
    SafeRelease(m_pBitmap);

    auto& d2dInfo = d2d_get_info();
    d2dInfo.bp_proper.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    D2D1_SIZE_U size = { (UINT)width, (UINT)height };
    HRESULT hr = m_pRenderTarget->CreateBitmap(size, 0, 0, &d2dInfo.bp_proper, &m_pBitmap);
    m_pRenderTarget->SetTarget(m_pBitmap);
    return SUCCEEDED(hr);
}

bool CD2DRender::calc_text(CD2DFont* pFont, LPCWSTR text, size_t textLen, DWORD textFormat, LPDRAWTEXTPARAMS lParam, float layoutWidth, float layoutHeight, float* pWidth, float* pHeight, IDWriteTextLayout** ppDWriteTextLayout)
{
    if (layoutWidth < 0)layoutWidth = 0;
    if (layoutHeight < 0)layoutHeight = 0;

    CD2DFont& font = *pFont;

    size_t* nPreFix = 0;
    int nPreFixCount = 0;
    LPWSTR lpwzTextFix = prefixstring(text, textFormat, nPreFix, nPreFixCount);
    HRESULT hr = 0;

    IDWriteInlineObject* pWriteInlineObject = font;
    IDWriteTextFormat* dxFormat = font;


    auto& d2dInfo = d2d_get_info();
    IDWriteTextLayout* pDWriteTextLayout = 0;
    hr = d2dInfo.pDWriteFactory->CreateTextLayout(lpwzTextFix ? lpwzTextFix : text,
                                                  (UINT32)textLen, dxFormat, layoutWidth, layoutHeight, &pDWriteTextLayout);
    if (ppDWriteTextLayout)
        *ppDWriteTextLayout = pDWriteTextLayout;


    if (FAILED(hr))
        return false;
    LOGFONTW& lf = font;

    if (lf.lfUnderline)
    {
        DWRITE_TEXT_RANGE tmp{};
        tmp.length = (UINT32)textLen;
        tmp.startPosition = 0;
        pDWriteTextLayout->SetUnderline(lf.lfUnderline, tmp);
    }
    if (lf.lfStrikeOut)
    {
        DWRITE_TEXT_RANGE tmp{};
        tmp.length = (UINT32)textLen;
        tmp.startPosition = 0;
        pDWriteTextLayout->SetStrikethrough(lf.lfStrikeOut, tmp);
    }
    pDWriteTextLayout->SetWordWrapping(
        (textFormat & DT_SINGLELINE)
        ? DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_NO_WRAP
        : DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_WRAP);
    if ((textFormat & DT_PATH_ELLIPSIS) || (textFormat & DT_WORD_ELLIPSIS) || (textFormat & DT_END_ELLIPSIS))
    {
        DWRITE_TRIMMING tmp{};
        if ((textFormat & DT_END_ELLIPSIS))
        {
            tmp.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
            tmp.delimiter = 0;
            tmp.delimiterCount = 0;
        }
        else if ((textFormat & DT_WORD_ELLIPSIS))
        {
            tmp.granularity = DWRITE_TRIMMING_GRANULARITY_WORD;
        }
        if (SUCCEEDED(hr))
        {
            pDWriteTextLayout->SetTrimming(&tmp, pWriteInlineObject);
        }
    }
    DWRITE_TEXT_METRICS t = { 0 };
    pDWriteTextLayout->GetMetrics(&t);
    if (pWidth)*pWidth = t.widthIncludingTrailingWhitespace;
    if (pHeight)*pHeight = t.height;
    DWRITE_TEXT_ALIGNMENT textAlignment;
    if (textFormat & DT_CENTER)
        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
    else if (textFormat & DT_RIGHT)
        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_TRAILING;
    else
        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
    pDWriteTextLayout->SetTextAlignment(textAlignment);
    DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment;
    if (textFormat & DT_VCENTER)
        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    else if (textFormat & DT_BOTTOM)
        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_FAR;
    else
        paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
    pDWriteTextLayout->SetParagraphAlignment(paragraphAlignment);
    if (nPreFix)
    {
        DWRITE_TEXT_RANGE tmp{};
        tmp.length = 1;
        tmp.startPosition = (UINT32)nPreFix[0];
        pDWriteTextLayout->SetUnderline(TRUE, tmp);
    }
    if (lpwzTextFix)
    {
        delete[] nPreFix;
        delete[] lpwzTextFix;
    }
    if (!ppDWriteTextLayout)
        pDWriteTextLayout->Release();
    return SUCCEEDED(hr);
}


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


NAMESPACE_D2D_END

