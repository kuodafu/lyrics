# lyric / lyric_desktop / lyric_show

**lyric**、**lyric_desktop**、**lyric_show** 是一组由 **扩大福** 开发的开源 C++ 项目，使用 [Apache License 2.0](./LICENSE) 协议授权。

## 项目简介

这三个项目旨在为播放器添加歌词支持, 功能分别如下:

> **lyric**: 用于解析、获取和设置歌词信息的核心库.

> **lyric_desktop**: 基于 `lyric` 实现的桌面歌词显示器, 可将歌词渲染在桌面上.

> **lyric_show**: 基于 `lyric`，用于生成歌词秀页面, 将渲染好的内容显示到指定位置上.




## 特性

- 基于 C++ 开发，接口简洁易用
- 支持 Windows 平台
- 轻量、无外部依赖
- Apache 2.0 协议，支持商业使用

## lyric使用方式

> 示例:

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include "lyric.h"

// 读取二进制文件，返回读取的字节数，读取内容写入 output
size_t ReadBinaryFile(const std::wstring& filePath, std::string& output)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);  // 以二进制 + 移到末尾
    if (!file)
        return 0;

    std::streamsize size = file.tellg();  // 获取文件大小
    if (size <= 0)
        return 0;

    output.resize(static_cast<size_t>(size));  // 分配空间
    file.seekg(0, std::ios::beg);              // 回到开头
    if (!file.read(&output[0], size))          // 读取数据
        return 0;

    return static_cast<size_t>(size);
}

