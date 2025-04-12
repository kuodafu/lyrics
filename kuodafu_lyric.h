#pragma once

#define LYRICCALL __stdcall

/// <summary>
/// 解析歌词, 返回krc解密后的数据, 返回的指针需要调用 lyric_free 释放
/// </summary>
/// <param name="pData">输入, 需要解密的歌词数据</param>
/// <param name="nSize">输入, 歌词数据的长度</param>
/// <returns>返回解密后的数据, 不使用时需要调用 lyric_free 释放</returns>
void* LYRICCALL lyric_parse(const void* pData, int nSize);

/// <summary>
/// 释放插件返回的内存地址
/// </summary>
/// <param name="pData">插件返回的内存地址</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_free(void* pData);