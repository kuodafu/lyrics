/*
 * Copyright (c) 2025 ���� (121007124@qq.com)
 *
 * Author      : ����
 * Website     : https://www.kuodafu.com
 * Project     : https://github.com/kuodafu/lyrics
 * QQ Group    : 121007124, 20752843
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


 /*
 * ���ļ��������ʵ����ĺ����ӿ�.
 */

#pragma once
#include <windows.h>

#define LYRICCALL __stdcall


// ��ʴ��ڰ�ťID, Ŀǰ���⼸��Ĭ�ϰ�ť
enum LYRIC_WND_BUTTON_ID
{
    LYRIC_WND_BUTTON_ID_FIRST           = 1001, // ��һ����ť������

    LYRIC_WND_BUTTON_ID_TRANSLATE2      = 1001, // ���밴ť
    LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL  = 1002, // ���밴ť, ѡ��ģʽ
    LYRIC_WND_BUTTON_ID_TRANSLATE1      = 1003, // ���밴ť
    LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL  = 1004, // ���밴ť, ѡ��ģʽ
    LYRIC_WND_BUTTON_ID_LRCWRONG        = 1005, // ��ʲ���
    LYRIC_WND_BUTTON_ID_VERTICAL        = 1006, // ������ť
    LYRIC_WND_BUTTON_ID_MAKELRC         = 1007, // �������

    LYRIC_WND_BUTTON_ID_FONT_DOWN       = 1008, // �����С
    LYRIC_WND_BUTTON_ID_FONT_UP         = 1009, // ��������
    LYRIC_WND_BUTTON_ID_BEHIND          = 1010, // ����Ӻ�
    LYRIC_WND_BUTTON_ID_AHEAD           = 1011, // �����ǰ
    LYRIC_WND_BUTTON_ID_LOCK            = 1012, // ������ť
    LYRIC_WND_BUTTON_ID_SETTING         = 1013, // ���ð�ť
    LYRIC_WND_BUTTON_ID_UNLOCK          = 1014, // ������ť
    LYRIC_WND_BUTTON_ID_CLOSE           = 1015, // �رհ�ť
    LYRIC_WND_BUTTON_ID_LRCCOLOR        = 1016, // ����������ɫ, ���ֵİ�ťͼ��
    LYRIC_WND_BUTTON_ID_MENU            = 1017, // �˵���ť

    LYRIC_WND_BUTTON_ID_LRCWRONG_V      = 1018, // ��ʲ���, ����İ�ťͼ��
    LYRIC_WND_BUTTON_ID_HORIZONTAL      = 1019, // ������ť
    LYRIC_WND_BUTTON_ID_PLAY            = 1020, // ����, �ص���������0�������ͣ��ť
    LYRIC_WND_BUTTON_ID_PAUSE           = 1021, // ��ͣ, �ص���������0����ɲ��Ű�ť
    LYRIC_WND_BUTTON_ID_PREV            = 1022, // ��һ��
    LYRIC_WND_BUTTON_ID_NEXT            = 1023, // ��һ��


    LYRIC_WND_BUTTON_ID_COUNT = LYRIC_WND_BUTTON_ID_NEXT - LYRIC_WND_BUTTON_ID_FIRST + 1,  // ��ť����

    LYRIC_WND_BUTTON_ID_MAKELRCV        = 1024, // �������, ����ť, ���ûŪ��..... ĿǰҲ�ò���
    LYRIC_WND_BUTTON_ID_SHOW            = 1025, // ��ʾ���, û�����ť, ����������¼�, �������ⲿ����, �ڲ�û�����ť, �ڲ�������������



};

enum LYRIC_WND_BUTTON_STATE
{
    LYRIC_WND_BUTTON_STATE_NORMAL   = 0,    // ����״̬
    LYRIC_WND_BUTTON_STATE_HOVER    = 1,    // �����ͣ
    LYRIC_WND_BUTTON_STATE_PUSHED   = 2,    // ��갴��
    LYRIC_WND_BUTTON_STATE_DISABLE = 4,    // ����״̬

    LYRIC_WND_BUTTON_STATE_ERROR    = -1,   // ��ȡʧ��
};

typedef struct LYRIC_WND_ARG
{
    RECT        rcWindow;       // Ҫ���õĴ���λ�úʹ�С
    DWORD       clrWndBack;     // ������ɫ
    DWORD       clrWndBorder;   // ���ڱ����߿���ɫ, Ϊ0����ʾ�߿�
    int         nFontSize;      // �����С
    LPCWSTR     pszFontName;    // ��������
    DWORD       clrBorder;      // �߿���ɫ

    DWORD*      pClrNormal;     // ��ͨ�����ɫ����
    int         nClrNormal;     // ��ͨ�����ɫ���鳤��

    DWORD*      pClrLight;      // ���������ɫ����
    int         nClrLight;      // ���������ɫ���鳤��

}*PLYRIC_WND_ARG;

