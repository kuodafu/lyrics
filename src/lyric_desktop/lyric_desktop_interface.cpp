/*
* ����ļ��������ʵ����Ľӿ�, ���нӿڶ�д������
* ���ǽӿں�������������д
*/

#include "lyric_wnd_function.h"
#include <CommCtrl.h>

using namespace NAMESPACE_LYRIC_DESKTOP;
using namespace NAMESPACE_D2D;

/// <summary>
/// ��ʼ��������, ���ʼ��D2D, ����DPI����, ע�ᴰ�����
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_init()
{
    return _ld_init();
}

/// <summary>
/// ȡ����ʼ��, ж��D2D, �ͷŸ�����Դ
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_uninit()
{
    return _ld_uninit();
}

/// <summary>
/// ��ȡ�������ڵ�Ĭ�ϲ���
/// </summary>
/// <param name="arg">����Ĭ�ϲ����Ľṹ</param>
void LYRICCALL lyric_desktop_get_default_arg(LYRIC_DESKTOP_ARG* arg)
{
    //static DWORD clrNormal[] =
    //{
    //    MAKEARGB(255, 0,52,138),
    //    MAKEARGB(255, 0,128,192),
    //    MAKEARGB(255, 3,202,252),
    //};

    static DWORD clrNormal[] =
    {
        MAKEARGB(255, 0,109,178),
        MAKEARGB(255, 3,189,241),
        MAKEARGB(255, 3,202,252),
    };

    static DWORD clrLight[] =
    {
        MAKEARGB(255, 255,255,255),
        MAKEARGB(255, 130,247,253),
        MAKEARGB(255, 3, 233, 252),
    };

    arg->clrWndBack = MAKEARGB(100, 0, 0, 0);
    arg->clrWndBorder = MAKEARGB(200, 0, 0, 0);
    arg->nFontSize = 24;
    arg->pszFontName = L"΢���ź�";
    arg->clrBorder = MAKEARGB(255, 33, 33, 33);

    arg->pClrNormal = clrNormal;
    arg->nClrNormal = _countof(clrNormal);

    arg->pClrLight = clrLight;
    arg->nClrLight = _countof(clrLight);

    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    const int cxScreen = rc.right - rc.left;
    const int cyScreen = rc.bottom - rc.top;

    const int width = 900;
    const int height = 200;
    arg->rcWindow.left = (cxScreen - width) / 2;
    arg->rcWindow.top = (cyScreen - height - 100);
    arg->rcWindow.right = arg->rcWindow.left + width;
    arg->rcWindow.bottom = arg->rcWindow.top + height;

}

/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <param name="arg">������ʴ��ڵĲ���, ����, ����λ��, ��ɫ����Ϣ</param>
/// <param name="pfnCommand">��ť������ص�����</param>
/// <param name="lParam">���ݵ� pfnCommand() ������Ĳ���</param>
/// <returns>���ش��ھ��</returns>
HWND LYRICCALL lyric_desktop_create(const LYRIC_DESKTOP_ARG* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _ld_create_layered_window(arg);
    if (!pWndInfo)
        return nullptr;
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    HWND hWnd = wnd_info.hWnd;

    RECT rcDesk;
    GetWindowRect(GetDesktopWindow(), &rcDesk);
    const int cxScreen = rcDesk.right - rcDesk.left;
    const int cyScreen = rcDesk.bottom - rcDesk.top;

    wnd_info.hWnd = hWnd;
    wnd_info.pfnCommand = pfnCommand;
    wnd_info.lParam = lParam;
    wnd_info.line1.align = 0;
    wnd_info.line2.align = 2;
    wnd_info.mode = LYRIC_MODE::DOUBLE_ROW;

    // λ����صķ�һ����
    wnd_info.dpi_change(hWnd);
    wnd_info.get_monitor();

    RECT rc = arg->rcWindow;
    wnd_info.scale(rc);

    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;

    if (rc.top > cyScreen - wnd_info.scale(250))
        rc.top = cyScreen - wnd_info.scale(250), rc.bottom = rc.top + height;
    if (rc.left < rcDesk.left)
        rc.left = (cxScreen - width) / 2, rc.right = rc.left + width;

    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);




    wnd_info.set_def_arg(arg);
    lyric_wnd_default_object(wnd_info);

    //TODO ���ﴴ���߾��ȶ�ʱ��, ����ˢ����������ˢ�¼��


    _ld_start_high_precision_timer(pWndInfo);

    if (wnd_info.nMinHeight)
        MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, wnd_info.nMinHeight, TRUE);

    return hWnd;
}

