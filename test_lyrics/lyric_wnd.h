#pragma once
#include <windows.h>



/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <returns>���ش��ھ��</returns>
HWND lyric_wnd_create();

/// <summary>
/// ��ʴ��ڼ���krc���, krc�ǿṷ��ʸ�ʽ, ���غ���Ե��ø��º�����ʾ
/// Ŀǰ��ʱֻ֧��krc, ����ܽ���������/qq���õ�ר�ø�ʸ�ʽ, ����������
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() ���صĴ��ھ��</param>
/// <param name="pKrcData">krc�ļ�����ָ��</param>
/// <param name="nKrcDataLen">krc�ļ����ݳߴ�</param>
/// <returns>�����Ƿ���سɹ�</returns>
bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen);

/// <summary>
/// ���¸����ʾ, ����ٶȻ�����, ����ε���Ҳ�ͼ��ٺ���, 200֡�����ȶ���ʾ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="nCurrentTimeMS">Ҫ���µ�����, ��λ�Ǻ���</param>
/// <returns>�����Ƿ���³ɹ�</returns>
bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS);


