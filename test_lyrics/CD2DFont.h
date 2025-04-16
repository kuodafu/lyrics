#pragma once
#include "d2d.h"

NAMESPACE_D2D_BEGIN

// ������ʽ
enum class FONTSTYLE
{
    FontStyleRegular    = 0,    // ����, Ĭ��
    FontStyleBold       = 1,    // ����
    FontStyleItalic     = 2,    // б��
    FontStyleBoldItalic = 3,    // �����б��
    FontStyleUnderline  = 4,    // �»���
    FontStyleStrikeout  = 8     // ɾ����
};

class CD2DFont
{
    IDWriteInlineObject* m_pWriteInlineObject;    // �����Ű�
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