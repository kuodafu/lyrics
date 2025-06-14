#include "lyric_desktop_function.h"
#include "resource.h"
#include <tinyxml2.h>

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

bool _lyric_parse_xml(LYRIC_DESKTOP_INFO& wnd_info);

bool lyric_wnd_load_image_parse(LYRIC_DESKTOP_INFO& wnd_info, tinyxml2::XMLNode* node);

bool lyric_wnd_load_image_recalc(LYRIC_DESKTOP_INFO& wnd_info)
{
    if (!wnd_info.dx.image_button)
        wnd_info.dx.re_create_image(&wnd_info);

    if (!wnd_info.dx.image_button)
        return false;

    struct OLD_BUTTON_ID
    {
        int id;
        int old_id;
        int index;
    };

    // 保存几个按钮原来的状态
    OLD_BUTTON_ID old_id[] =
    {
        { LYRIC_DESKTOP_BUTTON_ID_PLAY, 0, -1 },
        { LYRIC_DESKTOP_BUTTON_ID_PAUSE, 0, -1 },
        { LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY, 0, -1 },
        { LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY, 0, -1 },
        { LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY_SEL, 0, -1 },
        { LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY_SEL, 0, -1 },
    };

    int index = -1;
    for (auto& item : wnd_info.button.rcBtn)
    {
        index++;
        auto it = std::find_if(&old_id[0], &old_id[0] + _countof(old_id), [&item](const OLD_BUTTON_ID& s) { return s.id == item.id; });
        if (it != &old_id[0] + _countof(old_id))
            it->old_id = item.id, it->index = index;
    }

    
#define _MAKE(_s) { _s, 0, 0, {}, nullptr }
    // 这里就是定义按钮绘画的顺序索引, 加载的时候定义一次, 点击的时候会更改id, 
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
    {
        // 竖屏模式, 有几个按钮需要改
        wnd_info.button.rcBtn =
        {
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_HORIZONTAL),// 竖屏按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_PREV      ),// 上一首
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_PLAY      ),// 播放
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_NEXT      ),// 上一首
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_FONT_UP   ),// 字体增加
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN ),// 字体减小
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR  ),// 设置字体颜色, 田字的按钮图标
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_SETTING   ),// 设置按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_BEHIND    ),// 歌词延后
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_AHEAD     ),// 歌词提前
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LRCWRONG_V),// 歌词不对
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY),// 翻译按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY),// 音译按钮
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LOCK      ),// 锁定按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_CLOSE     ),// 关闭按钮

        };
    }
    else
    {
        wnd_info.button.rcBtn =
        {
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_VERTICAL  ),// 竖屏按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_PREV      ),// 上一首
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_PLAY      ),// 播放
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_NEXT      ),// 上一首
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_FONT_UP   ),// 字体增加
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN ),// 字体减小
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR  ),// 设置字体颜色, 田字的按钮图标
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_SETTING   ),// 设置按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_BEHIND    ),// 歌词延后
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_AHEAD     ),// 歌词提前
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LRCWRONG  ),// 歌词不对
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY),// 翻译按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY),// 音译按钮
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_LOCK      ),// 锁定按钮
            _MAKE(LYRIC_DESKTOP_BUTTON_ID_CLOSE     ),// 关闭按钮

        };
    }

#undef _MAKE

    for (auto& item : old_id)
    {
        if(item.index != -1)
            wnd_info.button.rcBtn[item.index].id = item.old_id;
    }

    if (wnd_info.button.rcSrc.empty())
    {
        // 还没加载xml的数据, 需要加载一下
        _lyric_parse_xml(wnd_info);
    }

    // 设置默认按钮信息后, 给按钮设置一些状态
    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(wnd_info, language);

    lyric_wnd_calc_btn_pos(wnd_info);
    lyric_wnd_calc_wnd_pos(wnd_info, false);
    return true;
}

// 有可能被多次调用, 所以需要每次都清空一下, 设备失效的时候会重新调用
bool lyric_wnd_load_image(LYRIC_DESKTOP_INFO& wnd_info)
{
    wnd_info.button.rcSrc.clear();
    wnd_info.button.rcBtn.clear();
    return lyric_wnd_load_image_recalc(wnd_info);
}


