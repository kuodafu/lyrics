#include "lyric_desktop_header.h"
#include <d2d/Color.h>
#include <cJSON/cJSON.h>
#include <charset_stl.h>

NAMESPACE_LYRIC_DESKTOP_BEGIN


// 获取json中的值, 返回是否修改了 value的值
template<typename _Ty>
static bool _json_get_value(int& change, cJSON* json, const char* name, _Ty& value)
{
    cJSON* item = cJSON_GetObjectItem(json, name);
    if (!item)
        return false;
    auto vl = (_Ty)cJSON_GetNumberValue(item);
    if (vl <= _Ty() || vl == value)
        return false; // 值为空, 或者值一样, 不处理

    value = vl; // 有值, 且不为空, 才赋值给value
    change++;
    return true;
}
// 获取json中的值, 返回是否修改了 value的值
template<>
static bool _json_get_value(int& change, cJSON* json, const char* name, bool& value)
{
    cJSON* item = cJSON_GetObjectItem(json, name);
    if (!item)
        return false;
    if (!cJSON_IsBool(item))
        return false; // 不是bool类型, 不能赋值给value
    bool vl = cJSON_IsTrue(item);
    if (vl == value)
        return false;
    value = vl;
    change++;
    return true;
}
template<>
static bool _json_get_value<std::wstring>(int& change, cJSON* json, const char* name, std::wstring& value)
{
    cJSON* item = cJSON_GetObjectItem(json, name);
    if (!item)
        return false;
    LPCSTR str = cJSON_GetStringValue(item);
    if (!str || !*str)
        return false;

    auto w = charset_stl::U2W(str);
    if (w == value)
        return false;
    value = std::move(w);
    change++;
    return true;
}

template<typename _Ty>
static bool _json_get_arr(int& change, cJSON* json, const char* name, std::vector<_Ty>& arr)
{
    cJSON* json_arr = cJSON_GetObjectItem(json, name);
    if (!json_arr)
        return false; // json没有这个数组, 不给数组赋值

    int count = cJSON_GetArraySize(json_arr);
    if (!count)
        return false; // json数组没有值, 不给数组赋值

    arr.resize(count);
    for (int i = 0; i < count; ++i)
    {
        cJSON* item = cJSON_GetArrayItem(json_arr, i);
        if (!item)
            continue;
        arr[i] = (_Ty)cJSON_GetNumberValue(item);
    }
    change++;
    return true;
}
template<typename _Ty>
static bool _json_add_arr(cJSON* json, const char* name, const std::vector<_Ty>& arr)
{
    if (arr.empty())
        return false;
    cJSON* json_arr = cJSON_AddArrayToObject(json, name);
    if (!json_arr)
        return false;
    for (const auto& value : arr)
    {
        cJSON_AddItemToArray(json_arr, cJSON_CreateNumber((double)value));
    }
    return true;
}


void LYRIC_DESKTOP_CONFIG::init()
{
    //TODO: 可以在这里做一个宏判断, 根据宏来决定是显示广告的文字还是使用配置的文字
    refreshRate = 100;
    szDefText = L"没歌词时显示的文本, 可以打广告之类的, QQ: 121007124";

    padding_text_ = 5.f;
    padding_wnd_ = 8.f;
    padding_text = 0.f;
    padding_wnd = 0.f;
    line1_align = 0;
    line2_align = 2;

    bVertical = false;
    bSingleLine = false;
    bSelfy = false;
    bSelyy = false;


    szFontName = L"微软雅黑";
    nFontSize = 24;
    lfWeight = 700;
    nLineSpace = 0;
    strokeWidth = 2.2f;
    strokeWidth_div = 0;
    fillBeforeDraw = false;
    rect_h = { 300, 800, 1000, 1000 };
    rect_v = { 1000, 100, 1300, 800 };


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
    clrBorderNormal = MAKEARGB(255, 33, 33, 33);
    clrBorderLight = MAKEARGB(255, 33, 33, 33);
    clrWndBack = MAKEARGB(100, 0, 0, 0);
    clrWndBorder = MAKEARGB(200, 0, 0, 0);
    clrLine = MAKEARGB(100, 255, 255, 255);
}

