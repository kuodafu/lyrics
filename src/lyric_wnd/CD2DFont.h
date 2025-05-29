#pragma once
#include "d2d.h"

NAMESPACE_D2D_BEGIN

// 字体样式
enum class FONTSTYLE
{
    FontStyleRegular    = 0,    // 正常, 默认
    FontStyleBold       = 1,    // 粗体
    FontStyleItalic     = 2,    // 斜体
    FontStyleBoldItalic = 3,    // 粗体和斜体
    FontStyleUnderline  = 4,    // 下划线
    FontStyleStrikeout  = 8     // 删除线
};

class CD2DFont
{
    IDWriteInlineObject* m_pWriteInlineObject;    // 文字排版
    IDWriteTextFormat* m_dxFormat;
    LOGFONTW m_logFont;

public:
    CD2DFont(LPCWSTR name, LONG lfHeight, FONTSTYLE fontStyle);
    CD2DFont(const LOGFONTA* logFont);
    CD2DFont(const LOGFONTW* logFont);
    ~CD2DFont();
    operator IDWriteInlineObject* () { return m_pWriteInlineObject; }
    operator IDWriteTextFormat* () { return m_dxFormat; }
    operator LOGFONTW& () { return m_logFont; }

private:
    bool create(const LOGFONTW* logFont);
};


NAMESPACE_D2D_END