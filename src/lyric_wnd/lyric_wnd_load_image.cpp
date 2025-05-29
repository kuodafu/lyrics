#include "lyric_wnd_function.h"
#include "resource.h"
#include "tinyxml2.h"

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

bool _lyric_parse_xml(LYRIC_WND_INFO& wnd_info);
LPCSTR _lyric_get_resource(int id, LPCWSTR type, DWORD* pSize);


bool lyric_wnd_load_image_parse(LYRIC_WND_INFO& wnd_info, tinyxml2::XMLNode* node);

bool lyric_wnd_load_image_recalc(LYRIC_WND_INFO& wnd_info)
{
    if (!wnd_info.dx.image)
        lyric_wnd_load_image(wnd_info);

    if (!wnd_info.dx.image)
        return false;

    size_t oldSize = wnd_info.button.rcBtn.size();
    int old_play_id = 0;
    if (oldSize > 0)
        old_play_id = wnd_info.button.rcBtn[2].id;
    
#define _MAKE(_s) { _s, 0, 0, {}, nullptr }
    // 这里就是定义按钮绘画的顺序索引, 加载的时候定义一次, 点击的时候会更改id, 
    if (wnd_info.has_mode(LYRIC_MODE::VERTICAL))
    {
        // 竖屏模式, 有几个按钮需要改
        wnd_info.button.rcBtn =
        {
            _MAKE(LYRIC_WND_BUTTON_ID_HORIZONTAL),// 竖屏按钮
            _MAKE(LYRIC_WND_BUTTON_ID_PREV      ),// 上一首
            _MAKE(LYRIC_WND_BUTTON_ID_PLAY      ),// 播放
            _MAKE(LYRIC_WND_BUTTON_ID_NEXT      ),// 上一首
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_FONT_UP   ),// 字体增加
            _MAKE(LYRIC_WND_BUTTON_ID_FONT_DOWN ),// 字体减小
            _MAKE(LYRIC_WND_BUTTON_ID_LRCCOLOR  ),// 设置字体颜色, 田字的按钮图标
            _MAKE(LYRIC_WND_BUTTON_ID_SETTING   ),// 设置按钮
            _MAKE(LYRIC_WND_BUTTON_ID_BEHIND    ),// 歌词延后
            _MAKE(LYRIC_WND_BUTTON_ID_AHEAD     ),// 歌词提前
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_LRCWRONG_V),// 歌词不对
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE1),// 翻译按钮
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE2),// 音译按钮
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_LOCK      ),// 锁定按钮
            _MAKE(LYRIC_WND_BUTTON_ID_CLOSE     ),// 关闭按钮

        };
    }
    else
    {
        wnd_info.button.rcBtn =
        {
            _MAKE(LYRIC_WND_BUTTON_ID_VERTICAL  ),// 竖屏按钮
            _MAKE(LYRIC_WND_BUTTON_ID_PREV      ),// 上一首
            _MAKE(LYRIC_WND_BUTTON_ID_PLAY      ),// 播放
            _MAKE(LYRIC_WND_BUTTON_ID_NEXT      ),// 上一首
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_FONT_UP   ),// 字体增加
            _MAKE(LYRIC_WND_BUTTON_ID_FONT_DOWN ),// 字体减小
            _MAKE(LYRIC_WND_BUTTON_ID_LRCCOLOR  ),// 设置字体颜色, 田字的按钮图标
            _MAKE(LYRIC_WND_BUTTON_ID_SETTING   ),// 设置按钮
            _MAKE(LYRIC_WND_BUTTON_ID_BEHIND    ),// 歌词延后
            _MAKE(LYRIC_WND_BUTTON_ID_AHEAD     ),// 歌词提前
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_LRCWRONG  ),// 歌词不对
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE1),// 翻译按钮
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE2),// 音译按钮
            _MAKE(0 ),   // 加个分割条
            _MAKE(LYRIC_WND_BUTTON_ID_LOCK      ),// 锁定按钮
            _MAKE(LYRIC_WND_BUTTON_ID_CLOSE     ),// 关闭按钮

        };
    }

#undef _MAKE

    if (oldSize > 0)
        wnd_info.button.rcBtn[2].id = old_play_id;

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
bool lyric_wnd_load_image(LYRIC_WND_INFO& wnd_info)
{
    DWORD png_size = 0;
    LPCSTR png = _lyric_get_resource(IDR_PNG_LYRIC, L"PNG", &png_size);
    wnd_info.dx.image = new CD2DImage(*wnd_info.dx.hCanvas, png, png_size);
    wnd_info.button.rcSrc.clear();
    wnd_info.button.rcBtn.clear();
    return lyric_wnd_load_image_recalc(wnd_info);
}


bool _lyric_parse_xml(LYRIC_WND_INFO& wnd_info)
{
    DWORD xml_size = 0;
    LPCSTR xml = _lyric_get_resource(IDR_XML_LYRIC, L"XML", &xml_size);

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
LPCSTR _lyric_get_resource(int id, LPCWSTR type, DWORD* pSize)
{
    HMODULE hModule = GetModuleHandleW(nullptr);
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(id), type);
    if (!hRes)
        return nullptr;

    HGLOBAL hGlobal = LoadResource(hModule, hRes);
    if (hGlobal)
    {
        *pSize = SizeofResource(hModule, hRes);
        return static_cast<const char*>(LockResource(hGlobal));
    }
    return nullptr;
}
// 解析xml里的位置信息, 记录起来
bool lyric_wnd_load_image_parse(LYRIC_WND_INFO& wnd_info, tinyxml2::XMLNode* node)
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






NAMESPACE_LYRIC_WND_END