/// <summary>
/// ��ʴ��ڼ��ظ������, �����ʲô������ nType ������ָ��
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() ���صĴ��ھ��</param>
/// <param name="pKrcData">krc�ļ�����ָ��</param>
/// <param name="nKrcDataLen">krc�ļ����ݳߴ�</param>
/// <param name="nType">��������, �� LYRIC_PARSE_TYPE ����</param>
/// <returns>�����Ƿ���سɹ�</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen, LYRIC_PARSE_TYPE nType)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    CCriticalSection cs(pWndInfo->pCritSec);    // ����, ��ֹ���ﲻ�Ǵ����̵߳���, �����̻߳᲻ͣ�ķ��ʸ�ʽṹ

    lyric_destroy(pWndInfo->hLyric);
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    wnd_info.hLyric = lyric_parse(pKrcData, nKrcDataLen, nType);
    pWndInfo->nTimeOffset = lyric_behind_ahead(wnd_info.hLyric, 0);
    pWndInfo->line1.clear();
    pWndInfo->line2.clear();

    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(*pWndInfo, language);

    if (language)
        wnd_info.add_mode(LYRIC_MODE::EXISTTRANS);
    else
        wnd_info.del_mode(LYRIC_MODE::EXISTTRANS);

    if (!wnd_info.hLyric)
        return false;
    auto& d2dInfo = d2d_get_info();

    // �������С����, ��Ȼ����ֶ��˻������ܶ������
    lyric_calc_text(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                          {
                              LYRIC_DESKTOP_INFO* pWndInfo = (LYRIC_DESKTOP_INFO*)pUserData;
                              CD2DRender& hCanvas = *pWndInfo->dx.hCanvas;
                              if (!pWndInfo->dx.hFont)
                                  lyric_wnd_default_object(*pWndInfo);

                              CComPtr<IDWriteTextLayout> pTextLayout;
                              lyric_wnd_create_text_layout(pText, nTextLen, *pWndInfo->dx.hFont, 0, 0, &pTextLayout);
                              float width = _lyric_wnd_load_krc_calc_text(pWndInfo, pTextLayout, pRetHeight);
                              return width;
                          }, pWndInfo);

    return true;
}

/// <summary>
/// ���²���ʱ��, �����ʾ�Ǹ������ʱ������ʾ��
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="nCurrentTimeMS">Ҫ���µ�ʱ��, ��λ�Ǻ���</param>
/// <returns>�����Ƿ���³ɹ�</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    wnd_info.nCurrentTimeMS = nCurrentTimeMS;
    //InvalidateRect(hWindowLyric, 0, 0);   // ʹ�������ʽ�Ῠ, ��֪��ɶ���, ֱ�ӵ����ػ���
    //lyric_wnd_invalidate(wnd_info);
    return true;
}

/// <summary>
/// ��������ø���ı���ɫ, Ŀǰֻ֧����ͨ��ɫ�͸�����ɫ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="isLight">���õ��Ƿ��Ǹ����ĸ��</param>
/// <param name="pClr">��ɫ����, ARGB��ɫֵ</param>
/// <param name="nCount">��ɫ�����һ��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_desktop_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (isLight)
    {
        pWndInfo->clrLight.assign(&pClr[0], &pClr[nCount]);
    }
    else
    {
        pWndInfo->clrNormal.assign(&pClr[0], &pClr[nCount]);
    }

    return pWndInfo->dx.re_create_brush(pWndInfo, isLight);
}

