#pragma once
#include <windows.h>



/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <returns>返回窗口句柄</returns>
HWND lyric_wnd_create();

/// <summary>
/// 歌词窗口加载krc歌词, krc是酷狗歌词格式, 加载后可以调用更新函数显示
/// 目前暂时只支持krc, 如果能解密网易云/qq引用的专用歌词格式, 后续再增加
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pKrcData">krc文件数据指针</param>
/// <param name="nKrcDataLen">krc文件数据尺寸</param>
/// <returns>返回是否加载成功</returns>
bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen);

/// <summary>
/// 更新歌词显示, 这个速度还可以, 百万次调用也就几百毫秒, 200帧都能稳定显示
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的世界, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS);


