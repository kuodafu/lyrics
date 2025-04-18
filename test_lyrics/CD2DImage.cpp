#include "CD2DImage.h"

NAMESPACE_D2D_BEGIN

static inline IStream* _img_createfromstream_init(const void* pData, size_t len)
{
    if (!len || !pData) return 0;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem)
    {
        IStream* stream;
        HRESULT hr = CreateStreamOnHGlobal(hMem, TRUE, &stream);
        if (SUCCEEDED(hr))
        {
            void* p = GlobalLock(hMem);
            if (p)
            {
                memcpy(p, pData, len);
                GlobalUnlock(hMem);
                return stream;
            }
        }
        GlobalFree(hMem);
    }
    return 0;
}

static IWICBitmap* _wic_convert(IWICBitmapFrameDecode*& pBitmap, bool bFreeOld)
{
    IWICBitmap* pBitmapConvert = (IWICBitmap*)pBitmap;
    CComPtr<IWICFormatConverter> pConverter;    // 位图格式转换
    auto& d2dInfo = d2d_get_info();
    // 创建位图格式转换
    HRESULT hr = d2dInfo.pWICFactory->CreateFormatConverter(&pConverter);
    Assert(SUCCEEDED(hr));
    if (SUCCEEDED(hr))
    {
        hr = pConverter->Initialize(pBitmap, GUID_WICPixelFormat32bppPBGRA,
                                    WICBitmapDitherType::WICBitmapDitherTypeNone, 0, 0.0,
                                    WICBitmapPaletteType::WICBitmapPaletteTypeCustom);
        if (SUCCEEDED(hr))    // 
        {
            // 创建位图自源
            d2dInfo.pWICFactory->CreateBitmapFromSource(pConverter,
                                                        WICBitmapCreateCacheOption::WICBitmapCacheOnDemand, &pBitmapConvert);
            if (SUCCEEDED(hr) && bFreeOld)
            {
                SafeRelease(pBitmap);
            }
        }
    }
    return pBitmapConvert;
}

static IWICBitmap* _wic_selectactiveframe(IWICBitmapDecoder* pDecoder, UINT index)
{
    IWICBitmapFrameDecode* pFrame = 0;
    HRESULT hr = pDecoder->GetFrame(index, &pFrame);
    IWICBitmap* ret = 0;
    if (SUCCEEDED(hr))
        ret = _wic_convert(pFrame, true);
    return ret;
}

static void _wic_drawframe(IWICBitmap* pWICBitmap, UINT maxFrames)
{
    if (maxFrames <= 1)
        return;
    auto& d2dInfo = d2d_get_info();
    D2D1_RENDER_TARGET_PROPERTIES rtb;
    memset(&rtb, 0, sizeof(rtb));
    CComPtr<ID2D1RenderTarget> rt;
    CComPtr<ID2D1Bitmap> pBitmap;

    HRESULT hr = d2dInfo.pFactory->CreateWicBitmapRenderTarget(pWICBitmap, rtb, &rt);
    Assert(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        rt->BeginDraw();
        hr = rt->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap);
        if (SUCCEEDED(hr) && pBitmap)
        {
            D2D1_RECT_F rc = { 0,1,1,0 };
            rt->DrawBitmap(pBitmap, rc);
        }
        hr = rt->EndDraw();
        Assert(SUCCEEDED(hr));
    }
}

static IWICBitmap* _wic_init_from_decoder(IWICBitmapDecoder* pDecoder)
{

    UINT pCount = 0;
    HRESULT hr = pDecoder->GetFrameCount(&pCount);
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
            IWICBitmap* pFrame = _wic_selectactiveframe(pDecoder, 0);
            if (pFrame)
            {
                _wic_drawframe(pFrame, pCount);
                return pFrame;
            }
        }
    }
    Assert(SUCCEEDED(hr));
    return nullptr;
}


CD2DImage::CD2DImage(const void* pData, size_t len): m_pDecoder(nullptr), m_pFrame(nullptr), m_pBitmap(nullptr)
{
    IStream* stream = _img_createfromstream_init(pData, len);
    if (!stream)
        return;

    _img_create_fromstream(stream);
}

CD2DImage::CD2DImage(IStream* stream) : m_pDecoder(nullptr), m_pFrame(nullptr), m_pBitmap(nullptr)
{
    _img_create_fromstream(stream);
}

CD2DImage::~CD2DImage()
{
    SafeRelease(m_pFrame);
    SafeRelease(m_pDecoder);
    SafeRelease(m_pBitmap);
}

ID2D1Bitmap1* CD2DImage::GetBitmap(HRESULT* phr)
{
    if (!m_pBitmap)
    {
        auto& d2dInfo = d2d_get_info();
        HRESULT hr = d2dInfo.pD2DDeviceContext->CreateBitmapFromWicBitmap(m_pFrame, &d2dInfo.bp_proper, &m_pBitmap);
        if (phr)
            *phr = hr;
    }
    return m_pBitmap;
}

void CD2DImage::_img_create_fromstream(IStream* stream)
{
    auto& d2dInfo = d2d_get_info();
    // 从流里创建WIC位图解码器
    HRESULT hr = d2dInfo.pWICFactory->CreateDecoderFromStream(stream, 0,
                                                              WICDecodeOptions::WICDecodeMetadataCacheOnLoad,
                                                              &m_pDecoder);
    if (FAILED(hr))
        return;

    m_pFrame = _wic_init_from_decoder(m_pDecoder);

}


NAMESPACE_D2D_END


