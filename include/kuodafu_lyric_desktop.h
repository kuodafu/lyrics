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
#include "kuodafu_lyric.h"


// ��ʴ��ڰ�ťID, Ŀǰ���⼸��Ĭ�ϰ�ť
enum LYRIC_DESKTOP_BUTTON_ID
{
    LYRIC_DESKTOP_BUTTON_ID_FIRST           = 1001, // ��һ����ť������

    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2      = 1001, // ���밴ť
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2_SEL  = 1002, // ���밴ť, ѡ��ģʽ
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1      = 1003, // ���밴ť
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1_SEL  = 1004, // ���밴ť, ѡ��ģʽ
    LYRIC_DESKTOP_BUTTON_ID_LRCWRONG        = 1005, // ��ʲ���
    LYRIC_DESKTOP_BUTTON_ID_VERTICAL        = 1006, // ������ť
    LYRIC_DESKTOP_BUTTON_ID_MAKELRC         = 1007, // �������

    LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN       = 1008, // �����С
    LYRIC_DESKTOP_BUTTON_ID_FONT_UP         = 1009, // ��������
    LYRIC_DESKTOP_BUTTON_ID_BEHIND          = 1010, // ����Ӻ�
    LYRIC_DESKTOP_BUTTON_ID_AHEAD           = 1011, // �����ǰ
    LYRIC_DESKTOP_BUTTON_ID_LOCK            = 1012, // ������ť
    LYRIC_DESKTOP_BUTTON_ID_SETTING         = 1013, // ���ð�ť
    LYRIC_DESKTOP_BUTTON_ID_UNLOCK          = 1014, // ������ť
    LYRIC_DESKTOP_BUTTON_ID_CLOSE           = 1015, // �رհ�ť
    LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR        = 1016, // ����������ɫ, ���ֵİ�ťͼ��
    LYRIC_DESKTOP_BUTTON_ID_MENU            = 1017, // �˵���ť

    LYRIC_DESKTOP_BUTTON_ID_LRCWRONG_V      = 1018, // ��ʲ���, ����İ�ťͼ��
    LYRIC_DESKTOP_BUTTON_ID_HORIZONTAL      = 1019, // ������ť
    LYRIC_DESKTOP_BUTTON_ID_PLAY            = 1020, // ����, �ص���������0�������ͣ��ť
    LYRIC_DESKTOP_BUTTON_ID_PAUSE           = 1021, // ��ͣ, �ص���������0����ɲ��Ű�ť
    LYRIC_DESKTOP_BUTTON_ID_PREV            = 1022, // ��һ��
    LYRIC_DESKTOP_BUTTON_ID_NEXT            = 1023, // ��һ��


    LYRIC_DESKTOP_BUTTON_ID_COUNT = LYRIC_DESKTOP_BUTTON_ID_NEXT - LYRIC_DESKTOP_BUTTON_ID_FIRST + 1,  // ��ť����

    LYRIC_DESKTOP_BUTTON_ID_MAKELRCV        = 1024, // �������, ����ť, ���ûŪ��..... ĿǰҲ�ò���
    LYRIC_DESKTOP_BUTTON_ID_SHOW            = 1025, // ��ʾ���, û�����ť, ����������¼�, �������ⲿ����, �ڲ�û�����ť, �ڲ�������������



};

enum LYRIC_DESKTOP_BUTTON_STATE
{
    LYRIC_DESKTOP_BUTTON_STATE_NORMAL   = 0,    // ����״̬
    LYRIC_DESKTOP_BUTTON_STATE_HOVER    = 1,    // �����ͣ
    LYRIC_DESKTOP_BUTTON_STATE_PUSHED   = 2,    // ��갴��
    LYRIC_DESKTOP_BUTTON_STATE_DISABLE  = 4,    // ����״̬

    LYRIC_DESKTOP_BUTTON_STATE_ERROR    = -1,   // ��ȡʧ��
};


// ��ʴ��ڰ�ť������¼�, ����0��ʾ�¼�����, ���ط�0��ʾ�����¼�
typedef int (CALLBACK* PFN_LYRIC_DESKTOP_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

/// <summary>
/// ��ʼ��������, ���ʼ��D2D, ����DPI����, ע�ᴰ�����
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_init();

/// <summary>
/// ȡ����ʼ��, ж��D2D, �ͷŸ�����Դ
/// </summary>
/// <returns>�����Ƿ��ʼ���ɹ�</returns>
bool LYRICCALL lyric_desktop_uninit();

/// <summary>
/// �ͷŸ�ʴ��ڷ��ص�ָ��
/// </summary>
/// <returns></returns>
void LYRICCALL lyric_desktop_free(void* ptr);

/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <param name="arg">������ʴ��ڵĲ���, Ϊ����ʹ��Ĭ��ֵ, ����, ����λ��, ��ɫ����Ϣ</param>
/// <param name="pfnCommand">��ť������ص�����</param>
/// <param name="lParam">���ݵ� pfnCommand() ������Ĳ���</param>
/// <returns>���ش��ھ��</returns>
HWND LYRICCALL lyric_desktop_create(const char* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// ��ʴ��ڼ��ظ������, �����ʲô������ nType ������ָ��
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() ���صĴ��ھ��</param>
/// <param name="pKrcData">krc�ļ�����ָ��</param>
/// <param name="nKrcDataLen">krc�ļ����ݳߴ�</param>
/// <param name="nType">��������, �� LYRIC_PARSE_TYPE ����</param>
/// <returns>�����Ƿ���سɹ�</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen, LYRIC_PARSE_TYPE nType);

/// <summary>
/// ���²���ʱ��, �����ʾ�Ǹ������ʱ������ʾ��
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="nCurrentTimeMS">Ҫ���µ�ʱ��, ��λ�Ǻ���</param>
/// <returns>�����Ƿ���³ɹ�</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS);


/// <summary>
/// ��ȡ��ʴ��ڵ�������Ϣ, ��ʹ��ʱ��Ҫ���� lyric_desktop_free() �ͷ�
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="arg">�ο����ظ�ʴ��ڵ�������Ϣ, ���ص����ݲ����޸�</param>
/// <returns>��������json��Ϣ</returns>
char* LYRICCALL lyric_desktop_get_config(HWND hWindowLyric);

/// <summary>
/// ���ø������, ���ú�����´������ö�Ӧ�Ķ���
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="argJson">���õ�json�ַ���</param>
/// <returns>����Ӱ���˶��ٸ�����, ʧ�ܷ���0</returns>
int LYRICCALL lyric_desktop_set_config(HWND hWindowLyric, const char* argJson);

/// <summary>
/// ���ø�ʴ��ڵ��¼�
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">�������¼�ID</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_desktop_call_event(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id);

/// <summary>
/// ���ð�ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <param name="state">Ҫ���õ�״̬</param>
/// <returns>�����Ƿ���óɹ�</returns>
bool LYRICCALL lyric_desktop_set_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id, LYRIC_DESKTOP_BUTTON_STATE state);

/// <summary>
/// ��ȡ��ť״̬
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="id">��ťID</param>
/// <returns>���ذ�ť״̬</returns>
LYRIC_DESKTOP_BUTTON_STATE LYRICCALL lyric_desktop_get_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id);


