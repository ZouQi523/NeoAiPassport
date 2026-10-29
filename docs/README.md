# 公共资料与开发记录

本目录存放多个应用共享的硬件资料、环境说明和工程决策。应用特有的构建及使用说明放在各应用目录。

## 已确认的组织方式

- 工程及 GitHub 仓库名称为 `NeoAiPassport`。
- 整个工作区对应一个公开 GitHub 仓库，应用不拆成独立仓库。
- 各应用放在 `projects/`，设备端和必要的电脑端按应用归档。
- 本机工具链放在 `.toolchains/`，构建缓存和本机配置不上传。
- 首个应用方向为麦克风；具体交互方式和传输方案尚未确定。

## 环境状态

本次初始化只建立工程结构，不安装开发环境、不下载官方完整固件、不操作设备。

后续选择固件基础后，应记录所选版本、工具链要求、依赖锁定方式和实际验证过的安装步骤。不要把前期调研中的版本要求当成所有后续应用的固定要求。

## 官方参考入口

- [用 AI Agent 创造专属玩法](https://ai-passport.folotoy.cn/guides/create-a-play-with-agent/)
- [官方开发仓库（Gitee）](https://gitee.com/FoloToy/ai-passport)
- [官方开发仓库（GitHub）](https://github.com/FoloToy/ai-passport)
- [官方项目文档](https://github.com/FoloToy/ai-passport/blob/main/docs/README.md)
- [官方环境引导](https://github.com/FoloToy/ai-passport/blob/main/docs/development/engineering/environment-setup.zh_CN.md)

外部入口可能随上游更新。正式实现时应固定源码版本，并查阅该版本对应的说明。
