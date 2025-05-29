# lyric / lyric_wnd / lyric_show

**lyric**、**lyric_wnd**、**lyric_show** 是一组由 **扩大福** 开发的开源 C++ 项目，使用 [Apache License 2.0](./LICENSE) 协议授权。

## 项目简介

这三个项目旨在为播放器添加歌词支持, 功能分别如下:

> **lyric**: 用于解析、获取和设置歌词信息的核心库.
> **lyric_wnd**: 基于 `lyric` 实现的桌面歌词显示器, 可将歌词渲染在桌面上.
> **lyric_show**: 基于 `lyric`，用于生成歌词秀页面, 可获取已排版好的可视化歌词内容.



## 特性

- 基于 C++ 开发，接口简洁易用
- 支持 Windows 平台
- 轻量、无外部依赖
- Apache 2.0 协议，支持商业使用

## 使用方式

> 示例:

```cpp
#include "lyric.h"

int main()
{
    lyric::DoSomething();
    return 0;
}
