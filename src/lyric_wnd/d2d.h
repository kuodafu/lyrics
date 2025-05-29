#pragma once
#include <windows.h>
#include <d2d1_1.h>
#include <wincodec.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <atlbase.h>
#include <gdiplus.h>


#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "Msimg32.lib")

#ifndef Assert
#   if defined( DEBUG ) || defined( _DEBUG )
#       include <assert.h>
#       define ____L(s) L ## s
#       define __L(s) ____L(s)
#       define Assert(b) {if(!(b)){wchar_t __debug_str[520];swprintf_s(__debug_str, 520, L"检测到错误, 文件 " __L(__FILE__) L" , 行号 = %d\n", __LINE__);OutputDebugStringW(__debug_str);}}assert(b)
#   else
#       define Assert(b)
#   endif //DEBUG || _DEBUG
#endif


#define RGB2ARGB(_cr,a)     ((DWORD)(BYTE)(a) << 24) | ((_cr & 0x000000ff) << 16) | ((_cr & 0x0000ff00)) | ((_cr & 0x00ff0000) >> 16)
#define ARGB2RGB(_cr)       ((COLORREF)( ((BYTE)((_cr & 0xff0000) >> 16)|((WORD)((BYTE)((_cr & 0xff00) >> 8))<<8)) | (((DWORD)(BYTE)(_cr & 0xff))<<16) ))


#define MAKERGBA(r,g,b,a)   ((COLORREF)( (((BYTE)(b) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(r))<<16)) | (((DWORD)(BYTE)(a))<<24) ))
#define MAKEBGRA(b,g,r,a)   ((COLORREF)( (((BYTE)(b) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(r))<<16)) | (((DWORD)(BYTE)(a))<<24) ))
#define MAKEABGR(a,b,g,r)   ((COLORREF)( (((BYTE)(b) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(r))<<16)) | (((DWORD)(BYTE)(a))<<24) ))
#define MAKEARGB(a,r,g,b)   ((COLORREF)( (((BYTE)(b) | ((WORD)((BYTE)(g))<<8)) | (((DWORD)(BYTE)(r))<<16)) | (((DWORD)(BYTE)(a))<<24) ))

#define MAKERGB(r,g,b)      ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define MAKEBGR(b,g,r)      ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))


#define ARGB_GETA(_cr)      ((BYTE)((_cr & 0xff000000) >> 24))  //(LOBYTE(((DWORD)(_cr)) >> 24))
#define ARGB_GETR(_cr)      ((BYTE)((_cr & 0x00ff0000) >> 16))  //(LOBYTE(((DWORD)(_cr)) >> 16))
#define ARGB_GETG(_cr)      ((BYTE)((_cr & 0x0000ff00) >> 8))   //(LOBYTE(((WORD)(_cr)) >> 8))
#define ARGB_GETB(_cr)      ((BYTE)((_cr & 0x000000ff)))        //(LOBYTE(rgb))

#define ARGB_SETA(_cr, _v)  (((DWORD)_cr & 0x00ffffff) | ((DWORD)(BYTE)(_v) << 24))
#define ARGB_SETR(_cr, _v)  (((DWORD)_cr & 0xff00ffff) | ((DWORD)(BYTE)(_v) << 16))
#define ARGB_SETG(_cr, _v)  (((DWORD)_cr & 0xffff00ff) | ((WORD)(BYTE)(_v) << 8))
#define ARGB_SETB(_cr, _v)  (((DWORD)_cr & 0xffffff00) | ((BYTE)(_v)))

#define RGB_GETR(_cr)       ((BYTE)((_cr & 0x000000ff)))        //(LOBYTE(rgb))
#define RGB_GETG(_cr)       ((BYTE)((_cr & 0x0000ff00) >> 8))   //(LOBYTE(((WORD)(rgb)) >> 8))
#define RGB_GETB(_cr)       ((BYTE)((_cr & 0x00ff0000) >> 16))  //(LOBYTE((rgb)>>16))

