#include "lyric_wnd_function.h"

using namespace NAMESPACE_D2D;


NAMESPACE_LYRIC_WND_BEGIN


template<typename _Ty>
inline void SafeDelete(_Ty*& p)
{
    if (p)
        delete p;
    p = nullptr;
}

bool LYRIC_WND_DX::re_create(LYRIC_WND_INFU* pWndInfo)
{
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    auto& clrNormal = wnd_info.clrNormal;
    auto& clrLight = wnd_info.clrLight;
    RECT rc;
    GetClientRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    SafeDelete(hCanvas);
    hCanvas = new CD2DRender(cxClient, cyClient);

    SafeDelete(image_shadow);
    image_shadow = __shadow_image(*hCanvas);

    if (!hFont)
        re_create_font(pWndInfo);

    re_create_brush(pWndInfo, true);
    re_create_brush(pWndInfo, false);
    re_create_border(pWndInfo);
    re_create_image(pWndInfo);

    SafeDelete(hbrWndBorder);
    SafeDelete(hbrWndBack);
    hbrWndBorder = new CD2DBrush(*hCanvas, clrWndBorder);
    hbrWndBack = new CD2DBrush(*hCanvas, clrBack);


    SafeDelete(hbrLine);
    hbrLine = new CD2DBrush(*hCanvas, MAKEARGB(100, 255, 255, 255));



    return true;
}

bool LYRIC_WND_DX::re_create_brush(LYRIC_WND_INFU* pWndInfo, bool isLight)
{
    DWORD* pClr;
    int size;
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    NAMESPACE_D2D::CD2DBrush_LinearGradient** ppBrush;
    if (isLight)
    {
        SafeDelete(hbrLight);
        pClr = &wnd_info.clrLight[0];
        size = (int)wnd_info.clrLight.size();
        ppBrush = &hbrLight;
    }
    else
    {
        SafeDelete(hbrNormal);
        pClr = &wnd_info.clrNormal[0];
        size = (int)wnd_info.clrNormal.size();
        ppBrush = &hbrNormal;
    }

    POINT_F pt1 = { 0.f, 0.f };
    POINT_F pt2 = { 1.f, .1f };
    *ppBrush = new CD2DBrush_LinearGradient(*hCanvas, pt1, pt2, pClr, size);
    return *ppBrush != nullptr;
}

bool LYRIC_WND_DX::re_create_border(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(hbrBorder);
    hbrBorder = new CD2DBrush(*hCanvas, pWndInfo->clrBorder);
    return hbrBorder != nullptr;
}

bool LYRIC_WND_DX::re_create_font(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(hFont);

    LOGFONT lf = pWndInfo->lf;
    lf.lfHeight = -MulDiv(lf.lfHeight, pWndInfo->scale, 72);
    hFont = new CD2DFont(&lf);

    IDWriteTextLayout* pTextLayout = lyric_wnd_create_text_layout(pWndInfo->pszDefText, pWndInfo->nDefText, *hFont, 0, 0);
    if (pTextLayout)
    {
        DWRITE_TEXT_METRICS metrics = { 0 };
        pTextLayout->GetMetrics(&metrics);
        SafeRelease(pTextLayout);
        pWndInfo->nLineDefWidth = metrics.widthIncludingTrailingWhitespace;
        pWndInfo->nLineHeight = metrics.height;
    }

    return hFont != nullptr;
}

bool LYRIC_WND_DX::re_create_image(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(image);
    lyric_wnd_load_image(*pWndInfo);
    return false;
}

bool LYRIC_WND_DX::destroy(bool isDestroyFont)
{
    // 除了字体, 剩下的都是设备相关的
    if (isDestroyFont)
        SafeDelete(hFont);

    SafeRelease(pBitmapBack);

    SafeDelete(hCanvas);
    SafeDelete(hbrBorder);
    SafeDelete(hbrWndBorder);
    SafeDelete(hbrWndBack);
    SafeDelete(hbrLine);
    SafeDelete(hbrNormal);
    SafeDelete(hbrLight);
    SafeDelete(image);
    SafeDelete(image_shadow);
    return true;
}


void LYRIC_WND_INFU::set_def_arg(const LYRIC_WND_ARG* arg)
{
    // 有值就根据传递进来的值设置, 没有值就设置默认值
    LYRIC_WND_ARG def = { 0 };
    lyric_wnd_get_default_arg(&def);
    if (!arg)
        arg = &def;

    if (arg->pClrNormal)
        clrNormal.assign(&arg->pClrNormal[0], &arg->pClrNormal[arg->nClrNormal]);
    if (arg->pClrLight)
        clrLight.assign(&arg->pClrLight[0], &arg->pClrLight[arg->nClrLight]);

    clrBorder = arg->clrBorder;
    dx.clrWndBorder = arg->clrWndBorder;
    dx.clrBack = arg->clrWndBack;

    LPCWSTR pszFontName = arg->pszFontName;
    if (!pszFontName || !*pszFontName)
        pszFontName = L"微软雅黑";

    wcscpy_s(lf.lfFaceName, pszFontName);
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = arg->nFontSize;


}

LYRIC_WND_CACHE_OBJ::LYRIC_WND_CACHE_OBJ()
{
    preIndex = -1;
    preText = nullptr;
    preLength = 0;
    pBitmapNormal = nullptr;
    pBitmapLight = nullptr;
    rcBounds = { 0 };
}

LYRIC_WND_CACHE_OBJ::~LYRIC_WND_CACHE_OBJ()
{
    SafeRelease(pBitmapNormal);
    SafeRelease(pBitmapLight);
}

LYRIC_WND_INFU::LYRIC_WND_INFU()
{
    hWnd = nullptr;
    hTips = nullptr;
    hLyric = nullptr;
    prevIndexLine = -1;
    prevWidth = 0;
    nTimeOffset = 0;
    nLineHeight = 0;
    nCurrentTimeMS = 0;
    isFillBack = 0;
    status = 0;
    change = 0;
    nMinWidth = 0;
    nMinHeight = 0;
    nLineTop1 = 0;
    nLineTop2 = 0;
    rcWindow = { 0 };
    nLineDefWidth = 0;
    shadowRadius = 0.f;
    mode = LYRIC_MODE::DOUBLE_ROW;
    pszDefText = L"该歌曲暂时没有歌词";
    nDefText = 9;

    dx.clrBack = MAKEARGB(100, 0, 0, 0);
    clrBorder = MAKEARGB(255, 33, 33, 33);

    pfnCommand = nullptr;
    lParam = 0;
}

// 设置歌词窗口数据到窗口
void lyric_wnd_set_data(HWND hWnd, PLYRIC_WND_INFU pWndInfo)
{
    SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pWndInfo);
}

// 从窗口获取歌词窗口数据
PLYRIC_WND_INFU lyric_wnd_get_data(HWND hWnd)
{
    return (PLYRIC_WND_INFU)GetWindowLongPtrW(hWnd, 0);
}

NAMESPACE_LYRIC_WND_END