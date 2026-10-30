<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# 资源目录（Assets）

本目录集中存放可复用的资源（字库、图片、音乐、电池参数等），按资源类型分子目录管理。每个资源放在其类型对应的子目录，并记录放置路径、命名方式、集成方式与来源/许可。二进制资源（字体、图片、音频）不属于纯 markdown 文档，请勿与文档混放。涉及版权/授权的资源需注明来源与许可。

## 字库（fonts）

可复用的字库文件与生成的字库源码放在 `fonts/`。

- 命名要能反映字族、字重、字级与格式。
- 记录来源、许可、字符范围、转换命令与目标放置路径。
- 添加字库前评估 Flash 与内部 RAM 影响；ESP32-C3 无 PSRAM。
- 不提交许可不允许分发的字库。

### Countdown Bomb 中文字库

- `fonts/bomb-ui-subset.ttf`：Google Fonts 的 Noto Sans SC Regular 子集，保留源字体供再生成；来源与 SHA-256 见 `fonts/bomb-font-source.json`。
- `fonts/bomb_font_18.c`：18 px、4 bpp、未压缩 LVGL 9 字库，包含 `main/bomb_text.h` 的全部中文与可打印 ASCII，共 149 字符，位图 14,084 字节。
- 字体及派生字形数据遵循 [SIL OFL 1.1](fonts/OFL.txt)，保留 Adobe 版权。应用字体名为 Bomb UI。
- `fonts/bomb-font-inventory.json` 记录字符、度量和每个字形的像素哈希。
- 普通固件构建不依赖 Pillow。需要重新生成时，在有 Pillow 12.3.0 的隔离 Python 环境运行：

```sh
python tools/generate_bomb_font.py
python tools/check_bomb_font.py
```

`--fetch` 显式从 Google Fonts 获取按文案裁剪的 TTF 与 OFL；默认只使用保留的源字体。
生成后重新解析 C 字库，检查所有字形像素、实际 cmap 和缺字负例。
设备上全部状态的中文显示仍需实测。界面预览来自真实 LVGL 软件渲染，不是实拍。

## 图片（images）

可复用的源图与生成的显示资产放在 `images/`。

| 文件 | 尺寸与格式 | 用途与来源 |
| --- | --- | --- |
| [`images/countdown-bomb-preview.png`](images/countdown-bomb-preview.png) | 768 × 720，PNG | 当前应用真实 LVGL 界面的软件渲染：设置、最大时长、倒计时、三线、成功和爆炸，不是实机照片。 |
| [`images/home.jpg`](images/home.jpg) | 3840 × 2160，JPEG | 嵌入中英文项目 README 的产品主图，突出 AI Passport 产品形象与开放、人人可创作的理念。 |
| [`images/readme-hardware-specs.png`](images/readme-hardware-specs.png) | 2172 × 724，PNG RGBA | 保留为可选技术参考图，不再用于首页主视觉。于 2026-09-17 使用内置图像生成工具为本仓库生成；已根据文档中的硬件能力契约核对图中的六项标签与参数。 |
| [`images/logo-wordmark.png`](images/logo-wordmark.png) | 1648 × 336，PNG RGBA | 从仓库原始 `images/logo.png` 中精确裁切并去除背景的黑色字标；用于中英文项目 README 的浅色主题。 |
| [`images/logo-wordmark-dark.png`](images/logo-wordmark-dark.png) | 1648 × 336，PNG RGBA | 提取字标的白色版本；README 使用 `<picture>` 在 GitHub 深色主题下显示。 |

- 使用描述性命名，并记录尺寸、像素格式、转换步骤与目标路径。
- 优先采用适合 240 × 320 RGB565 显示的格式，并纳入 Flash 与内部 RAM 考量。
- 许可允许时保留可编辑源文件，并记录来源与许可。
- 图片中不得包含设备二维码秘密、凭证或个人数据。

## 音乐与音效（music）

可复用的音乐与音效源码放在 `music/`。

- 记录来源、许可、采样率、位深、声道、转换命令与目标路径。
- 与当前 BSP 音频路径匹配时优先采用 16 kHz、16 位单声道 PCM。
- 嵌入音频前评估 Flash 与内部 RAM 成本；长录音应流式或分块。
- 无再分发许可不提交媒体文件。

### Countdown Bomb 爆炸音效

- [`music/xm4167.mp3`](music/xm4167.mp3) 是用户提供的爆炸录音，原名 `xm4167.mp3`，
  1.896 秒、MP3、48 kHz 双声道。SHA-256 为
  `2a0d66f873746fbc1db034a48e950493c47fb9608efdb4dde6f2ab7978c5526d`。
  未附单独许可，不属于项目代码 MIT 许可覆盖范围。
- [`music/bomb-explosion.pcm`](music/bomb-explosion.pcm) 是固件嵌入的有符号小端
  16 kHz／16 位单声道版本，61,440 字节、1.920 秒。保留全部源录音，不裁剪、不变速；
  峰值按 0.878188 衰减至最高 24,000，不增益放大。已有结尾静音和对齐补零
  提供超过 128 ms 的静音，用于排空 DMA。
  PCM SHA-256 为 `7f174cd62f72279e94dac4e7b134c593a30b38ee75ae1e8d3ca5ab1fe5d0081d`。
- [`music/bomb-explosion.wav`](music/bomb-explosion.wav) 含完全相同的 PCM，供电脑预览；
  固件仅将原始 PCM 嵌入只读 Flash。
- [`music/bomb-explosion-source.json`](music/bomb-explosion-source.json) 记录格式、哈希、
  衰减、采样数量和所用 FFmpeg 8.0.1 版本。使用本地 FFmpeg，在根目录执行
  `python3 tools/prepare_bomb_explosion.py` 可离线重建；普通固件构建不需要 FFmpeg 或 MP3 解码器。
- `main/bomb_sound.c` 通过独立任务以 1 KiB 缓冲每次写入 512 采样，选错线或超时爆炸
  各播放一次，输出音量为 75%。爆炸画面显示 2.4 秒。实际起音、响度、尾音和
  连续多局动画／音效并行表现仍需在卡片试听。

## 电池参数（battery）

[`battery/wanli-500mah-profile2-xil40x-20261020.txt`](battery/wanli-500mah-profile2-xil40x-20261020.txt)
保存万锂新能源 500 mAh 电芯的 CW2017 参数（profile2、XIL40X、2026-10-20）。
该文件是项目提供的 `喜梨_智能工卡_万锂新能源500mAh_profile2_XIL40X_20261020.txt`
的原样副本，按维护者要求保留用于本次固件更新；原文件没有附带单独的许可说明。

格式为 80 行两位十六进制数字，每行一个字节，按寄存器顺序排列。
BSP 在 `components/bsp/src/bsp_battery.c` 中嵌入这 80 字节，主机检查会验证
源文件及数组内容完全一致。文本文件 SHA-256：
`7ea96eba4634f4495a37c25ffacd54c1deaa0693b162c892d671a2ba68460919`。
解码后 80 字节 SHA-256：
`fa7365671093ef449c5bdcfb4adb279e3d275f888109f50b6b2f21e9b96220d5`。
这是电量计参数，不是充电配置，也不能通用于所有 500 mAh 电芯；详见
[硬件指南](../docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.zh_CN.md#9-cw2017-电池计)。