#define RGB_SETR(_cr, _v)   (((DWORD)_cr & 0xffffff00) | ((BYTE)(_v)))
#define RGB_SETG(_cr, _v)   (((DWORD)_cr & 0xffff00ff) | ((WORD)(BYTE)(_v) << 8))
#define RGB_SETB(_cr, _v)   (((DWORD)_cr & 0xff00ffff) | ((DWORD)(BYTE)(_v) << 16))

// RGB 颜色反转, 灰色反转后还是灰色
#define RGB_REVERSE(_cr)    MAKERGB( (255 - RGB_GETR(_cr)), (255 - RGB_GETG(_cr)), (255 - RGB_GETB(_cr)) )
// ARGB 颜色反转, 灰色反转后还是灰色, 透明度不变
#define ARGB_REVERSE(_cr)   MAKEARGB( (ARGB_GETA(_cr)), (255 - ARGB_GETR(_cr)), (255 - ARGB_GETG(_cr)), (255 - ARGB_GETB(_cr)) )





#define NAMESPACE_D2D d2d
#define NAMESPACE_D2D_BEGIN namespace NAMESPACE_D2D {
#define NAMESPACE_D2D_END }

NAMESPACE_D2D_BEGIN


template<typename T>inline void SafeRelease(T*& pObj)
{
    if (pObj)
        pObj->Release();
    pObj = nullptr;
}
template<typename T>inline void SafeAddref(T* pObj)
{
    if (pObj)
        pObj->AddRef();
}

template<typename T, typename R>inline bool __query(T l, R r)
{
    return ((R)l & r) == r;
}


struct D2D_GDI_DATA_STRUCT
{
    ID2D1Factory1* pFactory;                    // D2D工厂接口, 这个接口是所有D2D程序的起始点, 几乎所有的D2D资源都是由这个接口创建的
    IDWriteFactory* pDWriteFactory;             // 文字工厂接口
    IWICImagingFactory2* pWICFactory;           // WIC图像工厂接口
    ID2D1Device* pD2DDevice;                    // d2d设备
    float DpiX;
    float DpiY;
    ~D2D_GDI_DATA_STRUCT();
};

// D2D使用的颜色类, 内部记录的是DWORD 类型的ARGB颜色
struct ARGB_D2D
{
    // BGRA
    DWORD argb{};
    ARGB_D2D() :argb(0) {}
    ARGB_D2D(DWORD argb) { operator()(argb); }
    ARGB_D2D(BYTE a, BYTE r, BYTE g, BYTE b) { operator()(a, r, g, b); }
    ARGB_D2D(BYTE r, BYTE g, BYTE b) { operator()(255, r, g, b); }
    ARGB_D2D(BYTE alpha, DWORD rgb) { operator()(alpha, rgb); }
    ARGB_D2D(const D2D1_COLOR_F& color) { operator()(color); }