bool _lyric_parse_xml(LYRIC_DESKTOP_INFO& wnd_info)
{
    const LPCSTR xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<ImageConfig author="福仔" QQ="121007124" url="https://www.kuodafu.com">
    <common id="1" normal="0,0,18,23" hot="18,0,36,23" down="36,0,54,23" disable="54,0,72,23"/>
    <common id="2" normal="72,0,90,23" hot="90,0,108,23" down="108,0,126,23" disable="126,0,144,23"/>
    <common id="3" normal="144,0,162,23" hot="162,0,180,23" down="180,0,198,23" disable="198,0,216,23"/>
    <common id="4" normal="216,0,234,23" hot="234,0,252,23" down="252,0,270,23" disable="270,0,288,23"/>
    <common id="5" normal="288,0,346,23" hot="346,0,404,23" down="404,0,462,23" disable="462,0,520,23"/>
    <common id="6" normal="520,0,550,23" hot="550,0,580,23" down="580,0,610,23" disable="610,0,640,23"/>
    <common id="7" normal="640,0,698,23" hot="698,0,756,23" down="756,0,814,23" disable="814,0,872,23"/>
    <common id="8" normal="0,23,25,46" hot="25,23,50,46" down="50,23,75,46" disable="75,23,100,46"/>
    <common id="9" normal="100,23,125,46" hot="125,23,150,46" down="150,23,175,46" disable="175,23,200,46"/>
    <common id="10" normal="200,23,223,46" hot="223,23,246,46" down="246,23,269,46" disable="269,23,292,46"/>
    <common id="11" normal="292,23,315,46" hot="315,23,338,46" down="338,23,361,46" disable="361,23,384,46"/>
    <common id="12" normal="384,23,407,46" hot="407,23,430,46" down="430,23,453,46" disable="453,23,476,46"/>
    <common id="13" normal="476,23,498,46" hot="498,23,520,46" down="520,23,542,46" disable="542,23,564,46"/>
    <common id="14" normal="564,23,584,43" hot="584,23,604,43" down="604,23,624,43" disable="624,23,644,43"/>
    <common id="15" normal="644,23,662,41" hot="662,23,680,41" down="680,23,698,41" disable="698,23,716,41"/>
    <common id="16" normal="716,23,736,43" hot="736,23,756,43" down="756,23,776,43" disable="776,23,796,43"/>
    <common id="17" normal="796,23,818,45" hot="818,23,840,45" down="840,23,862,45" disable="862,23,884,45"/>
    <common id="18" normal="0,46,23,104" hot="23,46,46,104" down="46,46,69,104" disable="69,46,92,104"/>
    <common id="19" normal="92,46,115,77" hot="115,46,138,77" down="138,46,161,77" disable="161,46,184,77"/>
    <common id="20" normal="184,46,234,96" hot="234,46,284,96" down="284,46,334,96" disable="334,46,384,96"/>
    <common id="21" normal="384,46,434,96" hot="434,46,484,96" down="484,46,534,96" disable="534,46,584,96"/>
    <common id="22" normal="584,46,624,86" hot="624,46,664,86" down="664,46,704,86" disable="704,46,744,86"/>
    <common id="23" normal="744,46,784,86" hot="784,46,824,86" down="824,46,864,86" disable="864,46,904,86"/>
</ImageConfig>
)";
    const size_t xml_size = strlen(xml);

    using namespace tinyxml2;
    // 解析XML
    tinyxml2::XMLDocument doc;
    XMLError err = doc.Parse(xml, xml_size);
    if (err != XML_SUCCESS)
        return false;

    auto* root = doc.RootElement();
    if (!root)
        return false;

    XMLNode* node = root->FirstChild();
    if (!node)
        return false;

    while (node)
    {
        lyric_wnd_load_image_parse(wnd_info, node);
        node = node->NextSibling();
    }
    return true;
}

// 解析xml里的位置信息, 记录起来
bool lyric_wnd_load_image_parse(LYRIC_DESKTOP_INFO& wnd_info, tinyxml2::XMLNode* node)
{
    tinyxml2::XMLElement* ele = node->ToElement();

    if (!ele)
        return false;

    const tinyxml2::XMLAttribute* node_normal = ele->FindAttribute("normal");
    const tinyxml2::XMLAttribute* node_highlight = ele->FindAttribute("hot");
    const tinyxml2::XMLAttribute* node_down = ele->FindAttribute("down");
    const tinyxml2::XMLAttribute* node_disable = ele->FindAttribute("disable");

    auto pfn_get_rect = [ele](LPCSTR name, RECT& rect)
    {
        rect = { 0 };
        const tinyxml2::XMLAttribute* node = ele->FindAttribute(name);
        if (!node)
            return;
        LPCSTR str = node->Value();
        if (str)
            sscanf_s(str, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
        
    };

    auto& img_info = wnd_info.button.rcSrc.emplace_back();

    pfn_get_rect("normal"   , img_info.rcNormal);
    pfn_get_rect("hot"      , img_info.rcLight);
    pfn_get_rect("down"     , img_info.rcDown);
    pfn_get_rect("disable"  , img_info.rcDisable);

    return true;
}






NAMESPACE_LYRIC_DESKTOP_END