_NODISCARD inline UINT _hash_str(const char* _First) noexcept
{
    if (!_First)
        return 0;
    UINT _FNV_offset_basis = 2166136261U;
    constexpr UINT _FNV_prime = 16777619U;
    while (*_First)
    {
        _FNV_offset_basis ^= static_cast<UINT>(*_First++);
        _FNV_offset_basis *= _FNV_prime;
    }

    return _FNV_offset_basis;
}

int LYRIC_DESKTOP_CONFIG::parse(const char* pszJson, LYRIC_DESKTOP_INFO* pWndInfo)
{
    auto dpi = (float)pWndInfo->scale.GetDpi();
    auto scale = [dpi](float x) -> float
        {
            return x * dpi / 96.0f;
        };


    pWndInfo->author_hash = 0;
    cJSON* json = cJSON_Parse(pszJson);
    if (json)
    {
        LPCSTR author = "";
        author = cJSON_GetStringValue(cJSON_GetObjectItem(json, "author"));
        pWndInfo->author_hash = _hash_str(author);
    }
    if (json == nullptr)
    {
        this->padding_text = scale(this->padding_text_);
        this->padding_wnd = scale(this->padding_wnd_);
        if (!pWndInfo->dx.pRender)
            pWndInfo->dx.init(pWndInfo);    // 需要初始化dx

        return 0;
    }


    int change = 0;

    //TODO: 这里加个条件, 然后显示广告文字
    _json_get_value(change, json, "refreshRate", this->refreshRate);
    _json_get_value(change, json, "bVertical", this->bVertical);
    _json_get_value(change, json, "bSingleLine", this->bSingleLine);
    _json_get_value(change, json, "bSelfy", this->bSelfy);
    _json_get_value(change, json, "bSelyy", this->bSelyy);

    _json_get_value(change, json, "szDefText", this->szDefText);

    //
    if (pWndInfo->author_hash != 1600659896)
        this->szDefText = L"没歌词时显示的文本, 可以打广告之类的, QQ: 121007124";

    _json_get_value(change, json, "padding_text", this->padding_text_);
    _json_get_value(change, json, "padding_wnd", this->padding_wnd_);
    _json_get_value(change, json, "strokeWidth", this->strokeWidth);
    _json_get_value(change, json, "strokeWidth_div", this->strokeWidth_div);
    _json_get_value(change, json, "fillBeforeDraw", this->fillBeforeDraw);
    _json_get_value(change, json, "nLineSpace", this->nLineSpace);

    cJSON* lyric_mode = cJSON_GetObjectItem(json, "lyric_mode");
    if (lyric_mode)
    {
        cJSON* line1 = cJSON_GetObjectItem(lyric_mode, "line1");
        cJSON* line2 = cJSON_GetObjectItem(lyric_mode, "line2");

        _json_get_value(change, line1, "align", this->line1_align);
        _json_get_value(change, line2, "align", this->line2_align);
    }

    this->padding_text = scale(this->padding_text_);
    this->padding_wnd  = scale(this->padding_wnd_);

    int font_change = change;
    _json_get_value(change, json, "szFontName", this->szFontName);
    _json_get_value(change, json, "nFontSize", this->nFontSize);
    _json_get_value(change, json, "lfWeight", this->lfWeight);
    if (font_change == change)
        font_change = 0;    // 如果font_change 不为0, 那就是要创建字体

    cJSON* rect_v = cJSON_GetObjectItem(json, "rect_v");
    cJSON* rect_h = cJSON_GetObjectItem(json, "rect_h");
    _json_get_value(change, rect_v, "left", this->rect_v.left);
    _json_get_value(change, rect_v, "top", this->rect_v.top);
    _json_get_value(change, rect_v, "right", this->rect_v.right);
    _json_get_value(change, rect_v, "bottom", this->rect_v.bottom);
    _json_get_value(change, rect_h, "left", this->rect_h.left);
    _json_get_value(change, rect_h, "top", this->rect_h.top);
    _json_get_value(change, rect_h, "right", this->rect_h.right);
    _json_get_value(change, rect_h, "bottom", this->rect_h.bottom);

    int clrNormal_change = change;
    _json_get_arr(change, json, "clrNormal", this->clrNormal);
    _json_get_arr(change, json, "clrNormal_GradientStop", this->clrNormal_GradientStop);
    if (clrNormal_change == change)
        clrNormal_change = 0;
    int clrLight_change = change;
    _json_get_arr(change, json, "clrLight", this->clrLight);
    _json_get_arr(change, json, "clrLight_GradientStop", this->clrLight_GradientStop);
    if (clrLight_change == change)
        clrLight_change = 0;

    _json_get_value(change, json, "clrBorderNormal", this->clrBorderNormal);
    _json_get_value(change, json, "clrBorderLight", this->clrBorderLight);
    _json_get_value(change, json, "clrWndBack", this->clrWndBack);
    _json_get_value(change, json, "clrWndBorder", this->clrWndBorder);
    _json_get_value(change, json, "clrLine", this->clrLine);

    cJSON* debug = cJSON_GetObjectItem(json, "debug");
    if (debug)
    {
        _json_get_value(change, debug, "clrTextBackNormal", this->debug.clrTextBackNormal);
        _json_get_value(change, debug, "clrTextBackLight", this->debug.clrTextBackLight);
        _json_get_value(change, debug, "alwaysFillBack", this->debug.alwaysFillBack);
        _json_get_value(change, debug, "alwaysDraw", this->debug.alwaysDraw);
        _json_get_value(change, debug, "alwaysCache", this->debug.alwaysCache);
        _json_get_value(change, debug, "alwaysCache1", this->debug.alwaysCache1);
    }

    cJSON_Delete(json);
    if (font_change)
        pWndInfo->dx.re_create_font(pWndInfo);

    if (!pWndInfo->dx.pRender)
    {
        // 没有创建那就直接初始化, 创建的时候也是使用的这个结构的值
        pWndInfo->dx.init(pWndInfo);
    }
    else
    {
        // 有创建了渲染对象就重新创建画刷
        if (clrNormal_change)
            pWndInfo->dx.re_create_brush(pWndInfo, false);
        if (clrLight_change)
            pWndInfo->dx.re_create_brush(pWndInfo, true);

        pWndInfo->dx.re_create_brush(pWndInfo->dx.hbrBorderNormal, this->clrBorderNormal);
        pWndInfo->dx.re_create_brush(pWndInfo->dx.hbrBorderLight, this->clrBorderLight);
        pWndInfo->dx.re_create_brush(pWndInfo->dx.hbrWndBorder, this->clrWndBorder);
        pWndInfo->dx.re_create_brush(pWndInfo->dx.hbrWndBack, this->clrWndBack);
        pWndInfo->dx.re_create_brush(pWndInfo->dx.hbrLine, this->clrLine);

    }

    return change;
}