    inline static DWORD toArgb(const D2D1_COLOR_F& color)
    {
        DWORD clr = MAKEBGRA(
            (BYTE)(color.b * 255.0f),
            (BYTE)(color.g * 255.0f),
            (BYTE)(color.r * 255.0f),
            (BYTE)(color.a * 255.0f)
        );
        return clr;
    }
    inline static DWORD toArgb(COLORREF rgb, BYTE alpha = 255) { return RGB2ARGB(rgb, 255); }
    inline static DWORD toArgb(BYTE r, BYTE g, BYTE b, BYTE alpha = 255) { return MAKERGBA(r, g, b, alpha); }
    inline static DWORD toRgb(DWORD argb) { return ARGB2RGB(argb); }
    inline DWORD toRgb() const { return toRgb(argb); }
    inline ARGB_D2D& operator()(BYTE alpha, DWORD rgb)
    {
        // RGB  在内存中[0]=r, [1]=g, [2]=b, [3]=未使用
        // ARGB 在内存中[0]=b, [1]=g, [2]=r, [3]=a
        argb = RGB2ARGB(rgb, alpha);
        return *this;
    }
    inline ARGB_D2D& operator()(DWORD argb) { this->argb = argb; return *this; }
    inline ARGB_D2D& operator()(BYTE r, BYTE g, BYTE b) { operator()(255, r, g, b); return *this; }
    inline ARGB_D2D& operator()(BYTE a, BYTE r, BYTE g, BYTE b)
    {
        argb = MAKEARGB(a, r, g, b);
        return *this;
    }
    inline DWORD toGdiplus()const { return argb; }
    inline BYTE getA()const { return ARGB_GETA(argb); }
    inline BYTE getR()const { return ARGB_GETR(argb); }
    inline BYTE getG()const { return ARGB_GETG(argb); }
    inline BYTE getB()const { return ARGB_GETB(argb); }
    inline void setA(BYTE a) { argb = ARGB_SETA(argb, a); }
    inline void setR(BYTE r) { argb = ARGB_SETR(argb, r); }
    inline void setG(BYTE g) { argb = ARGB_SETG(argb, g); }
    inline void setB(BYTE b) { argb = ARGB_SETB(argb, b); }
    inline void operator=(DWORD argb) { this->argb = argb; }
    inline operator DWORD() { return toGdiplus(); }
    inline operator D2D1_COLOR_F() { return toDx(); }

    inline ARGB_D2D& operator()(const D2D1_COLOR_F& color)
    {
        this->argb = toArgb(color);
        return *this;
    }
    inline D2D1_COLOR_F toDx() const
    {
        return toDx(argb);
    }
    inline static D2D1_COLOR_F toDx(DWORD argb)
    {
        D2D1_COLOR_F dxColor{};
        dxColor.a = (float)ARGB_GETA(argb) / 255.0f;
        dxColor.r = (float)ARGB_GETR(argb) / 255.0f;
        dxColor.g = (float)ARGB_GETG(argb) / 255.0f;
        dxColor.b = (float)ARGB_GETB(argb) / 255.0f;
        return dxColor;
    }


};


// D2D 矩形, 左顶右底
template<typename T>
struct RECTBASE
{
    T left;
    T top;
    T right;
    T bottom;
    RECTBASE() : left(0), top(0), right(0), bottom(0) {}
    RECTBASE(const Gdiplus::RectF& rc) { operator()(rc); }
    RECTBASE(const Gdiplus::Rect& rc) { operator()(rc); }
    RECTBASE(const D2D1_RECT_F& rc) { operator()(rc); }
    RECTBASE(const D2D1_RECT_U& rc) { operator()(rc); }
    RECTBASE(const D2D1_RECT_L& rc) { operator()(rc); }
    RECTBASE(T left, T top, T right, T bottom) { operator()(left, top, right, bottom); }


    inline T width() const { return right - left; }
    inline T height() const { return bottom - top; }
    inline const RECTBASE* ptr()const { return this; }
    inline RECTBASE* operator()(const Gdiplus::RectF& rc)
    {
        this->left = static_cast<T>(rc.X);
        this->top = static_cast<T>(rc.Y);
        this->right = static_cast<T>(rc.X + rc.Width);
        this->bottom = static_cast<T>(rc.Y + rc.Height);
        return this;
    }
    inline RECTBASE* operator()(const Gdiplus::Rect& rc)
    {
        this->left = static_cast<T>(rc.X);
        this->top = static_cast<T>(rc.Y);
        this->right = static_cast<T>(rc.X + rc.Width);
        this->bottom = static_cast<T>(rc.Y + rc.Height);
        return this;
    }

