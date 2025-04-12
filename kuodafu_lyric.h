#pragma once

#define LYRICCALL __stdcall

/// <summary>
/// �������, ����krc���ܺ������, ���ص�ָ����Ҫ���� lyric_free �ͷ�
/// </summary>
/// <param name="pData">����, ��Ҫ���ܵĸ������</param>
/// <param name="nSize">����, ������ݵĳ���</param>
/// <returns>���ؽ��ܺ������, ��ʹ��ʱ��Ҫ���� lyric_free �ͷ�</returns>
void* LYRICCALL lyric_parse(const void* pData, int nSize);

/// <summary>
/// �ͷŲ�����ص��ڴ��ַ
/// </summary>
/// <param name="pData">������ص��ڴ��ַ</param>
/// <returns>�޷���ֵ</returns>
void LYRICCALL lyric_free(void* pData);