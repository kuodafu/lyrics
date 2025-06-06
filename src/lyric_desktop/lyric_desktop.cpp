/*
    这个文件主要显示歌词的窗口
    一个透明窗口, 显示一行/两行歌词, 歌词顶部有一些按钮操作
*/

#include <kuodafu_lyric_desktop.h>
#include <string>
#include <unordered_map>
#include "lyric_wnd_function.h"
#include <CommCtrl.h>
#include <d2d/CCustomTextRenderer.h>

#ifdef _LIB
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyric_libD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric_lib.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyric_libD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric_lib.lib")
#      endif
#   endif
#else
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyricD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyricD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric.lib")
#      endif
#   endif
#endif


using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

float _lyric_wnd_load_krc_calc_text(PLYRIC_DESKTOP_INFO pWndInfo, IDWriteTextLayout* pTextLayout, float* pHeight)
{
    *pHeight = 0;

    if (!pTextLayout)
        return 0.f;
    const bool is_vertical = pWndInfo->has_mode(LYRIC_MODE::VERTICAL);
    if (!is_vertical)
    {
        // 不是纵向, 直接计算然后返回
        CCustomTextRenderer renderer;
        pTextLayout->Draw(0, &renderer, 0, 0);
        *pHeight = renderer.get_height();
        return renderer.get_width();
    }
    
    // 走到这里就是纵向, 需要额外计算, 因为纵向的英文数字 这些字符会旋转, 需要特别计算
    // 只需要计算高度, 宽度是所有字符里最宽的那个, 高度是累加, 有旋转的加宽度, 没旋转的加高度

    float width = 0.f, height = 0.f;

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

                                         if (width < _width)
                                             width = _width; // 宽度只记录最大的宽度, 竖屏歌词宽度是固定的

                                         wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
                                         bool is_alpha = isLatinCharacter(ch);
                                         if (is_alpha)
                                             height += _width;   // 需要旋转的字符就加上宽度
                                         else
                                             height += _height;  // 非旋转的字符就加上高度
                                     }

                                     return S_OK;
                                 }
    );
    pTextLayout->Draw(0, &renderer, 0, 0);

    *pHeight = height;
    return width;
}

NAMESPACE_LYRIC_DESKTOP_END