    inline RECTBASE* operator()(T left, T top, T right, T bottom)
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
        return this;
    }
    inline RECTBASE* offset(T left, T top)
    {
        this->left += left;
        this->right += left;
        this->top += top;
        this->bottom += top;
        return this;
    }

    inline operator Gdiplus::RectF()
    {
        Gdiplus::RectF rc;
        rc.X = static_cast<Gdiplus::REAL>(left);
        rc.Y = static_cast<Gdiplus::REAL>(top);
        rc.Width = static_cast<Gdiplus::REAL>(right - left);
        rc.Height = static_cast<Gdiplus::REAL>(bottom - top);
        return rc;
    }
    inline operator Gdiplus::Rect()
    {
        Gdiplus::Rect rc;
        rc.X = static_cast<INT>(left);
        rc.Y = static_cast<INT>(top);
        rc.Width = static_cast<INT>(right - left);
        rc.Height = static_cast<INT>(bottom - top);
        return rc;
    }




    inline RECTBASE* operator()(const D2D1_RECT_F& rc)
    {
        this->left = static_cast<T>(rc.left);
        this->top = static_cast<T>(rc.top);
        this->right = static_cast<T>(rc.right);
        this->bottom = static_cast<T>(rc.bottom);
        return this;
    }
    inline RECTBASE* operator()(const D2D1_RECT_U& rc)
    {
        this->left = static_cast<T>(rc.left);
        this->top = static_cast<T>(rc.top);
        this->right = static_cast<T>(rc.right);
        this->bottom = static_cast<T>(rc.bottom);
        return this;
    }
    inline RECTBASE* operator()(const D2D1_RECT_L& rc)
    {
        this->left = static_cast<T>(rc.left);
        this->top = static_cast<T>(rc.top);
        this->right = static_cast<T>(rc.right);
        this->bottom = static_cast<T>(rc.bottom);
        return this;
    }

    inline operator D2D1_RECT_F()
    {
        D2D1_RECT_F rc;
        rc.left = static_cast<FLOAT>(left);
        rc.top = static_cast<FLOAT>(top);
        rc.right = static_cast<FLOAT>(right);
        rc.bottom = static_cast<FLOAT>(bottom);
        return rc;
    }
    inline operator D2D1_RECT_U()
    {
        D2D1_RECT_U rc;
        rc.left = static_cast<UINT32>(left);
        rc.top = static_cast<UINT32>(top);
        rc.right = static_cast<UINT32>(right);
        rc.bottom = static_cast<UINT32>(bottom);
        return rc;
    }
    inline operator D2D1_RECT_L()
    {
        D2D1_RECT_L rc;
        rc.left = static_cast<LONG>(left);
        rc.top = static_cast<LONG>(top);
        rc.right = static_cast<LONG>(right);
        rc.bottom = static_cast<LONG>(bottom);
        return rc;
    }


};


// 圆角矩形, 左顶右底 + x/y角度
template<typename T>struct RECTROUNDEDBASE : RECTBASE<T>
{
    using RECTBASE<T>::offset;
    T radiusX;
    T radiusY;
    RECTROUNDEDBASE() :RECTBASE<T>(), radiusX(0), radiusY(0) {}
    RECTROUNDEDBASE(T left, T top, T right, T bottom, T radiusX, T radiusY = 0)
    {
        operator()(left, top, right, bottom, radiusX, radiusY);
    }
    RECTROUNDEDBASE(const RECTBASE<T>& rc, T radiusX = 0, T radiusY = 0)
    {
        operator()(rc, radiusX, radiusY);
    }
    inline const RECTROUNDEDBASE* ptr()const { return this; }
    inline RECTROUNDEDBASE* offset(T left, T top, T radiusX, T radiusY)
    {
        offset(left, top);
        this->radiusX = radiusX;
        this->radiusY = radiusY;
        return this;
    }
    // 初始化圆角矩形, 
    inline RECTROUNDEDBASE* operator()(T left, T top, T right, T bottom, T radiusX, T radiusY = 0)
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
        this->radiusX = radiusX;
        this->radiusY = radiusY;
        return this;
    }
    inline RECTROUNDEDBASE* operator()(const RECTBASE<T>& rc, T radiusX = 0, T radiusY = 0)
    {
        this->left = rc.left;
        this->top = rc.top;
        this->right = rc.right;
        this->bottom = rc.bottom;
        this->radiusX = radiusX;
        this->radiusY = radiusY;
        return this;
    }
};

