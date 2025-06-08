#include "lyric_desktop_config.h"
#include <d2d/Color.h>


NAMESPACE_LYRIC_DESKTOP_BEGIN

void LYRIC_DESKTOP_CONFIG::default_config()
{
    refreshRate = 100;
    pszDefText = L"没有歌词时显示的文字, 可以打广告";
    nDefText = 17;
    padding_text_ = 5.f;
    padding_wnd_ = 8.f;
    padding_text = 0.f;
    padding_wnd = 0.f;
    mode = LYRIC_MODE::DOUBLE_ROW;

    pszFontName = L"微软雅黑";
    nFontSize = 24;
    lfWeight = 400;
    nLineSpace = 0;

    pos_h = {};
    pos_v = {};


    clrNormal =
    {
        MAKEARGB(255, 0,109,178),
        MAKEARGB(255, 3,189,241),
        MAKEARGB(255, 3,202,252),
    };
    clrLight =
    {
        MAKEARGB(255, 255,255,255),
        MAKEARGB(255, 130,247,253),
        MAKEARGB(255, 3, 233, 252),
    };
    clrBorder = MAKEARGB(255, 33, 33, 33);
    clrWndBack = MAKEARGB(100, 0, 0, 0);
    clrWndBorder = MAKEARGB(200, 0, 0, 0);

}

NAMESPACE_LYRIC_DESKTOP_END

