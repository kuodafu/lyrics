#pragma once
#include "CD2DRender.h"
KUODAFU_NAMESPACE_BEGIN


class CD2DBrush
{
    ID2D1SolidColorBrush* m_pBrush;    // 纯色画刷

public:
    CD2DBrush(CD2DRender& d2dRender, DWORD argb);
    ~CD2DBrush();
    operator ID2D1SolidColorBrush*() const { return m_pBrush; }
    ID2D1SolidColorBrush* operator->() const { return m_pBrush; }


    bool SetColor(DWORD argb);
    DWORD GetColor(DWORD argb);
};

// 线性渐变画刷
class CD2DBrush_LinearGradient
{
    ID2D1LinearGradientBrush* m_pBrush;    // 线性渐变画刷

public:
    // 创建线性渐变画刷
    // pt1,pt2 = 起点, 终点坐标
    // color,colorCount = 颜色数组, 颜色数组数量
    // fillMode = 填充模式
    // pRatios, ratiosCount = 渐变比例数组, 取值范围0-1, 数组数量
    CD2DBrush_LinearGradient(CD2DRender& d2dRender, const POINT_F& pt1, const POINT_F& pt2, ARGB* color, DWORD colorCount, int fillMode = 0, const float* pRatios = 0, DWORD ratiosCount = 0);
    ~CD2DBrush_LinearGradient();
    operator ID2D1LinearGradientBrush*() const { return m_pBrush; }
    ID2D1LinearGradientBrush* operator->() const { return m_pBrush; }

};
// 线性渐变画刷
class CD2DBrush_RadialGradient
{
    ID2D1RadialGradientBrush* m_pBrush;    // 线性渐变画刷

public:
    // 创建放射渐变画刷, 从中心点往外扩散渐变
    // pos = 包含中心点以及向外扩展的长度
    // color,colorCount = 颜色数组, 颜色数组数量
    // fillMode = 填充模式
    // pRatios, ratiosCount = 渐变比例数组, 取值范围0-1, 数组数量
    CD2DBrush_RadialGradient(CD2DRender& d2dRender, const ELLIPSE_F* pos, ARGB* color, DWORD colorCount, int fillMode = 0, const float* pRatios = 0, DWORD ratiosCount = 0);
    ~CD2DBrush_RadialGradient();
    operator ID2D1RadialGradientBrush*() const { return m_pBrush; }
    ID2D1RadialGradientBrush* operator->() const { return m_pBrush; }

};



KUODAFU_NAMESPACE_END
