#include "CD2DBrush.h"
#include <vector>

NAMESPACE_D2D_BEGIN

CD2DBrush::CD2DBrush(CD2DRender& d2dRender, DWORD argb): m_pBrush(nullptr)
{
    ID2D1DeviceContext* pD2DDeviceContext = d2dRender;
    HRESULT hr = pD2DDeviceContext->CreateSolidColorBrush(ARGB_D2D::toDx(argb),
                                                          D2D1::BrushProperties(),
                                                          &m_pBrush);
}


CD2DBrush::~CD2DBrush()
{
    SafeRelease(m_pBrush);
}

bool CD2DBrush::SetColor(DWORD argb)
{
    if (!m_pBrush)
        return false;
    m_pBrush->SetColor(ARGB_D2D::toDx(argb));
    return true;
}

DWORD CD2DBrush::GetColor(DWORD argb)
{
    if (!m_pBrush)
        return false;
    D2D1_COLOR_F clr = m_pBrush->GetColor();
    return ARGB_D2D::toArgb(clr);
}




CD2DBrush_LinearGradient::CD2DBrush_LinearGradient(CD2DRender& d2dRender, const POINT_F& pt1, const POINT_F& pt2, ARGB* color, DWORD colorCount, int fillMode, const float* pRatios, DWORD ratiosCount) : m_pBrush(nullptr)
{
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES pt{};
    memcpy(&pt.startPoint, &pt1, sizeof(pt.startPoint));
    memcpy(&pt.endPoint, &pt2, sizeof(pt.endPoint));

    HRESULT hr = S_OK;

    std::vector<D2D1_GRADIENT_STOP> gradientStops(colorCount);
    const float* ratios = nullptr;
    if (pRatios && ratiosCount == colorCount)
        ratios = pRatios;

    for (DWORD i = 0; i < colorCount; i++)
    {
        gradientStops[i].color = ARGB_D2D::toDx(color[i]);
        if (ratios)
            gradientStops[i].position = ratios[i];
        else
            gradientStops[i].position = ((float)i) / (((float)colorCount) - 1.0f);	// 0-1
    }

    CComPtr<ID2D1GradientStopCollection> pGradientStops;
    ID2D1DeviceContext* pD2DDeviceContext = d2dRender;

    hr = pD2DDeviceContext->CreateGradientStopCollection(
        &gradientStops[0], colorCount,
        D2D1_GAMMA_2_2,
        (D2D1_EXTEND_MODE)fillMode,
        &pGradientStops);


    if (FAILED(hr))
        return;

    D2D1_BRUSH_PROPERTIES props{};
    props.opacity = 1.0f;
    props.transform._11 = 1.0f;
    props.transform._22 = 1.0f;
    hr = pD2DDeviceContext->CreateLinearGradientBrush(pt, props, pGradientStops, &m_pBrush);

}

CD2DBrush_LinearGradient::~CD2DBrush_LinearGradient()
{
    SafeRelease(m_pBrush);
}

CD2DBrush_RadialGradient::CD2DBrush_RadialGradient(CD2DRender& d2dRender, const ELLIPSE_F* pos, ARGB* color, DWORD colorCount, int fillMode, const float* pRatios, DWORD ratiosCount) : m_pBrush(nullptr)
{

    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES pt;
    pt.center.x = pos->x;
    pt.center.y = pos->y;
    pt.gradientOriginOffset.x = 0;
    pt.gradientOriginOffset.y = 0;
    pt.radiusX = pos->radiusX;
    pt.radiusY = pos->radiusY;
    std::vector<D2D1_GRADIENT_STOP> gradientStops(colorCount);
    const float* ratios = 0;
    if (pRatios && ratiosCount == colorCount)
        ratios = pRatios;

    for (DWORD i = 0; i < colorCount; i++)
    {
        gradientStops[i].color = ARGB_D2D::toDx(color[i]);
        if (ratios)
            gradientStops[i].position = ratios[i];
        else
            gradientStops[i].position = ((float)i) / (((float)colorCount) - 1.0f);	// 0-1
    }

    CComPtr<ID2D1GradientStopCollection> pGradientStops = 0;
    ID2D1DeviceContext* pD2DDeviceContext = d2dRender;
    HRESULT hr = pD2DDeviceContext->CreateGradientStopCollection(&gradientStops[0], colorCount,
                                                                         D2D1_GAMMA_2_2,
                                                                         (D2D1_EXTEND_MODE)fillMode,
                                                                         &pGradientStops);

    if (FAILED(hr))
        return;

    hr = pD2DDeviceContext->CreateRadialGradientBrush(pt, pGradientStops, &m_pBrush);

}

CD2DBrush_RadialGradient::~CD2DBrush_RadialGradient()
{
    SafeRelease(m_pBrush);
}


NAMESPACE_D2D_END

