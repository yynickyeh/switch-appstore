# 字体文件说明

本应用需要中文字体文件才能正常显示文字。

## 方法一：添加字体到 romfs

1. 下载 [Noto Sans CJK SC](https://github.com/googlefonts/noto-cjk/releases) 字体
2. 将 `NotoSansCJK-Regular.ttc` 复制到 `romfs/fonts/` 目录
3. 重新编译应用 (`make clean && make`)

## 方法二：复制字体到 SD 卡

将字体文件复制到 Switch SD 卡的以下任一位置：
- `sdmc:/switch/fonts/NotoSansCJKsc-Regular.otf`
- `sdmc:/switch/appstore/fonts/NotoSansCJKsc-Regular.otf`

## 推荐字体

- [Noto Sans CJK SC](https://github.com/googlefonts/noto-cjk) - Google 开源中文字体
- [思源黑体](https://github.com/adobe-fonts/source-han-sans) - Adobe 开源字体
