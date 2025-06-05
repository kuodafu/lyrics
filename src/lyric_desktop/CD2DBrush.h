#pragma once
#include "CD2DRender.h"
KUODAFU_NAMESPACE_BEGIN


class CD2DBrush
{
    ID2D1SolidColorBrush* m_pBrush;    // ��ɫ��ˢ

public:
    CD2DBrush(CD2DRender& d2dRender, DWORD argb);
    ~CD2DBrush();
    operator ID2D1SolidColorBrush*() const { return m_pBrush; }
    ID2D1SolidColorBrush* operator->() const { return m_pBrush; }


    bool SetColor(DWORD argb);
    DWORD GetColor(DWORD argb);
};

// ���Խ��仭ˢ
class CD2DBrush_LinearGradient
{
    ID2D1LinearGradientBrush* m_pBrush;    // ���Խ��仭ˢ

public:
    // �������Խ��仭ˢ
    // pt1,pt2 = ���, �յ�����
    // color,colorCount = ��ɫ����, ��ɫ��������
    // fillMode = ���ģʽ
    // pRatios, ratiosCount = �����������, ȡֵ��Χ0-1, ��������
    CD2DBrush_LinearGradient(CD2DRender& d2dRender, const POINT_F& pt1, const POINT_F& pt2, ARGB* color, DWORD colorCount, int fillMode = 0, const float* pRatios = 0, DWORD ratiosCount = 0);
    ~CD2DBrush_LinearGradient();
    operator ID2D1LinearGradientBrush*() const { return m_pBrush; }
    ID2D1LinearGradientBrush* operator->() const { return m_pBrush; }

};
// ���Խ��仭ˢ
class CD2DBrush_RadialGradient
{
    ID2D1RadialGradientBrush* m_pBrush;    // ���Խ��仭ˢ

public:
    // �������佥�仭ˢ, �����ĵ�������ɢ����
    // pos = �������ĵ��Լ�������չ�ĳ���
    // color,colorCount = ��ɫ����, ��ɫ��������
    // fillMode = ���ģʽ
    // pRatios, ratiosCount = �����������, ȡֵ��Χ0-1, ��������
    CD2DBrush_RadialGradient(CD2DRender& d2dRender, const ELLIPSE_F* pos, ARGB* color, DWORD colorCount, int fillMode = 0, const float* pRatios = 0, DWORD ratiosCount = 0);
    ~CD2DBrush_RadialGradient();
    operator ID2D1RadialGradientBrush*() const { return m_pBrush; }
    ID2D1RadialGradientBrush* operator->() const { return m_pBrush; }

};



KUODAFU_NAMESPACE_END
