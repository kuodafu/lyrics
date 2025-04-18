#include "CD2DFont.h"
#include <string>
#include <charset_stl.h>

NAMESPACE_D2D_BEGIN

CD2DFont::CD2DFont(LPCWSTR name, LONG lfHeight, FONTSTYLE fontStyle)
{
    m_dxFormat = nullptr;
    m_pWriteInlineObject = nullptr;

    SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTW), &m_logFont, 0);
    // ÅÐ¶Ï ÊÇ·ñÆôÓÃdpiËõ·Å,logFont.lfHeight /= dpiY;
    if (name)
        memcpy(m_logFont.lfFaceName, name, (wcslen(name) + 1) * sizeof(wchar_t));
    if (lfHeight != 0)
        m_logFont.lfHeight = lfHeight;
    int style = (int)fontStyle;
    if (style)
    {
        m_logFont.lfWeight = (style & (int)FONTSTYLE::FontStyleBold) ? FW_BOLD : FW_NORMAL;
        m_logFont.lfItalic = (style & (int)FONTSTYLE::FontStyleItalic) ? 1 : 0;
        m_logFont.lfUnderline = (style & (int)FONTSTYLE::FontStyleUnderline) ? 1 : 0;
        m_logFont.lfStrikeOut = (style & (int)FONTSTYLE::FontStyleStrikeout) ? 1 : 0;
    }
    create(&m_logFont);
}

CD2DFont::CD2DFont(const LOGFONTA* logFont)
{
    m_dxFormat = nullptr;
    m_pWriteInlineObject = nullptr;
    auto w = (charset_stl::A2W)(logFont->lfFaceName);

    LOGFONTW lf = { 0 };
    memcpy(&lf, logFont, sizeof(LOGFONTA));
    wcscpy_s(lf.lfFaceName, w.c_str());
    create(&lf);
}

CD2DFont::CD2DFont(const LOGFONTW* logFont)
{
    m_dxFormat = nullptr;
    m_pWriteInlineObject = nullptr;

    create(logFont);
}

CD2DFont::~CD2DFont()
{
    if (m_dxFormat)
        m_dxFormat->Release();
    if (m_pWriteInlineObject)
        m_pWriteInlineObject->Release();
}

bool CD2DFont::create(const LOGFONTW* logFont)
{
    memcpy(&m_logFont, logFont, sizeof(LOGFONTW));

    if (m_logFont.lfWeight == 0)
        m_logFont.lfWeight = FW_NORMAL;

    static wchar_t pLocalName[260];
    if (pLocalName[0] == 0)
        GetUserDefaultLocaleName(pLocalName, 260);

    int lfHeight = (logFont->lfHeight > 0) ? logFont->lfHeight : (logFont->lfHeight * -1);

    auto& d2dInfo = d2d_get_info();
    HRESULT hr = d2dInfo.pDWriteFactory->CreateTextFormat(
        logFont->lfFaceName,
        0,
        (DWRITE_FONT_WEIGHT)logFont->lfWeight,
        (DWRITE_FONT_STYLE)logFont->lfItalic,
        DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL,
        static_cast<FLOAT>(lfHeight),
        pLocalName,
        &m_dxFormat);


    if (SUCCEEDED(hr))
        hr = d2dInfo.pDWriteFactory->CreateEllipsisTrimmingSign(m_dxFormat, &m_pWriteInlineObject);

    return SUCCEEDED(hr);
}






NAMESPACE_D2D_END

