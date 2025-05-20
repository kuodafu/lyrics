#include "lyric_wnd_function.h"
#include "CCustomTextRenderer.h"

using namespace NAMESPACE_D2D;


NAMESPACE_LYRIC_WND_BEGIN


template<typename _Ty>
inline void SafeDelete(_Ty*& p)
{
    if (p)
        delete p;
    p = nullptr;
}

bool LYRIC_WND_DX::re_create(LYRIC_WND_INFO* pWndInfo)
{
    LYRIC_WND_INFO& wnd_info = *pWndInfo;
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

bool LYRIC_WND_DX::re_create_brush(LYRIC_WND_INFO* pWndInfo, bool isLight)
{
    DWORD* pClr;
    int size;
    LYRIC_WND_INFO& wnd_info = *pWndInfo;
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

bool LYRIC_WND_DX::re_create_border(LYRIC_WND_INFO* pWndInfo)
{
    SafeDelete(hbrBorder);
    hbrBorder = new CD2DBrush(*hCanvas, pWndInfo->clrBorder);
    return hbrBorder != nullptr;
}

bool LYRIC_WND_DX::re_create_font(LYRIC_WND_INFO* pWndInfo)
{
    SafeDelete(hFont);

    LOGFONT lf = pWndInfo->lf;
    lf.lfHeight = -MulDiv(lf.lfHeight, pWndInfo->scale, 72);
    hFont = new CD2DFont(&lf);

    IDWriteTextLayout* pTextLayout = lyric_wnd_create_text_layout(pWndInfo->pszDefText, pWndInfo->nDefText, *hFont, 0, 0);
    if (pTextLayout)
    {
        float width = 0.f, height = 0.f;
        pWndInfo->word_width = 0;
        pWndInfo->word_height = 0;
        pWndInfo->nLineDefWidth = 0;
        pWndInfo->nLineDefHeight = 0;

        CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                         FLOAT baselineOriginX,
                                         FLOAT baselineOriginY,
                                         DWRITE_MEASURING_MODE measuringMode,
                                         DWRITE_GLYPH_RUN const* glyphRun,
                                         DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                         IUnknown* clientDrawingEffect)
        {
            // 获取字体度量
            for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
            {
                DWRITE_FONT_METRICS metrics{};
                glyphRun->fontFace->GetMetrics(&metrics);

                float designUnitsPerEm = (float)metrics.designUnitsPerEm;
                float _height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
                float _width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

                if (pWndInfo->word_width < _width)
                    pWndInfo->word_width = _width;      // 竖屏用宽度
                if (pWndInfo->word_height < _height)
                    pWndInfo->word_height = _height;    // 横屏用高度

                wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';

                pWndInfo->nLineDefWidth += _width;      // 记录总宽度, 横屏用
                pWndInfo->nLineDefHeight += _height;    // 记录总高度, 竖屏用

            }
        });
        pTextLayout->Draw(0, &renderer, 0, 0);

        SafeRelease(pTextLayout);
    }

    return hFont != nullptr;
}

bool LYRIC_WND_DX::re_create_image(LYRIC_WND_INFO* pWndInfo)
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


void LYRIC_WND_INFO::set_def_arg(const LYRIC_WND_ARG* arg)
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

void LYRIC_WND_INFO::dpi_change(HWND hWnd)
{
    scale = hWnd;
    const int _10 = scale(10);
    if (dx.hFont)
        dx.re_create_font(this);
    padding = (float)(_10 / 2);
    shadowRadius = (float)_10;

}

float LYRIC_WND_INFO::get_lyric_line_height() const
{
    if (has_mode(LYRIC_MODE::VERTICAL))
    {
        // 竖屏, 取宽度加上边距
        return word_width + padding * 2;
    }
    return word_height + padding * 2;
}

float LYRIC_WND_INFO::get_lyric_line_width(float vl) const
{
    if (has_mode(LYRIC_MODE::VERTICAL))
        return (vl ? vl : nLineDefHeight) + padding * 2;
    
    return (vl ? vl : nLineDefWidth) + padding * 2;
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

LYRIC_WND_INFO::LYRIC_WND_INFO()
{
    const int clear_size = offsetof(LYRIC_WND_INFO, line1);
    memset(this, 0, clear_size);
    prevIndexLine = -1;
    mode = LYRIC_MODE::DOUBLE_ROW;
    pszDefText = L"该歌曲暂时没有歌词";
    nDefText = 9;

    dx.clrBack = MAKEARGB(100, 0, 0, 0);
    clrBorder = MAKEARGB(255, 33, 33, 33);

    pfnCommand = nullptr;
    lParam = 0;
}

// 设置歌词窗口数据到窗口
void lyric_wnd_set_data(HWND hWnd, PLYRIC_WND_INFO pWndInfo)
{
    SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pWndInfo);
}

// 从窗口获取歌词窗口数据
PLYRIC_WND_INFO lyric_wnd_get_data(HWND hWnd)
{
    return (PLYRIC_WND_INFO)GetWindowLongPtrW(hWnd, 0);
}

bool isLatinCharacter(wchar_t ch)
{
    return
        // Printable ASCII characters: 0x21 ('!') to 0x7E ('~')
        (ch >= 0x20 && ch <= 0x7E) ||

        // Latin-1 Supplement
        (ch >= 0x00C0 && ch <= 0x00D6) || // 
        (ch >= 0x00D8 && ch <= 0x00F6) || // 
        (ch >= 0x00F8 && ch <= 0x00FF) || // 

        // Latin Extended-A
        (ch >= 0x0100 && ch <= 0x017F) ||

        // Latin Extended-B
        (ch >= 0x0180 && ch <= 0x024F);
}

NAMESPACE_LYRIC_WND_END