// ��ʴ��ڰ�ť������¼�, ����0��ʾ�¼�����, ���ط�0��ʾ�����¼�
typedef int (CALLBACK* PFN_LYRIC_WND_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

/// <summary>
/// ��ʼ��������, ���ʼ��D2D, ����DPI����, ע�ᴰ�����
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_wnd_init();

/// <summary>
/// ȡ����ʼ��, ж��D2D, �ͷŸ�����Դ
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_wnd_uninit();

/// <summary>
/// ��ȡ�������ڵ�Ĭ�ϲ���
/// </summary>
/// <param name="arg">����Ĭ�ϲ����Ľṹ</param>
void LYRICCALL lyric_wnd_get_default_arg(LYRIC_WND_ARG* arg);

/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <param name="arg">������ʴ��ڵĲ���, ����, ����λ��, ��ɫ����Ϣ</param>
/// <param name="pfnCommand">��ť������ص�����</param>
/// <param name="lParam">���ݵ� pfnCommand() ������Ĳ���</param>
/// <returns>���ش��ھ��</returns>
HWND LYRICCALL lyric_wnd_create(const LYRIC_WND_ARG* arg, PFN_LYRIC_WND_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// ��ʴ��ڼ���krc���, krc�ǿṷ��ʸ�ʽ, ���غ���Ե��ø��º�����ʾ
/// Ŀǰ��ʱֻ֧��krc, ����ܽ���������/qq���õ�ר�ø�ʸ�ʽ, ����������
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() ���صĴ��ھ��</param>
/// <param name="pKrcData">krc�ļ�����ָ��</param>
/// <param name="nKrcDataLen">krc�ļ����ݳߴ�</param>
/// <param name="isDecrypted">krc�����Ƿ��Ѿ�������, isDecrypted Ϊtrueʱ, pDataָ��UTF16�����KRC���ܺ������, nSize��ʾpData���ַ���</param>
/// <returns>�����Ƿ���سɹ�</returns>
bool LYRICCALL lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen, bool isDecrypted);

/// <summary>
/// ���²���ʱ��, �����ʾ�Ǹ������ʱ������ʾ��
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="nCurrentTimeMS">Ҫ���µ�ʱ��, ��λ�Ǻ���</param>
/// <returns>�����Ƿ���³ɹ�</returns>
bool LYRICCALL lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS);

/// <summary>
/// ��������ø���ı���ɫ, Ŀǰֻ֧����ͨ��ɫ�͸�����ɫ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="isLight">���õ��Ƿ��Ǹ����ĸ��</param>
/// <param name="pClr">��ɫ����, ARGB��ɫֵ</param>
/// <param name="nCount">��ɫ�����һ��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_wnd_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount);

/// <summary>
/// ���ø���ı�����, ��������ͨ��ʹ��ͬһ������, ����ʹ�ò�ͬ��ɫ����
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="pszName">��������</param>
/// <param name="nSize">����ߴ�</param>
/// <param name="isBold">�Ƿ�Ӵ�</param>
/// <param name="isItalic">�Ƿ�б��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_wnd_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic);

/// <summary>
/// ���ø�ʴ��ڱ���ɫ, ARGB��ɫֵ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="clr">������ɫARGB��ɫֵ</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_wnd_set_clr_back(HWND hWindowLyric, DWORD clr);

/// <summary>
/// ���ø�ʴ��ڸ���ı��߿���ɫ, ARGB��ɫֵ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="clr">ARGB��ɫֵ</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool LYRICCALL lyric_wnd_set_clr_border(HWND hWindowLyric, DWORD clr);

/// <summary>
/// ��ȡ��ʴ��ڵ�������Ϣ, Ӧ����Ҫ��������浽ĳ���ط�, �ȴ�����ʱ�򴫵ݽ�����ԭ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="arg">�ο����ظ�ʴ��ڵ�������Ϣ, ���ص����ݲ����޸�</param>
/// <returns></returns>
bool LYRICCALL lyric_wnd_get_config(HWND hWindowLyric, LYRIC_WND_ARG* arg);

/// <summary>
/// ���ø�ʴ��ڵ��¼�
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">�������¼�ID</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_wnd_call_event(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id);

/// <summary>
/// ���ð�ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <param name="state">Ҫ���õ�״̬</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_wnd_set_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id, LYRIC_WND_BUTTON_STATE state);

/// <summary>
/// ��ȡ��ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <returns>���ذ�ť״̬</returns>
LYRIC_WND_BUTTON_STATE LYRICCALL lyric_wnd_get_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id);


