# NeoAiPassport

围绕 AI Passport 可编程硬件构建的开源应用工作区。

同一个仓库承载多个独立应用，以及共用的资料、工具和素材。设备端程序与需要配套的电脑端程序按应用组织，通过 GitHub 同步源码和开发记录。

## 当前状态

项目处于初始化阶段：已建立目录和协作约定，尚未引入设备固件源码、安装工具链或提供可运行应用。

首个应用方向是利用设备麦克风解决电脑语音输入需求。音频传输方式、电脑接入方式和是否进行语音识别仍待确认；通过 USB 连接设备本身不代表电脑已经能把它当作音频输入设备。

## 目录

```text
NeoAiPassport/
├── README.md
├── LICENSE
├── AGENTS.md
├── docs/                   公共资料、环境说明和开发记录
├── projects/
│   └── microphone/         麦克风应用
│       ├── README.md
│       └── firmware/       设备端程序的预留位置
├── shared/                 经实际应用验证的共用工具和素材
└── .toolchains/            本机开发环境，不纳入 Git
```

电脑端代码有明确需求时，再建立 `projects/<应用名>/desktop/`。其他目录同样按需扩展，避免提前搭建没有用途的框架。

## 应用索引

| 应用 | 目标 | 状态 |
| --- | --- | --- |
| [Microphone](projects/microphone/README.md) | 使用设备麦克风帮助电脑进行语音输入 | 需求与可行性待确认 |

## 开发与同步

首次获取：

```sh
git clone https://github.com/ZouQi523/NeoAiPassport.git
cd NeoAiPassport
```

日常开发从同步开始：

```sh
git status --short --branch
git pull --ff-only
git switch -c feature/your-change
```

如果工作区有修改，先保存并处理这些修改，再拉取远程内容。完成改动后，检查差异，提交到对应分支并推送；较大的改动通过 Pull Request 合并到 `main`。不要用强制推送覆盖其他电脑上的工作。

每个应用负责自己的依赖说明、构建入口、测试和使用方法。当前没有可执行的构建命令；实际源码接入后再补充可复现的安装与构建步骤。

参考资料与环境约定见 [docs](docs/README.md)。

## 开源许可

本项目原创内容采用 [MIT License](LICENSE)。将来引入第三方源码或素材时，保留其原有许可证、版权和来源说明；本仓库许可证不替代第三方许可。

本项目由社区独立维护，目标硬件由 FoloToy 提供。