// 坐标类型, 存放x和y
template<typename T>struct POINTBASE
{
    T x;
    T y;
    POINTBASE() : x(0), y(0) {}
    POINTBASE(T x, T y) { operator()(x, y); }
    inline POINTBASE* operator()(T x, T y) { this->x = x; this->y = y; return this; }
    inline const POINTBASE* ptr()const { return this; }


    inline operator D2D1_POINT_2F() const { return toDx(); }
    inline operator D2D1_POINT_2L() const { return toDxL(); }
    inline operator D2D1_POINT_2U() const { return toDxU(); }
    inline operator Gdiplus::PointF() const { return toGpPointF(); }
    inline operator Gdiplus::Point() const { return toGpPoint(); }

    // 转成d2d需要的 坐标类型, float
    inline D2D1_POINT_2F toDx() const
    {
        D2D1_POINT_2F pt = { static_cast<float>(x), static_cast<float>(y) };
        return pt;
    }

    // 转成d2d需要的 坐标类型, long
    inline D2D1_POINT_2L toDxL() const
    {
        D2D_POINT_2L pt = { static_cast<LONG>(x), static_cast<LONG>(y) };
        return pt;
    }

    // 转成d2d需要的 坐标类型, unsigned int
    inline D2D1_POINT_2U toDxU() const
    {
        D2D1_POINT_2U pt = { static_cast<UINT32>(x), static_cast<UINT32>(y) };
        return pt;
    }

    // 转成gdi+使用的坐标, float
    inline Gdiplus::PointF toGpPointF() const
    {
        Gdiplus::PointF pt = { static_cast<float>(x), static_cast<float>(y) };
        return pt;
    }

    // 转成gdi使用的坐标
    inline Gdiplus::Point toGpPoint() const
    {
        Gdiplus::Point pt = { static_cast<INT>(x), static_cast<INT>(y) };
        return pt;
    }
};

// 椭圆坐标, x和y坐标记录椭圆的中心点, radiusX 和 radiusY 记录椭圆的横向宽度, 纵向宽度, 如果 radiusX=radiusY 那就是个正圆
template<typename T>struct ELLIPSEBASE : POINTBASE<T>
{
    T radiusX;
    T radiusY;
    ELLIPSEBASE() : POINTBASE<T>(), radiusX(0), radiusY(0) {}
    ELLIPSEBASE(T left, T top, T right, T bottom) { operator()(left, top, right, bottom); }
    ELLIPSEBASE(const POINTBASE<T>& pt, T radiusX = 0, T radiusY = 0) { operator()(pt, radiusX, radiusY); }
    ELLIPSEBASE(const RECTBASE<T>& rc) { operator()(rc); }
    ELLIPSEBASE(const RECTBASE<T>* rc) { operator()(rc); }
    inline ELLIPSEBASE* operator()(T left, T top, T right, T bottom)
    {
        T cx = right - left;
        T cy = bottom - top;
        this->x = left + cx / 2;
        this->y = top + cy / 2;
        this->radiusX = cx / 2;
        this->radiusY = cy / 2;
        return this;
    }
    inline ELLIPSEBASE* operator()(const POINTBASE<T>& pt, T radiusX = 0, T radiusY = 0)
    {
        this->x = pt.x;
        this->y = pt.y;
        this->radiusX = radiusX;
        this->radiusY = radiusY;
        return this;
    }
    inline ELLIPSEBASE* operator()(const RECTBASE<T>& rc)
    {
        return operator()(&rc);
    }
    inline ELLIPSEBASE* operator()(const RECTBASE<T>* rc)
    {
        T cx = rc->right - rc->left;
        T cy = rc->bottom - rc->top;
        this->x = rc->left + cx / 2;
        this->y = rc->top + cy / 2;
        this->radiusX = cx / 2;
        this->radiusY = cy / 2;
        return this;
    }
    inline const ELLIPSEBASE* ptr()const { return this; }

    // 将当前记录的坐标加上偏移值, 如果需要减少, 请传递负值, 内部不会改变椭圆大小
    inline ELLIPSEBASE* offset(T x, T y)
    {
        this->x += x;
        this->y += y;
        //this->radiusX += x;
        //this->radiusY += y;
        return this;
    }

    inline operator Gdiplus::RectF() { return toGpRectF(); }
    inline operator Gdiplus::Rect() { return toGpRect(); }
    inline operator RECT() { return toRect(); }


    // 转成gdi+使用的矩形, 根据记录的椭圆坐标计算出gdi+使用的矩形
    inline Gdiplus::RectF toGpRectF() const
    {
        Gdiplus::RectF rc;
        rc.Width = static_cast<float>(this->radiusX * 2);
        rc.Height = static_cast<float>(this->radiusY * 2);
        rc.X = static_cast<float>(((float)this->x) - rc.Width / 2);
        rc.Y = static_cast<float>(((float)this->y) - rc.Height / 2);
        return rc;
    }

    // 转成gdi+使用的矩形, 根据记录的椭圆坐标计算出gdi+使用的矩形
    inline Gdiplus::Rect toGpRect() const
    {
        Gdiplus::Rect rc;
        rc.Width = static_cast<INT>(this->radiusX * 2);
        rc.Height = static_cast<INT>(this->radiusY * 2);
        rc.X = static_cast<INT>(((INT)this->x) - rc.Width / 2);
        rc.Y = static_cast<INT>(((INT)this->y) - rc.Height / 2);
        return rc;
    }

    // 转成gdi使用的矩形, 根据记录的椭圆坐标计算出gdi使用的矩形
    inline RECT toRect() const
    {
        RECT rc;
        rc.right = static_cast<LONG>(this->radiusX * 2);
        rc.bottom = static_cast<LONG>(this->radiusY * 2);
        rc.left = static_cast<LONG>(((LONG)this->x) - rc.right / 2);
        rc.top = static_cast<LONG>(((LONG)this->y) - rc.right / 2);
        rc.right += rc.left;
        rc.bottom += rc.top;
        return rc;
    }
};