/// <summary>
/// ���ø���ı�����, ��������ͨ��ʹ��ͬһ������, ����ʹ�ò�ͬ��ɫ����
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="pszName">��������</param>
/// <param name="nSize">����ߴ�</param>
/// <param name="isBold">�Ƿ�Ӵ�</param>
/// <param name="isItalic">�Ƿ�б��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_desktop_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (!pszName || !*pszName)
        pszName = L"΢���ź�";
    if (nSize == 0)
        nSize = 24;

    LOGFONTW& lf = pWndInfo->lf;
    wcscpy_s(lf.lfFaceName, pszName);
    lf.lfHeight = nSize;
    lf.lfWeight = isBold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = isItalic ? TRUE : FALSE;

    return pWndInfo->dx.re_create_font(pWndInfo);
}

/// <summary>
/// ���ø�ʴ��ڱ���ɫ, ARGB��ɫֵ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="clr">������ɫARGB��ɫֵ</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_desktop_set_clr_back(HWND hWindowLyric, DWORD clr)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->dx.clrBack = clr;
    return true;
}

/// <summary>
/// ���ø�ʴ��ڸ���ı��߿���ɫ, ARGB��ɫֵ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="clr">ARGB��ɫֵ</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_desktop_set_clr_border(HWND hWindowLyric, DWORD clr)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->clrBorder = clr;
    return true;
}

/// <summary>
/// ��ȡ��ʴ��ڵ�������Ϣ, Ӧ����Ҫ��������浽ĳ���ط�, �ȴ�����ʱ�򴫵ݽ�����ԭ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="arg">�ο����ظ�ʴ��ڵ�������Ϣ, ���ص����ݲ����޸�</param>
/// <returns></returns>
bool LYRICCALL lyric_desktop_get_config(HWND hWindowLyric, LYRIC_DESKTOP_ARG* arg)
{
    if (!arg || !hWindowLyric)
        return false;

    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    GetWindowRect(hWindowLyric, &arg->rcWindow);
    pWndInfo->scale.unscale(arg->rcWindow); // ���浽���������Ҫ����δ���ŵ�����

    arg->clrWndBack = pWndInfo->dx.clrBack;
    arg->nFontSize = pWndInfo->lf.lfHeight;
    arg->pszFontName = pWndInfo->lf.lfFaceName;
    arg->clrBorder = pWndInfo->clrBorder;
    arg->nClrNormal = (int)pWndInfo->clrLight.size();
    arg->pClrNormal = arg->nClrNormal > 0 ? &pWndInfo->clrLight[0] : nullptr;
    arg->nClrLight = (int)pWndInfo->clrNormal.size();
    arg->pClrLight = arg->nClrLight > 0 ? &pWndInfo->clrNormal[0] : nullptr;

    return true;
}

/// <summary>
/// ���ø�ʴ��ڵ��¼�
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">�������¼�ID</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_desktop_call_event(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_desktop::lyric_wnd_call_evt(*pWndInfo, id);
}

/// <summary>
/// ���ð�ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <param name="state">Ҫ���õ�״̬</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_desktop_set_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id, LYRIC_DESKTOP_BUTTON_STATE state)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_desktop::lyric_wnd_set_btn_state(*pWndInfo, id, state);
}

/// <summary>
/// ��ȡ��ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <returns>���ذ�ť״̬</returns>
LYRIC_DESKTOP_BUTTON_STATE LYRICCALL lyric_desktop_get_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return LYRIC_DESKTOP_BUTTON_STATE_ERROR;
    return lyric_desktop::lyric_wnd_get_btn_state(*pWndInfo, id);
}

