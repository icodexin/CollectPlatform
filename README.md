# 多模态数据采集与学习者状态实时感知平台客户端
Multi-modal Data Collection & Learner State Real-time Perception Platform (Client)

## 项目简介
本项目基于 Qt 6 开发，面向教育场景的多模态数据采集与状态感知。平台支持脑电、手环、相机等数据源的实时接入、传输与可视化，采用模块化设计，支持 Windows/macOS 多平台构建与部署。

## 架构概览
当前仓库由 3 个核心模块组成：

1. `coapp`（Qt Widgets, C++17）
   - 学生端采集终端（应用显示名：`Student Collection Terminal`）。
   - 负责设备接入、数据采集、视频推流、MQTT 发布、采集配置管理。
2. `muapp`（Qt Quick/QML, C++20）
   - 多模态可视化与状态感知终端。
   - 负责实时数据拉取、图表渲染、页面交互、WebSocket 通信。
3. `common`（静态库）
   - 公共基础能力：配置管理、传感器数据模型、序列化与队列工具。

## 技术栈与依赖
| 类别 | 内容 |
|---|---|
| 核心框架 | Qt 6（Widgets / Quick / QML） |
| 构建系统 | CMake + CMake Presets + vcpkg toolchain |
| 编译器 | MSVC（Windows）、Clang（macOS） |
| 核心依赖 | Qt Graphs、Qt Multimedia、Qt WebSockets、Qt MQTT、msgpack-cxx、FFmpeg |
| 第三方 UI | HuskarUI |

## 目录结构
```text
.
├── 3rdparty/
│   ├── HuskarUI/                    # 第三方 UI 组件库
│   └── patches/HuskarUI.diff        # HuskarUI 本地补丁
├── common/                          # 共享静态库 PlatformCommon
│   └── src/
│       ├── SettingsManager.*        # 公共配置管理
│       └── model/                   # EEG/Wristband 数据模型与序列化
├── coapp/                           # 学生端采集应用（Qt Widgets）
│   ├── src/
│   │   ├── services/                # EEG/Band/Camera/MQTT/推流等服务
│   │   ├── views/                   # 主窗口与页面
│   │   └── components/              # 复用 UI 组件
│   └── resources/                   # 图标、QSS、i18n、macOS plist
├── muapp/                           # 可视化应用（Qt Quick + QML）
│   ├── src/
│   │   ├── controllers/             # EEG/Band 视图控制
│   │   ├── models/                  # 帧模型与 MinMaxQueue
│   │   └── services/                # 数据流、视频拉流、WebSocket
│   ├── ui/                          # QML Pages/Views/Components
│   └── resources/                   # 图标与应用资源
├── scripts/
│   ├── build_3rd.ps1                # Windows 第三方依赖构建
│   └── build_3rd.sh                 # macOS 第三方依赖构建
├── CMakeLists.txt                   # 根 CMake，聚合 common/coapp/muapp
├── CMakePresets.json                # 平台预设（debug/release/multi-config）
└── vcpkg.json                       # vcpkg 依赖声明
```

## 构建说明

### 1. 环境准备
- Qt 6（需包含项目用到的模块：Core、Widgets、Quick、Qml、Graphs、Multimedia、WebSockets、Mqtt 等）
- CMake 3.20+
- vcpkg（配置 `VCPKG_ROOT`）
- Windows: Visual Studio 2022 + C++ 工具链
- macOS: Xcode Command Line Tools

必须配置以下环境变量（`CMakePresets.json` 会直接读取）：

1. `QT6_DIR`, Qt6 的安装路径。需要定位到 `Qt_ROOT/<ver>/<arch>` 目录，如`.../6.8.3/msvc2022_64/`
2. `VCPKG_ROOT`, vcpkg 根目录（包含 `vcpkg`/`vcpkg.exe` 可执行文件的路径）。

### 2. 构建第三方依赖（先执行）
```bash
# macOS / Linux
sh ./scripts/build_3rd.sh
```

```powershell
# Windows（需要先进入VS专用Powershell环境中，注意更换为你机器上实际VS路径）
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64
.\scripts\build_3rd.ps1
```

### 3. 使用 CMake Presets 构建
常用配置预设（`configurePresets`）：
- `debug` / `release`（按当前平台自动选择）
- `debug-windows-x64` / `release-windows-x64`
- `debug-macos-arm64` / `release-macos-arm64`
- `windows-x64` / `windows-arm64` / `macos-arm64`（Multi-Config）

示例：
```bash
# 以当前平台 release 为例
cmake --preset release
cmake --build --preset release-build -j 8
```

或指定平台：
```bash
cmake --preset release-macos-arm64
cmake --build --preset release-build -j 8
```

### 4. 产物与部署
- 根 CMake 默认输出目录：`../output`（可通过 `BINARY_OUT_DIRECTORY` 覆盖）
- 应用打包目录：`../output/app/<CONFIG>/coapp`、`../output/app/<CONFIG>/muapp`
- 构建后会调用：
  - Windows：`windeployqt`
  - macOS：`macdeployqt`

## 模块职责
| 模块 | 职责 | 代表组件 |
|---|---|---|
| `coapp` | 设备接入、采集与上传 | `EEGReceiver`、`BandServer`、`CameraService`、`MqttPublisher`、`VideoPushService` |
| `muapp` | 数据拉取、处理与可视化 | `DataStreamService`、`WebsocketClient`、`EEGViewController`、`BandViewController`、QML Charts |
| `common` | 通用模型与基础设施 | `SettingsManager`、`ISensorData`、`ISensorSerializer`、`CQueue` |

## 备注
- 文档内容已按当前仓库代码结构（2026-03-09）同步。
- 若后续产品命名变化（如学生端/教师端定义调整），请以各模块 `main.cpp` 的显示名与实际部署场景为准更新本 README。
