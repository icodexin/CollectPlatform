# 多模态数据采集与学习者状态实时感知平台客户端
Multi-modal Data Collection & Learner State Real-time Perception Platform (Client)

## 项目简介
本项目是基于 Qt 6 框架开发的跨平台客户端应用，核心目标是实现多模态数据（脑电、手环、相机等）的实时采集、处理、传输与可视化，最终支撑教育场景下学习者生理/行为状态的实时感知与分析。项目兼顾数据采集的稳定性、处理的实时性和界面的交互性，支持 Windows/macOS 多平台部署。

## 核心特性
- 📊 **多模态数据采集**：支持脑电信号、手环生理数据、相机视觉数据等多设备/多类型数据接入；
- ⚡ **实时处理与可视化**：内置高效数据结构（如 `MinMaxQueue`）实现数据极值计算，结合 Qt Graphs 完成实时图表渲染；
- 🌐 **跨平台兼容**：基于 Qt 6 实现 Windows (x64/arm64)、macOS (arm64) 跨平台构建与运行；
- 📡 **数据传输能力**：集成 MQTT 协议，支持采集数据向服务端/其他终端的实时传输；
- 🤖 **自动化构建**：通过 GitHub Actions 实现多架构自动化编译、打包与产物分发；
- 🛠️ **模块化设计**：采集端、可视化端、公共工具模块解耦，便于扩展与维护。

## 技术栈
| 类别         | 核心技术/工具                                                                 |
|--------------|------------------------------------------------------------------------------|
| 核心框架     | Qt 6.8.3（Qt Widgets/Qt Quick/QML）                                          |
| 构建工具     | CMake 3.25+、vcpkg（依赖管理）                                               |
| 编译器       | MSVC 2022（Windows）、Clang（macOS）                                         |
| 关键依赖     | Qt Graphs、Qt Multimedia、Qt WebSockets、Qt MQTT、msgpack（数据序列化）       |
| 第三方组件   | HuskarUI（UI 组件库）                                                        |
| CI/CD        | GitHub Actions                                                               |
| 数据结构     | 线程安全队列（CQueue）、极值计算队列（MinMaxQueue）                          |

## 项目结构
```
├── 3rdparty/                # 第三方依赖库（含补丁）
│   ├── HuskarUI/            # HuskarUI 可视化组件库
│   └── patches/             # 依赖库补丁文件
├── coapp/                   # 采集端应用（Qt Widgets）
│   ├── src/                 # 核心源码
│   │   ├── BandServer/      # 手环数据采集服务
│   │   ├── EEGReceiver/     # 脑电数据接收模块
│   │   ├── CameraService/   # 相机数据采集服务
│   │   └── LogBox/          # 日志组件
│   ├── CMakeLists.txt       # 模块构建配置
│   └── coapp.pro            # Qt 工程文件（兼容配置）
├── muapp/                   # 可视化端应用（Qt Quick/QML）
│   ├── qml/                 # QML 界面文件
│   │   ├── Charts/          # 图表组件（单/多折线图）
│   │   ├── Pages/           # 业务页面（首页/实时数据/离线页）
│   │   └── Models/          # 数据模型（MinMaxQueue 等）
│   ├── src/                 # 逻辑源码
│   └── CMakeLists.txt       # 模块构建配置
├── common/                  # 公共工具模块
│   ├── SettingsManager/     # 全局配置管理
│   ├── CQueue/              # 线程安全队列
│   └── Utils/               # 通用工具函数
├── scripts/                 # 构建脚本
│   ├── build_3rd.ps1        # Windows 编译第三方依赖
│   ├── build_3rd.sh         # macOS/Linux 编译第三方依赖
│   └── deploy.ps1           # Windows 打包部署脚本
├── CMakeLists.txt           # 根项目构建配置
├── CMakePresets.json        # CMake 预设（多架构/模式）
└── README.md                # 项目说明文档
```

## 快速开始

### 1. 环境准备
#### 基础依赖（必装）
- Qt 6.8.3（需包含 `qtgraphs`、`qtmultimedia`、`qtwebsockets`、`qttcpserver` 模块）
- CMake 3.25+
- vcpkg（推荐全局安装，配置 `VCPKG_ROOT` 环境变量）
#### 平台专属依赖
- **Windows**：Visual Studio 2022（带 C++ 开发工具）、PowerShell 7+
- **macOS**：Xcode 15+（Command Line Tools）、Homebrew

### 2. 编译第三方依赖
项目依赖 `HuskarUI`、`Qt6Mqtt` 等第三方组件，需先编译：
```bash
# Windows, 需要在VS Powershell命令行环境中执行
& 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\Launch-VsDevShell.ps1' -arch=amd64
.\scripts\build_3rd.ps1

# macOS（终端）
sh ./scripts/build_3rd.sh
```

### 3. 编译项目
#### 方式1：使用 CMake 命令行
```bash
# 创建构建目录
mkdir build && cd build

# 加载 CMake 预设（以 Windows x64 Release 为例）
cmake --preset windows-x64-release

# 编译项目（-j 后接CPU核心数，加速编译）
cmake --build . --config Release -j 8
```

#### 方式2：使用 IDE（CLion/Qt Creator）
1. 打开项目根目录的 `CMakeLists.txt`；
2. 在 IDE 中配置 CMake 预设（选择对应平台/模式）；
3. 执行「Reload CMake Project」后，点击构建按钮。

### 4. 运行与测试
- 编译完成后，可执行文件位于 `build/coapp/Release`（Windows）或 `build/coapp`（macOS）；
- 运行前确保已接入对应硬件设备（脑电/手环/相机），然后运行可视化端（muapp）测试数据展示。

## 功能模块说明
| 模块     | 核心职责                         | 关键组件                                       |
|--------|------------------------------|--------------------------------------------|
| coapp  | 多设备数据采集、原始数据处理、设备通信管理        | BandServer、EEGReceiver、CameraService       |
| muapp  | 采集数据可视化、用户交互、状态展示            | SingleLineChart、MultiLineChart、MinMaxQueue |
| common | 全局配置、通用数据结构、工具函数，为其他模块提供基础支撑 | SettingsManager、CQueue                     |

---

## 总结
1. 本项目是基于Qt 6的跨平台多模态数据采集客户端，核心为设备数据采集+实时可视化，适用于教育场景的学习者状态感知；
2. 构建流程需先编译第三方依赖，再通过CMake（命令行/IDE）编译主项目，支持Windows/macOS多平台；
3. 项目采用模块化设计，coapp负责采集、muapp负责可视化、common提供通用能力，可通过GitHub Actions实现自动化构建。