// 画线坐标, 记录起始点和结束点
template<typename T>struct LINEPOINTBASE : POINTBASE<T>
{
    T x1;
    T y1;
    LINEPOINTBASE() : POINTBASE<T>(), x1(0), y1(0) {}
    LINEPOINTBASE(T x, T y, T x1, T y1) { operator()(x, y, x1, y1); }
    LINEPOINTBASE(const POINT& pt1, const POINT& pt2) { operator()(pt1, pt2); }
    LINEPOINTBASE(const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2) { operator()(pt1, pt2); }
    inline LINEPOINTBASE* operator()(const D2D1_POINT_2F& pt1, const D2D1_POINT_2F& pt2)
    {
        this->x = pt1.x;
        this->y = pt1.y;
        this->x1 = pt2.x;
        this->y1 = pt2.y;
        return this;
    }
    inline LINEPOINTBASE* operator()(const POINT& pt1, const POINT& pt2)
    {
        this->x = pt1.x;
        this->y = pt1.y;
        this->x1 = pt2.x;
        this->y1 = pt2.y;
        return this;
    }
    inline LINEPOINTBASE* operator()(T x, T y, T x1, T y1)
    {
        this->x = x;
        this->y = y;
        this->x1 = x1;
        this->y1 = y1;
        return this;
    }
    // 获取起始坐标
    inline D2D1_POINT_2F pt1() const
    {
        D2D1_POINT_2F pt1 = { this->x, this->y };
        return pt1;
    }
    // 获取结束坐标
    inline D2D1_POINT_2F pt2() const
    {
        D2D1_POINT_2F pt2 = { this->x1, this->y1 };
        return pt2;
    }
    inline const LINEPOINTBASE* ptr()const { return this; }
    // 偏移横线坐标, 内部不会更改线条的宽高
    inline LINEPOINTBASE* offset(T x, T y)
    {
        this->x += x;
        this->x1 += x;
        this->y += y;
        this->y1 += y;
        return this;
    }
};