int main()
{
    auto pszFile_u16 = LR"(d:\kuodafu\lyric\qrc\周杰伦 - 本草纲目.qrc)";
    auto pszFile_u8 = u8R"(d:\kuodafu\lyric\qrc\周杰伦 - 本草纲目.qrc)";
    auto pszFile_gbk = u8R"(d:\kuodafu\lyric\qrc\周杰伦 - 本草纲目.qrc)";
    //! 这里演示按传递文件
    {
        int nType = LYRIC_PARSE_TYPE_QRC    // 表示传递进去的歌词数据是qq音乐的qrc文件
            | LYRIC_PARSE_TYPE_PATH         // 表示传递进去的参数是路径
            | LYRIC_PARSE_TYPE_UTF16        // 因为传递的是文件路径, 所以得指明文件路径是什么编码
            ;
        HLYRIC hLyric = lyric_parse(pszFile_u16,// 传递文件路径
                                    -1,         // 传递的是路径, 支持填写-1, 不写-1则需要写文本的字节数, 不是字符数, 单位是字节
                                    static_cast<LYRIC_PARSE_TYPE>(nType));

        //TODO 这里自行对解析结果进行判断, 解析失败返回空指针
        lyric_destroy(hLyric);
    }

    //! 这里演示传递文件数据
    {
        std::string krc_data;
        ReadBinaryFile(pszFile_u16, krc_data);

        int nType = LYRIC_PARSE_TYPE_KRC    // 表示传递进去的歌词数据是krc歌词
            | LYRIC_PARSE_TYPE_DATA         // 表示传递进去的参数是歌词数据
            | LYRIC_PARSE_TYPE_ENCRYPT      // 表示传递进去的歌词数据还没解密, 还是加密状态
            ;   // 上面这3个值都是0, 也可以直接填0

        HLYRIC hLyric = lyric_parse(krc_data.c_str(),   // 传递文件数据
                                    krc_data.size(),    // 数据尺寸
                                    static_cast<LYRIC_PARSE_TYPE>(nType));

        //TODO 这里自行对解析结果进行判断, 解析失败返回空指针
        lyric_destroy(hLyric);
    }

    //! 这里演示传递已经解密的歌词数据
    {
        // 这里使用库提供的解密函数来解密, 这里解密krc歌词
        // 调用参数和 lyric_parse() 完全一样, 不同的就是返回值
        // lyric_decrypt() 返回的是解密后的文本, 需要调用 lyric_free() 释放
        // lyric_parse() 返回的是解析的句柄, 需要调用 lyric_destroy() 销毁
        int nType = LYRIC_PARSE_TYPE_KRC    // 表示传递进去的歌词数据是krc歌词
            | LYRIC_PARSE_TYPE_PATH         // 表示传递进去的参数是路径
            | LYRIC_PARSE_TYPE_UTF8         // 表示明文的编码是utf8, 如果是其他编码改这个标志
            ;
        wchar_t* krc_text = lyric_decrypt(pszFile_u8, -1, static_cast<LYRIC_PARSE_TYPE>(nType));

        //TODO 这里自行对解密结果进行判断, 解密失败返回空指针

        // 加载krc歌词, 传递的参数是krc文件数据, 尺寸就是文件的尺寸, 单位是自己
        int nType = LYRIC_PARSE_TYPE_KRC    // 表示传递进去的歌词数据是krc歌词
            | LYRIC_PARSE_TYPE_DECRYPT      // 表示歌词已经解密, 有这个标志就意味着传递进去的是明文, 尺寸可以写-1
            | LYRIC_PARSE_TYPE_UTF16        // 表示明文的编码是utf16, 如果是其他编码改这个标志
            ;
        HLYRIC hLyric = lyric_parse(krc_text,   // 歌词明文数据
                                    -1,         // 歌词明文数据尺寸, 单位是字节, -1表示\0结尾
                                    static_cast<LYRIC_PARSE_TYPE>(nType));

        // 释放解密后的文本
        lyric_free(krc_text);
        lyric_destroy(hLyric);
    }

    //! 这里演示一些其他功能
    {
        int nType = LYRIC_PARSE_TYPE_KRC    // 表示传递进去的歌词数据是krc歌词
            | LYRIC_PARSE_TYPE_PATH         // 表示传递进去的参数是路径
            | LYRIC_PARSE_TYPE_UTF8         // 表示明文的编码是utf8, 如果是其他编码改这个标志
            ;
        HLYRIC hLyric = lyric_parse(pszFile_gbk, -1, LYRIC_PARSE_TYPE_KRC);

        // 挂接一个计算计算歌词文本的回调函数
        // 在调用 lyric_calc() 时如果没有计算歌词文本宽高, 则会调用这里绑定的回调
        lyric_calc_text(hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                        {
                            // 调用 lyric_calc() 时如果没有计算过歌词文本宽高, 则会调用到这里
                            // 这里可以自行发挥计算文本宽高, 返回的宽高是浮点型, 单位是像素
                            // 这里返回的宽高会在 lyric_calc() 里用来计算高亮位置
                            // pText 是要计算的文本, nTextLen 是文本长度, 单位是字符, 不包含结束符
                            return 0.f;
                        }, nullptr);

        // 这里设置歌词提前或者延后, 设置后是会影响到 lyric_calc() 计算的歌词行个高亮位置
        // 正数就是提前, 负数就是延后, 单位是毫秒, 填0就是不设置, 返回当前设置的值
        lyric_behind_ahead(hLyric, 123);

        int time_ms = 123456;   // 要获取歌词的时间, 单位是毫秒
        LYRIC_CALC_STRUCT arg = { 0 };
        // 计算指定时间是在哪一行歌词里, 效率还行, 每秒执行千万次没问题
        // 假设时间大于 第5行歌词的结束时间, 且小于第6行歌词的开始时间, 则返回第5行歌词的信息
        // LYRIC_CALC_STRUCT 结构里包含这个时间点的歌词行/字信息
        lyric_calc(hLyric, time_ms, &arg);

        // 调用后得到了歌词位置和高亮位置, 这里就可以进行绘画操作了
        // ....
        // ....
        // ....
        lyric_destroy(hLyric);  // 不使用时必须销毁, 不然会内存泄漏
    
    }

}


```