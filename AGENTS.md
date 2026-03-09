## Skills
A skill is a set of local instructions to follow that is stored in a `SKILL.md` file.

### Available skills
- HuskarUIExpert: Expert on HuskarUI components. Supports metadata query, component lookup/search/listing, and source verification for QML/C++ usage. (file: /Users/yangxin/Documents/CollectPlatform/Client/.codex/skills/HuskarUIExpert/SKILL.md)

### How to use skills
- Trigger: If the user explicitly asks for HuskarUI component guidance, API lookup, examples, or QML snippet generation based on HuskarUI, use `HuskarUIExpert`.
- Load minimally: Read `SKILL.md` first; only query `guide.metainfo.json` for specific components/keywords (do not load the entire JSON file).
- Preferred query method:
  - `python .codex/skills/HuskarUIExpert/query_metainfo.py .codex/skills/HuskarUIExpert/guide.metainfo.json <ComponentName_or_Keyword>`
  - `python .codex/skills/HuskarUIExpert/query_metainfo.py .codex/skills/HuskarUIExpert/guide.metainfo.json list`
- Fallback (without Python): use `grep -n` to locate a component in `guide.metainfo.json`, then read only nearby ranges.

### Notes
- Source origin: `3rdparty/HuskarUI/ai_tools/HuskarUIExpert`
- Local reusable copy location: `.codex/skills/HuskarUIExpert/`

## Project Context

### Overview
- This repository is a Qt 6 based multi-modal data collection and visualization client.
- It currently contains three core modules:
  - `coapp`: student-side collection terminal (Qt Widgets, C++).
  - `muapp`: multimodal visualization/perception app (Qt Quick/QML + C++).
  - `common`: shared static library for settings and data model serialization.

### Top-Level Structure
- `CMakeLists.txt`: root build entry, includes `common`, `coapp`, `muapp`.
- `CMakePresets.json`: cross-platform build presets (Windows x64/arm64, macOS arm64; debug/release/multi-config).
- `vcpkg.json`: dependency management integration via vcpkg toolchain.
- `3rdparty/`
  - `HuskarUI/`: third-party UI library (submodule-style dependency).
  - `patches/HuskarUI.diff`: local patch for HuskarUI.
- `scripts/`
  - `build_3rd.ps1`, `build_3rd.sh`: build/install third-party dependencies (notably `HuskarUI`, `Qt6Mqtt`).

### Module Breakdown

#### 1) `coapp` (Student Collection Terminal)
- App display name from source: `Student Collection Terminal`.
- Tech: Qt Widgets + C++17.
- Main responsibility: device/data collection and upstream transport.
- Key source layout:
  - `coapp/src/services/`
    - `EEGReceiver.*`: EEG data ingest.
    - `BandServer.*`: wristband data service.
    - `CameraService.*`: camera capture.
    - `VideoPushService.*`, `FFmpegHelper.*`: video stream push/codec helper.
    - `MqttPublisher.*`: MQTT publish pipeline.
    - `DataPipe.*`, `DataSerializer.*`: data flow/serialization integration.
    - `CoSettingsMgr.*`: app settings manager wrapper.
  - `coapp/src/views/`: main widgets pages (`MainWindow`, `DeviceView`, `EEGView`, `BandView`, `CameraView`, `SettingView`).
  - `coapp/src/views/settings/`: settings panels (camera/eeg/band/mqtt/stream/info).
  - `coapp/src/components/`: reusable widgets (`LogBox`, `BarCard`, `RecordingIndicator`, etc.).
  - `coapp/resources/`: qss, icons, i18n (`CoApp_zh_CN.ts`, `CoApp_en.ts`), macOS plist template.
- Build deps from CMake:
  - Qt6: `Core Widgets Svg Network Mqtt Multimedia LinguistTools`.
  - `FFMPEG`.
  - `PlatformCommon` (from `common`).

#### 2) `muapp` (Visualization/Perception App)
- App display name from source: `多模态数据采集与学习者状态实时感知平台`.
- Tech: Qt Quick/QML + C++20.
- Main responsibility: realtime data pull, processing, and chart/UI rendering.
- Key source layout:
  - `muapp/ui/Pages/`: `HomePage`, `RealtimePage`, `OfflinePage`, `StoragePage`, `SettingPage`.
  - `muapp/ui/Views/`: `EEGView.qml`, `BandView.qml`.
  - `muapp/ui/Components/Charts/`: chart primitives and helpers (`SingleLineChart`, `MultiLineChart`, axes/themes/util).
  - `muapp/src/services/`
    - `DataStreamService.*`: stream data service.
    - `VideoPullService.*`: video pull service.
    - `MuSettingsMgr.*`: settings manager.
    - `network/WebsocketClient.*`, `network/WebsocketMgr.*`: websocket communication.
  - `muapp/src/controllers/`: view controllers for EEG/Band.
  - `muapp/src/models/`: frame model + `MinMaxQueue` (realtime extrema/stat support).
- Build deps from CMake:
  - Qt6: `Core WebSockets Quick Qml Graphs Multimedia`.
  - `HuskarUI`.
  - `FFMPEG`.
  - `PlatformCommon` (from `common`).

#### 3) `common` (Shared Library)
- CMake target: static library `PlatformCommon`.
- Tech: C++17.
- Responsibility: shared infra/models for both apps.
- Key content:
  - `src/SettingsManager.*`: unified settings persistence abstraction.
  - `src/model/`
    - `CQueue.*`: queue abstraction.
    - `ISensorData.*`, `ISensorSerializer.*`, `serialize.h`: sensor abstraction and serialization interface.
    - `EEGData.*`, `WristbandData.*`: concrete data models.
- Build deps:
  - `msgpack-cxx`.
  - Qt6 `Core`.

### Build & Packaging Flow
- Root CMake injects:
  - `BINARY_OUT_DIRECTORY` default: `${sourceDir}/../output`.
  - `CMAKE_PREFIX_PATH += ${BINARY_OUT_DIRECTORY}/3rdparty`.
- Typical order:
  1. Build third-party dependencies via `scripts/build_3rd.*`.
  2. Configure with `CMakePresets.json` preset.
  3. Build targets (`coapp`, `muapp`, `common`).
  4. Auto deploy:
    - Windows: `windeployqt`.
    - macOS: `macdeployqt`.

### Current Role Mapping (based on code evidence)
- `coapp` = student-side data collection terminal.
- `muapp` = multimodal visualization/perception terminal.
- Note: if future product naming changes teacher/student mapping, re-check `main.cpp` display names and runtime usage.

### Quick Navigation Index
- Root build: `CMakeLists.txt`, `CMakePresets.json`.
- Student collection app: `coapp/`.
- Visualization app: `muapp/`.
- Shared models/settings: `common/`.
- 3rd-party build scripts: `scripts/build_3rd.ps1`, `scripts/build_3rd.sh`.

### Last Updated
- Date: 2026-03-09
- Source basis: repository structure + root/module CMake + app entrypoints.
