<p align="right">
  <strong>简体中文</strong> · <a href="readme.md">English</a>
</p>

# NeoAiPassport 项目记录

[仓库概览](../../README.zh_CN.md) · [官方开发文档](../README.zh_CN.md)

本记录描述项目需求和集成决策，与上游硬件和工程文档分开维护。

## 当前应用：定时炸弹

当前在 `feature/countdown-bomb` 开发离线倒计时拆弹试验，见[应用说明](countdown-bomb.zh_CN.md)。

## Microphone：用户已停止

2026-10-30，用户明确停止该应用。以下需求仅保留为历史上下文，
不再作为当前实施计划。当前任务是理解和验证官方基线及其构建、烧写流程。

### 此前确认的需求

- AI Passport 与 Mac 通过有线连接。
- 使用卡片麦克风收音，由豆包输入法负责转写和文字输入。
  当前没有要求另行接入语音识别服务。
- 一个按键按住期间采集、松开停止，对应豆包的按住说话。
- 另一个按键单击开始持续采集，再单击停止，对应豆包的持续说话。
- 设备屏幕用图标显示麦克风当前采集状态。
- 第三个按键的用途暂未确定。

### 停止时未决事项

- 是否接受 Mac 后台配套程序和虚拟音频设备。
- 豆包实际快捷键、已安装版本，以及对桥接音频和模拟按键的兼容性。
- 实体按键分配、长按阈值，以及按住采集和持续采集之间的交互。
- 断线与异常处理、第三键功能，以及设备已有功能和存储数据的保留要求。

此前已在 Mac 检测到 USB Serial/JTAG 串口。尚未在这台设备验证串口音频传输、
系统麦克风输入、键盘触发或豆包转写。出现 USB 设备不代表支持 USB Audio/HID。

## 目录迁移：2026-10-30

按用户要求取消自定义 `projects/` 组织，采用官方根目录固件结构，
同时取消未使用的 `shared/` 骨架。原工作区文件已保存在被忽略的本机迁移备份中，
已有 Git 历史和 `origin` 保留。

官方基线迁移当时位于 `feature/official-baseline`（应用停止后由 `feature/microphone` 更名）。
该分支从已有 NeoAiPassport 历史创建，
并非从上游 `main` 派生。最初迁移以未提交的源码快照形式准备，
当时未提交、推送、发布或写入设备；后续应用存档与实机测试另行取得授权并记录。尚未建立官方严格 fork 工作流要求的
干净 `main` 继承关系；调整 Git 历史属于单独决策。

### 来源版本和许可

