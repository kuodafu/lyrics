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
    // ������Ƕ��尴ť�滭��˳������, ���ص�ʱ����һ��, �����ʱ������id, 
    if (wnd_info.has_mode(LYRIC_MODE::VERTICAL))
    {
        // ����ģʽ, �м�����ť��Ҫ��
        wnd_info.button.rcBtn =
        {
            _MAKE(LYRIC_WND_BUTTON_ID_HORIZONTAL),// ������ť
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
            _MAKE(LYRIC_WND_BUTTON_ID_LRCWRONG_V),// ��ʲ���
            _MAKE(0 ),   // �Ӹ��ָ���
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE1),// ���밴ť
            _MAKE(LYRIC_WND_BUTTON_ID_TRANSLATE2),// ���밴ť
            _MAKE(0 ),   // �Ӹ��ָ���
            _MAKE(LYRIC_WND_BUTTON_ID_LOCK      ),// ������ť
            _MAKE(LYRIC_WND_BUTTON_ID_CLOSE     ),// �رհ�ť

        };
    }
    else
    {
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
    }

#undef _MAKE

    if (oldSize > 0)
        wnd_info.button.rcBtn[2].id = old_play_id;

    if (wnd_info.button.rcSrc.empty())
    {
        // ��û����xml������, ��Ҫ����һ��
        _lyric_parse_xml(wnd_info);
    }

    // ����Ĭ�ϰ�ť��Ϣ��, ����ť����һЩ״̬
    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(wnd_info, language);

    lyric_wnd_calc_btn_pos(wnd_info);
    lyric_wnd_calc_wnd_pos(wnd_info, false);
    return true;
}

// �п��ܱ���ε���, ������Ҫÿ�ζ����һ��, �豸ʧЧ��ʱ������µ���
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
// ����xml���λ����Ϣ, ��¼����
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


