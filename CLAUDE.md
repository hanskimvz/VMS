# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A standalone Windows IP‑camera Video Management System in C++17 / Qt6 / FFmpeg, built with MSYS2 MinGW64 + CMake. Docs and comments are in Korean.

**Product direction (2026‑09): "Intelligent VMS".** The goal is a 16‑channel VMS whose primary feature is VLM‑based semantic search: capture a frame per channel on an interval, send it to a local/remote Analyzer service (Ollama + Gemma first, engine‑agnostic), store fixed‑schema JSON metadata, and let the user search in natural language and click through to playback. Read `doc/intelligent-vms-concept.md` (design, schema, Analyzer HTTP contract) and `doc/intelligent-vms-research.md` (engine choices, throughput math, roadmap) before working on anything AI‑related. `doc/known-issues.md` lists the review findings that must be fixed before always‑on 16‑channel sessions are viable; `doc/architecture.md` and `doc/features.md` describe only what the code does today. Loosely modeled on [rapidvms](https://github.com/veyesys/rapidvms) (AGPL), whose source may sit in the git‑ignored `rapidvms/` folder for reference only; never copy code from it.

There is a second, unrelated program in `Camera/`: a PyQt5 tool that drives a specific camera vendor's HTTP JSON API (`/api/v1/...`, token via `X-Token` header, endpoints listed in `Camera/cgis.py`). Run it with `cam_device_manager.bat`, which uses the git‑ignored embedded `python-3.8.10-embed-amd64/`. `OnViF/discovery.py` is a throwaway WS‑Discovery script. Neither is part of the CMake build.

## Build / run / deploy

All commands run from an **MSYS2 MinGW64** shell (or with `C:\msys64\mingw64\bin` on PATH). Toolchain: GCC 15, Qt 6.10, FFmpeg 8 (`avcodec-62`, `avformat-62`, `avutil-60`). FFmpeg is located via `pkg-config`, so `PKG_CONFIG_PATH` must resolve the MinGW64 `.pc` files.

```bash
pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,qt6-base,qt6-tools,ffmpeg,sqlite3}

# configure + build (the checked‑out build/ was generated with MinGW Makefiles, Debug)
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4

./build/VMS.exe        # log: %APPDATA%/VMS/VMS/vms_debug.log (qInstallMessageHandler in main.cpp)
```

`deploy.bat` (cmd.exe, run from repo root) expects a **Release** `build/VMS.exe`, runs `windeployqt`, and copies a hard‑coded FFmpeg/MinGW DLL list into `deploy/`. If FFmpeg is upgraded, update the `FFMPEG_DLLS` version suffixes in that script.

There is no test suite and no linter configured. Verification is manual: build (keep `-Wall -Wextra` at zero warnings), run, and check the log. A public RTSP test stream is listed in `doc/development.md`.

**Adding a source file requires editing `CMakeLists.txt`** (`CORE_SOURCES`/`CORE_HEADERS`, `UI_SOURCES`/`UI_HEADERS`, `UTILS_*`). Headers are listed explicitly because AUTOMOC needs them. Include paths are flat: `#include "camera_manager.h"` works from anywhere under `src/`.

## Architecture (the parts that span files)

**Ownership chain.** `main.cpp` → `MainWindow` owns `CameraManager` and `OnvifClient` via `unique_ptr` and hands raw pointers down to `CameraTree`, `DeviceManageWidget`, `VideoGrid`. `CameraManager` owns `Database` and one `StreamReceiver` per running camera (`QHash<id, StreamReceiver*>`). UI widgets never own receivers; they fetch them with `CameraManager::getStreamReceiver(id)` and connect to signals.

**Live video path.** `StreamReceiver` *is a* `QThread`. `run()` does `av_read_frame` → `avcodec_send/receive_frame` → `sws_scale` to RGB32 → emits `frameReady(VideoFrame)` (a `QImage` copy) across threads. `VideoWidget::setStreamReceiver` connects that signal and repaints with `QPainter`. There is no separate decoder thread and no OpenGL rendering. RTSP is forced to TCP with low‑latency flags in `StreamReceiver::open`; the demuxer option is `timeout` (µs), not the removed `stimeout`. `stop()` relies on `AVFormatContext::interrupt_callback` plus `wait()`; never reintroduce `QThread::terminate()`. UI code holds receivers only through `QPointer` because `CameraManager` deletes them (`deleteLater`) and announces it via `streamStopped`.

**Main/sub stream switching.** `MainWindow::shouldUseSubStream()` returns true whenever the grid layout is larger than 1×1. `updateStreamsForLayout()` restarts every running receiver with the other URL when the layout changes. `CameraInfo::getRtspUrl(useSub)` falls back to `rtsp://ip:554/stream1|stream2` when the DB has no URL, so a camera with an empty `rtsp_url_sub` will silently try `/stream2`.

**Two discovery mechanisms run in parallel** from `DeviceManageWidget::onStartSearch`: `OnvifClient::discover` (WS‑Discovery on 3702, followed by SOAP `GetDeviceInformation`/`GetCapabilities`/`GetProfiles`/`GetStreamUri`) and `DeviceDiscovery` (mDNS 5353, SSDP 1900). ONVIF hits are shown immediately; mDNS/UPnP hits are buffered in `m_pendingUpnpDevices`/`m_pendingMdnsDevices` and flushed only after both finish, deduplicated by IP against ONVIF results and already‑added cameras. **Multicast probes must go out per interface**: both classes create one socket per local IPv4 interface via `bindDiscoverySocket()` in `local_interfaces.h` (bind to the interface address + `setMulticastInterface`). A socket bound to `0.0.0.0` only reaches the default‑route interface, which on this dev machine is a VPN adapter, not the camera LAN. The Device Manage tab has an interface combo (default: all).

**OnvifClient is stateful and single‑request.** It hand‑builds SOAP with WS‑Security UsernameToken (`createSecurityHeader`), dispatches everything through one `QNetworkAccessManager`, and routes replies in `onHttpFinished` by inspecting the request. Results land in member fields (`m_capabilities`, `m_profiles`, `m_currentStreamUri`) and are also emitted as signals. Because there is one instance shared by `MainWindow`, `DeviceManageWidget`, `AddCameraDialog`, and `NetworkSettingsDialog`, overlapping requests to different cameras will clobber each other's state. `setCredentials` must be called before any SOAP call for that device.

**Persistence.** SQLite at `%APPDATA%/VMS/VMS/vms.db` (`QStandardPaths::AppDataLocation`), tables created in `Database::createTables()`. Window geometry/state go to `QSettings("VMS","VMS")` (registry). Passwords are stored in plaintext in the DB.

**Dead / unwired code, be aware before building on the docs.** `Recorder` and `VideoDecoder` compile but nothing instantiates them; `StreamReceiver` has no recorder hook and `PlaybackView` never queries `Database` for recordings. The recording sequence diagram in `doc/architecture.md` describes intended design, not current behavior. `PlaybackController` works only on a file chosen directly in `PlaybackView`. The "Record Schedule" and "Settings" tabs in `MainWindow` are placeholder pages.

## Conventions (from doc/development.md, followed in the code)

- Files `snake_case`, classes `PascalCase`, members `m_camelCase`, slots `onXxx()`. Classic `#ifndef` header guards, not `#pragma once`.
- FFmpeg headers are always wrapped in `extern "C" { }`. Every FFmpeg context owner has a `cleanup()` that must free in the destructor and on reopen.
- Cross‑thread data flows only through Qt signals carrying value types (`VideoFrame`, `StreamStats`). There is no `qRegisterMetaType`/`Q_DECLARE_METATYPE` anywhere; Qt 6 moc registers signal argument types automatically, so new signal payload structs just need to stay copyable.
- User‑facing strings are wrapped in `tr()`; UI text is English, comments/docs Korean.
- Logging goes through `qDebug/qWarning/qCritical`, which `main.cpp` tees to stderr and `vms_debug.log`.