- 用户指定入口：[FoloToy/ai-passport（Gitee）](https://gitee.com/FoloToy/ai-passport)。
- 对照来源：[FoloToy/ai-passport（GitHub）](https://github.com/FoloToy/ai-passport)。
- 2026-10-30，通过 Gitee Git 接口核对，两个入口的 `main` 提交一致。
- 版本：[`006c2279852e352a8433b2c21e5e09be2ca36f77`](https://github.com/FoloToy/ai-passport/tree/006c2279852e352a8433b2c21e5e09be2ca36f77)。
- 源提交日期：2026-10-21 UTC。
- 归档：[固定版本源码包](https://codeload.github.com/FoloToy/ai-passport/tar.gz/006c2279852e352a8433b2c21e5e09be2ca36f77)。
- 归档 SHA-256：`ede47b920e231e48bc8e9c158db4284da104b2b571216a530736cc4cec0ab068`。
- 许可：[MIT](../../LICENSE)，保留 FoloToy 原版权声明。
- 固件环境：ESP32-C3 对应的 ESP-IDF 5.5.3，组件依赖由 `dependencies.lock` 固定。

根 README 配对和本记录属于项目专属文档。已引入文件的本地调整记录如下：
AGENTS 配对中的项目约定、补充的忽略规则，以及 LICENSE 中新增的项目版权声明。
另有两项保持检查内容不变的 macOS 工具兼容调整：
`tools/install-actionlint.sh` 直接比较 SHA-256 输出，避免依赖 GNU 长选项；
`tools/validate.sh` 在 Darwin 使用 Apple 链接器的 `-dead_strip`，
其他系统仍使用 GNU 的 `--gc-sections`。
设备应用和 BSP 保持上游硬件测试基线。

## 参考实现

- [Mac Voice Remote](https://github.com/ihonghong/ai-passport-macos-voice-remote)：
  基于 BLE 的 Mac 配套程序、虚拟麦克风和实体快捷键控制。
- [Input Bridge](https://github.com/MindMobius/ai-passport-input-bridge)：
  USB／BLE 音频和输入法快捷键桥接，主要面向 Windows。
- [Folo AI Passport Voice](https://github.com/zhaohuaxiaoy/folo-ai-passport-voice)：
  USB／BLE 音频传输及独立云端转写服务。

这些仓库目前仅作为调研参考，其应用代码尚未引入、安装或在 NeoAiPassport 中测试。

## 官方基线环境与验证

以下是已完成的官方基线验证记录；当前应用见上方入口。Microphone 仍保持停止。
开发工具保持在忽略的 `.toolchains/` 下，安装和激活不修改全局 Git、shell 或包管理器配置。

- SDK：ESP-IDF v5.5.3，注解标签对象 `b31fcc7a314a44ad992b58f589f7d1d8a4fadff6`，
  实际提交 `2c211b236707889e8400c4dc5644dd5c4ee071e0`，已与 GitHub 官方标签核对。
- SDK 主目录使用乐鑫官方中国大陆 Git 服务；子模块按 SDK 固定的提交初始化。
- 工具和 Python 环境安装在 `.toolchains/espressif/`，使用 Python 3.12。
- 五个官方技能通过项目内 `.agents/skills/` 相对链接启用，链接被 Git 忽略。
- 官方完整门禁 `./tools/validate.sh` 已通过：仓库检查、电脑端测试、固件编译、合并镜像和归档校验均成功。实机验证尚未执行。

### 激活本机隔离环境

从仓库根目录执行；只影响当前终端：

```sh
export IDF_TOOLS_PATH="$PWD/.toolchains/espressif"
export IDF_PYTHON_ENV_PATH="$IDF_TOOLS_PATH/python_env/idf5.5_py3.12_env"
. .toolchains/esp-idf-v5.5.3/export.sh
idf.py --version
./tools/validate.sh
```

此入口使用项目内工具和 Python 环境。不会自动烧写设备，也不修改 shell 启动文件。

### 本次已验证固件

- 类型：未修改设备应用和 BSP 的官方硬件测试基线。
- Build：PASS；Host tests：PASS；Device tests：NOT RUN。
- Unverified：实机启动、屏幕、三键、音频、电池、无线及低功耗行为。
- 固件版本：`ddeae51-dirty`，来自当时尚未提交的基线工作区，不能当作上游发布版本。
- 合并镜像：`build/firmware/47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee/FoloToy-AI-Passport-full.bin`，1,593,152 字节，刷写起点 `0x0`。
- 完整镜像 SHA-256：`47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee`。
- 对应 ELF SHA-256：`66494830d26d971df1a32e66732e8f2eec51269ea8e41d8dc16ac8fa04baf94b`。
- 调试归档：`build/firmware/47a6ce35f63f4e0cad8a9247e19c586394737935074ce0eaad06397146d77fee/`，其 `manifest.json`、ELF、MAP 和镜像已独立复验。
- `dependencies.lock`、`sdkconfig.defaults`、`partitions.csv`、`main/` 和 `components/` 与固定的上游快照一致。
- 构建时只读检测到设备串口，尚未打开串口、烧写、清空或操作设备存储。

这些产物仅保存在被忽略的本机构建目录中，没有提交、推送或发布。
烧写这份基线将替换设备当前应用，合并镜像可能重置 NVS 设置；实机验证须另行确认。
