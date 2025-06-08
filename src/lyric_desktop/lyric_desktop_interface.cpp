/*
* ����ļ��������ʵ����Ľӿ�, ���нӿڶ�д������
* ���ǽӿں�������������д
*/

#include "lyric_desktop_function.h"
#include <CommCtrl.h>
#include <atlbase.h>

using namespace NAMESPACE_LYRIC_DESKTOP;
using namespace KUODAFU_NAMESPACE;

/// <summary>
/// ��ʼ��������, ���ʼ��D2D, ����DPI����, ע�ᴰ�����
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_init()
{
    return _lyric_dwsktop_init();
}

/// <summary>
/// ȡ����ʼ��, ж��D2D, �ͷŸ�����Դ
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_uninit()
{
    return _lyric_dwsktop_uninit();
}

void LYRICCALL lyric_desktop_free(void* ptr)
{
    ::free(ptr);
}


/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <param name="arg">������ʴ��ڵĲ���, ����, ����λ��, ��ɫ����Ϣ</param>
/// <param name="pfnCommand">��ť������ص�����</param>
/// <param name="lParam">���ݵ� pfnCommand() ������Ĳ���</param>
/// <returns>���ش��ھ��</returns>
HWND LYRICCALL lyric_desktop_create(const char* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _ld_create_layered_window(arg, pfnCommand, lParam);
    if (!pWndInfo)
        return nullptr;

    return pWndInfo->hWnd;
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
    pWndInfo->hLyric = nullptr;

    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;

    wnd_info.hLyric = lyric_parse(pKrcData, nKrcDataLen, nType);
    pWndInfo->nTimeOffset = lyric_behind_ahead(wnd_info.hLyric, 0);
    pWndInfo->line1.clear();
    pWndInfo->line2.clear();

    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(*pWndInfo, language);

    if (language)
        wnd_info.add_mode(LYRIC_DESKTOP_MODE::EXISTTRANS);
    else
        wnd_info.del_mode(LYRIC_DESKTOP_MODE::EXISTTRANS);

    if (!wnd_info.hLyric)
        return false;

    // �������С����, ��Ȼ����ֶ��˻������ܶ������
    lyric_calc_text(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                          {
                              LYRIC_DESKTOP_INFO* pWndInfo = (LYRIC_DESKTOP_INFO*)pUserData;
                              D2DRender& pRender = *pWndInfo->dx.pRender;
                              // û�д�������ʹ���һ��, �������豸�޹ض���, ����Ҫ����Ⱦ�����ﴴ��
                              if (!pWndInfo->dx.hFont)
                                  pWndInfo->dx.re_create_font(pWndInfo);

                              CComPtr<IDWriteTextLayout> pTextLayout;
                              lyric_wnd_create_text_layout(pText, nTextLen, pWndInfo->dx.hFont->GetDWTextFormat(), 0, 0, &pTextLayout);
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
/// ��ȡ��ʴ��ڵ�������Ϣ, ��ʹ��ʱ��Ҫ���� lyric_desktop_free() �ͷ�
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="arg">�ο����ظ�ʴ��ڵ�������Ϣ, ���ص����ݲ����޸�</param>
/// <returns>��������json��Ϣ</returns>
char* LYRICCALL lyric_desktop_get_config(HWND hWindowLyric)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return nullptr;
    return pWndInfo->config.to_json(pWndInfo);
}

/// <summary>
/// ���ø������, ���ú�����´������ö�Ӧ�Ķ���
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="argJson">���õ�json�ַ���</param>
/// <returns>����Ӱ���˶��ٸ�����, ʧ�ܷ���0</returns>
int LYRICCALL lyric_desktop_set_config(HWND hWindowLyric, const char* argJson)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return 0;
    return pWndInfo->config.parse(argJson, pWndInfo);
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