#define __DEF_TYPE_F(_struct, _type, _f) typedef _struct##BASE<_type> _struct##_f, *P##_struct##_f, *LP##_struct##_f;\
                                         typedef const _struct##_f* PC##_struct##_f, *LPC##_struct##_f

#define __DEF_TYPE_2(_struct) __DEF_TYPE_F(_struct, float, _F); __DEF_TYPE_F(_struct, int, _I);  __DEF_TYPE_F(_struct, int, _L)


typedef POINTBASE<float> POINT_F, * PPOINT_F, * LPPOINT_F; typedef const POINT_F* PCPOINT_F, * LPCPOINT_F; typedef POINTBASE<int> POINT_I, * PPOINT_I, * LPPOINT_I; typedef const POINT_I* PCPOINT_I, * LPCPOINT_I; typedef POINTBASE<int> POINT_L, * PPOINT_L, * LPPOINT_L; typedef const POINT_L* PCPOINT_L, * LPCPOINT_L;
typedef ELLIPSEBASE<float> ELLIPSE_F, * PELLIPSE_F, * LPELLIPSE_F; typedef const ELLIPSE_F* PCELLIPSE_F, * LPCELLIPSE_F; typedef ELLIPSEBASE<int> ELLIPSE_I, * PELLIPSE_I, * LPELLIPSE_I; typedef const ELLIPSE_I* PCELLIPSE_I, * LPCELLIPSE_I; typedef ELLIPSEBASE<int> ELLIPSE_L, * PELLIPSE_L, * LPELLIPSE_L; typedef const ELLIPSE_L* PCELLIPSE_L, * LPCELLIPSE_L;
typedef RECTBASE<float> RECT_F, * PRECT_F, * LPRECT_F; typedef const RECT_F* PCRECT_F, * LPCRECT_F; typedef RECTBASE<int> RECT_I, * PRECT_I, * LPRECT_I; typedef const RECT_I* PCRECT_I, * LPCRECT_I; typedef RECTBASE<int> RECT_L, * PRECT_L, * LPRECT_L; typedef const RECT_L* PCRECT_L, * LPCRECT_L;
typedef RECTROUNDEDBASE<float> RECTROUNDED_F, * PRECTROUNDED_F, * LPRECTROUNDED_F; typedef const RECTROUNDED_F* PCRECTROUNDED_F, * LPCRECTROUNDED_F; typedef RECTROUNDEDBASE<int> RECTROUNDED_I, * PRECTROUNDED_I, * LPRECTROUNDED_I; typedef const RECTROUNDED_I* PCRECTROUNDED_I, * LPCRECTROUNDED_I; typedef RECTROUNDEDBASE<int> RECTROUNDED_L, * PRECTROUNDED_L, * LPRECTROUNDED_L; typedef const RECTROUNDED_L* PCRECTROUNDED_L, * LPCRECTROUNDED_L;
typedef LINEPOINTBASE<float> LINEPOINT_F, * PLINEPOINT_F, * LPLINEPOINT_F; typedef const LINEPOINT_F* PCLINEPOINT_F, * LPCLINEPOINT_F; typedef LINEPOINTBASE<int> LINEPOINT_I, * PLINEPOINT_I, * LPLINEPOINT_I; typedef const LINEPOINT_I* PCLINEPOINT_I, * LPCLINEPOINT_I; typedef LINEPOINTBASE<int> LINEPOINT_L, * PLINEPOINT_L, * LPLINEPOINT_L; typedef const LINEPOINT_L* PCLINEPOINT_L, * LPCLINEPOINT_L;

//__DEF_TYPE_2(POINT);
//__DEF_TYPE_2(ELLIPSE);
//__DEF_TYPE_2(RECT);
//__DEF_TYPE_2(RECTROUNDED);
//__DEF_TYPE_2(LINEPOINT);

#undef __DEF_TYPE_2
#undef __DEF_TYPE_F

typedef DWORD ARGB;

bool d2d_init(bool isDebug);
D2D_GDI_DATA_STRUCT& d2d_get_info();



NAMESPACE_D2D_END