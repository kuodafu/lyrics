/*
    ����ļ���Ҫ��ʾ��ʵĴ���
    һ��͸������, ��ʾһ��/���и��, ��ʶ�����һЩ��ť����
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
        // ��������, ֱ�Ӽ���Ȼ�󷵻�
        CCustomTextRenderer renderer;
        pTextLayout->Draw(0, &renderer, 0, 0);
        *pHeight = renderer.get_height();
        return renderer.get_width();
    }
    
    // �ߵ������������, ��Ҫ�������, ��Ϊ�����Ӣ������ ��Щ�ַ�����ת, ��Ҫ�ر����
    // ֻ��Ҫ����߶�, ����������ַ��������Ǹ�, �߶����ۼ�, ����ת�ļӿ��, û��ת�ļӸ߶�

    float width = 0.f, height = 0.f;

    CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                     FLOAT baselineOriginX,
                                     FLOAT baselineOriginY,
                                     DWRITE_MEASURING_MODE measuringMode,
                                     DWRITE_GLYPH_RUN const* glyphRun,
                                     DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                     IUnknown* clientDrawingEffect)
                                 {
                                     // ��ȡ�������
                                     for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
                                     {
                                         DWRITE_FONT_METRICS metrics{};
                                         glyphRun->fontFace->GetMetrics(&metrics);

                                         float designUnitsPerEm = (float)metrics.designUnitsPerEm;
                                         float _height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
                                         float _width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

                                         if (width < _width)
                                             width = _width; // ���ֻ��¼���Ŀ��, ������ʿ���ǹ̶���

                                         wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
                                         bool is_alpha = isLatinCharacter(ch);
                                         if (is_alpha)
                                             height += _width;   // ��Ҫ��ת���ַ��ͼ��Ͽ��
                                         else
                                             height += _height;  // ����ת���ַ��ͼ��ϸ߶�
                                     }

                                     return S_OK;
                                 }
    );
    pTextLayout->Draw(0, &renderer, 0, 0);

    *pHeight = height;
    return width;
}

NAMESPACE_LYRIC_DESKTOP_END