char* LYRIC_DESKTOP_CONFIG::to_json(LYRIC_DESKTOP_INFO* pWndInfo) const
{
    cJSON* json = cJSON_CreateObject();
    if (json == nullptr)
        return nullptr;

    auto author = (LPCSTR)u8"kuodafu QQ: 121007124, group: 20752843";
    cJSON_AddStringToObject(json, "author", author);    // 1600659896
    cJSON_AddNumberToObject(json, "refreshRate", this->refreshRate);
    cJSON_AddBoolToObject(json, "bVertical", this->bVertical);
    cJSON_AddBoolToObject(json, "bSingleLine", this->bSingleLine);
    cJSON_AddBoolToObject(json, "bSelfy", this->bSelfy);
    cJSON_AddBoolToObject(json, "bSelyy", this->bSelyy);

    std::string szDefText = charset_stl::W2U(this->szDefText);
    cJSON_AddStringToObject(json, "szDefText", szDefText.c_str());


    cJSON_AddNumberToObject(json, "padding_text", this->padding_text_);
    cJSON_AddNumberToObject(json, "padding_wnd", this->padding_wnd_);
    cJSON_AddNumberToObject(json, "strokeWidth", this->strokeWidth);
    cJSON_AddNumberToObject(json, "strokeWidth_div", this->strokeWidth_div);
    cJSON_AddBoolToObject(json, "fillBeforeDraw", this->fillBeforeDraw);
    cJSON_AddNumberToObject(json, "nLineSpace", this->nLineSpace);

    cJSON* lyric_mode = cJSON_AddObjectToObject(json, "lyric_mode");
    cJSON* line1 = cJSON_AddObjectToObject(lyric_mode, "line1");
    cJSON* line2 = cJSON_AddObjectToObject(lyric_mode, "line2");
    cJSON_AddNumberToObject(line1, "align", this->line1_align);
    cJSON_AddNumberToObject(line2, "align", this->line2_align);

    std::string szFontName = charset_stl::W2U(this->szFontName);
    cJSON_AddStringToObject(json, "szFontName", szFontName.c_str());
    cJSON_AddNumberToObject(json, "nFontSize", this->nFontSize);
    cJSON_AddNumberToObject(json, "lfWeight", this->lfWeight);


    RECT rc_h = this->rect_h;
    RECT rc_v = this->rect_v;
    pWndInfo->scale.rerect(rc_h);   // 缩放回100%的尺寸
    pWndInfo->scale.rerect(rc_v);

    cJSON* rect_v = cJSON_AddObjectToObject(json, "rect_v");
    cJSON_AddNumberToObject(rect_v, "left", rc_v.left);
    cJSON_AddNumberToObject(rect_v, "top", rc_v.top);
    cJSON_AddNumberToObject(rect_v, "right", rc_v.right);
    cJSON_AddNumberToObject(rect_v, "bottom", rc_v.bottom);

    cJSON* rect_h = cJSON_AddObjectToObject(json, "rect_h");
    cJSON_AddNumberToObject(rect_h, "left", rc_h.left);
    cJSON_AddNumberToObject(rect_h, "top", rc_h.top);
    cJSON_AddNumberToObject(rect_h, "right", rc_h.right);
    cJSON_AddNumberToObject(rect_h, "bottom", rc_h.bottom);


    _json_add_arr(json, "clrNormal", this->clrNormal);
    _json_add_arr(json, "clrNormal_GradientStop", this->clrNormal_GradientStop);
    _json_add_arr(json, "clrLight", this->clrLight);
    _json_add_arr(json, "clrLight_GradientStop", this->clrLight_GradientStop);

    cJSON_AddNumberToObject(json, "clrBorderNormal", this->clrBorderNormal);
    cJSON_AddNumberToObject(json, "clrBorderLight", this->clrBorderLight);
    cJSON_AddNumberToObject(json, "clrWndBack", this->clrWndBack);
    cJSON_AddNumberToObject(json, "clrWndBorder", this->clrWndBorder);
    cJSON_AddNumberToObject(json, "clrLine", this->clrLine);

    cJSON* debug = cJSON_AddObjectToObject(json, "debug");
    cJSON_AddNumberToObject(debug, "clrTextBackNormal", this->debug.clrTextBackNormal);
    cJSON_AddNumberToObject(debug, "clrTextBackLight", this->debug.clrTextBackLight);
    cJSON_AddBoolToObject(debug, "alwaysFillBack", this->debug.alwaysFillBack);
    cJSON_AddBoolToObject(debug, "alwaysDraw", this->debug.alwaysDraw);
    cJSON_AddBoolToObject(debug, "alwaysCache", this->debug.alwaysCache);
    cJSON_AddBoolToObject(debug, "alwaysCache1", this->debug.alwaysCache1);

    char* pszJson = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    size_t len = strlen(pszJson);
    auto ret = (char*)malloc(len + 1);  // 明确指明这里的内存是malloc分配的
    if (ret)
    {
        memcpy(ret, pszJson, len);
        ret[len] = '\0';
    }
    cJSON_free(pszJson);
    return ret;
}


NAMESPACE_LYRIC_DESKTOP_END

