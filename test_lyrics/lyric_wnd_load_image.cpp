#include "lyric_wnd_function.h"
#include "resource.h"
#include "tinyxml2.h"

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

bool lyric_wnd_load_image_parse(LYRIC_WND_INFU& wnd_info, tinyxml2::XMLNode* node);



// �п��ܱ���ε���, ������Ҫÿ�ζ����һ��, �豸ʧЧ��ʱ������µ���
bool lyric_wnd_load_image(LYRIC_WND_INFU& wnd_info)
{
    // ��ȡ��Դ
    auto pfn_get_res = [](int id, LPCWSTR type, DWORD* pSize) -> LPCSTR
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
    };

    DWORD xml_size = 0, png_size = 0;
    LPCSTR xml = pfn_get_res(IDR_XML_LYRIC, L"XML", &xml_size);
    LPCSTR png = pfn_get_res(IDR_PNG_LYRIC, L"PNG", &png_size);

    wnd_info.dx.image = new CD2DImage(*wnd_info.dx.hCanvas, png, png_size);
    wnd_info.button.rcSrc.reserve(30);
    wnd_info.button.rcBtn.reserve(30);
    wnd_info.button.rcSrc.clear();
    wnd_info.button.rcBtn.clear();
#define _MAKE(_s) { _s, 0, 0, {}, nullptr }
    // ������Ƕ��尴ť�滭��˳������, ���ص�ʱ����һ��, �����ʱ������id, 
    wnd_info.button.rcBtn =
    {
        _MAKE(LYRIC_WND_BUTTON_ID_VERTICAL  ),// ������ť
        _MAKE(LYRIC_WND_BUTTON_ID_PREV      ),// ��һ��
        _MAKE(LYRIC_WND_BUTTON_ID_PLAY      ),// ����
        _MAKE(LYRIC_WND_BUTTON_ID_NEXT      ),// ��һ��
        _MAKE(0 ),   // �Ӹ��ָ���
        _MAKE(LYRIC_WND_BUTTON_ID_FONT_UP   ),// ��������
        _MAKE(LYRIC_WND_BUTTON_ID_FONT_DOWN ),// �����С
        _MAKE(LYRIC_WND_BUTTON_ID_LRCCOLOR  ),// ����������ɫ, ���ֵİ�ťͼ��
        _MAKE(LYRIC_WND_BUTTON_ID_SETTING   ),// ���ð�ť
        _MAKE(LYRIC_WND_BUTTON_ID_BEHIND    ),// ����Ӻ�
        _MAKE(LYRIC_WND_BUTTON_ID_AHEAD     ),// �����ǰ
        _MAKE(0 ),   // �Ӹ��ָ���
        _MAKE(LYRIC_WND_BUTTON_ID_LRCWRONG  ),// ��ʲ���
        _MAKE(0 ),   // �Ӹ��ָ���
        _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE1),// ���밴ť
        _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE2),// ���밴ť
        _MAKE(0 ),   // �Ӹ��ָ���
        _MAKE(LYRIC_WND_BUTTON_ID_LOCK      ),// ������ť
        _MAKE(LYRIC_WND_BUTTON_ID_CLOSE     ),// �رհ�ť

    };
#undef _MAKE


    using namespace tinyxml2;
    // ����XML
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

    // ����Ĭ�ϰ�ť��Ϣ��, ����ť����һЩ״̬
    int language = lyric_get_language(wnd_info.hLyric);

    LYRIC_WND_BUTTON_STATE b1 = __query(language, 1) ? LYRIC_WND_BUTTON_STATE_NORMAL : LYRIC_WND_BUTTON_STATE_DISABLE;
    LYRIC_WND_BUTTON_STATE b2 = __query(language, 2) ? LYRIC_WND_BUTTON_STATE_NORMAL : LYRIC_WND_BUTTON_STATE_DISABLE;

    lyric_wnd_set_btn_state(wnd_info, LYRIC_WND_BUTTON_ID_TRANSLATE1, b1);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_WND_BUTTON_ID_TRANSLATE2, b2);

    const int _10 = wnd_info.scale(10);
    const int _20 = _10 * 2;
    const int _50 = _10 * 5;

    const int offset = _10;
    wnd_info.button.width = lyric_wnd_calc_button(wnd_info, wnd_info.button.maxHeight, offset);

    lyric_wnd_calc_height(wnd_info);

    return true;
}

void lyric_wnd_calc_height(LYRIC_WND_INFU& wnd_info)
{
    const int _10 = wnd_info.scale(10);
    const int _20 = _10 * 2;
    const int _50 = _10 * 5;

    wnd_info.nLineTop1 = wnd_info.button.maxHeight + _20;
    wnd_info.nLineTop2 = wnd_info.nLineTop1 + (int)wnd_info.nLineHeight + _10;
    wnd_info.nMinWidth = wnd_info.button.width + _50;
    wnd_info.nMinHeight = wnd_info.nLineTop2 + (int)wnd_info.nLineHeight + _10;
}
// ����xml���λ����Ϣ, ��¼����
bool lyric_wnd_load_image_parse(LYRIC_WND_INFU& wnd_info, tinyxml2::XMLNode* node)
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



int lyric_wnd_calc_button(LYRIC_WND_INFU& wnd_info, int& maxHeight, int offset)
{
    int width = 0;
    maxHeight = 0;
    const int offset2 = offset / 2;

    int left = 0;
    int index = -1;
    for (LYRIC_WND_BUTTON_INFO& item : wnd_info.button.rcBtn)
    {
        index++;
        item.prcSrc = nullptr;
        if (item.id == 0)
        {
            item.rc.left = left + offset2;
            left += offset;
            width += offset;
        }
        else
        {
            auto& item_src = wnd_info.button.rcSrc[item.id - LYRIC_WND_BUTTON_ID_FIRST];
            if (wnd_info.button.indexDown == index)
            {
                // ��ǰ��ť�ǰ���״̬
                item.prcSrc = &item_src.rcDown;
            }
            else if (wnd_info.button.index == index)
            {
                // ��ǰ��ť���ȵ�״̬
                item.prcSrc = &item_src.rcLight;
            }
            else if (__query(item.state, LYRIC_WND_BUTTON_STATE_DISABLE))
            {
                // ��ǰ��ť�ǽ�ֹ״̬
                item.prcSrc = &item_src.rcDisable;
            }
            else
            {
                item.prcSrc = &item_src.rcNormal;   // ʣ�µľ��ǽ�ֹ״̬
            }
            const RECT& rc = *item.prcSrc;
            const int btn_width = rc.right - rc.left;
            const int btn_height = rc.bottom - rc.top;
            if (maxHeight < btn_height)
                maxHeight = btn_height;

            item.rc.left = left;
            left += btn_width + offset;
            width += btn_width + offset;
        }

    }
    return width;
}





NAMESPACE_LYRIC_WND_END


