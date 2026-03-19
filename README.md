# VMS - Video Management System

Qt6와 FFmpeg 기반의 IP 카메라 관리 소프트웨어입니다.

## 주요 기능

- **라이브 뷰**: 다중 카메라 실시간 모니터링 (1x1, 2x2, 3x3, 4x4 레이아웃)
- **ONVIF 지원**: 카메라 자동 검색 및 프로필 획득
- **PTZ 제어**: 팬/틸트/줌 원격 제어
- **녹화**: FFmpeg 기반 MP4/MKV 녹화
- **재생**: 타임라인 기반 녹화 영상 재생
- **카메라 관리**: SQLite 데이터베이스 기반 카메라 정보 저장

## 스크린샷

```
┌─────────────────────────────────────────────────────────────────┐
│ File  View  Help                                    [─][□][×]   │
├──────────┬──────────────────────────────────────────────────────┤
│ Cameras  │  ┌────────────┐  ┌────────────┐                      │
│ ├ Online │  │  Camera 1  │  │  Camera 2  │                      │
│ │ └ Cam1 │  │            │  │            │                      │
│ │ └ Cam2 │  │   [LIVE]   │  │   [LIVE]   │                      │
│ └ Offline│  └────────────┘  └────────────┘                      │
│          │  ┌────────────┐  ┌────────────┐                      │
│ PTZ      │  │  Camera 3  │  │  Camera 4  │                      │
│ [↖][↑][↗]│  │            │  │            │                      │
│ [←]   [→]│  │ No Signal  │  │ No Signal  │                      │
│ [↙][↓][↘]│  └────────────┘  └────────────┘                      │
├──────────┴──────────────────────────────────────────────────────┤
│ [Live View] [Playback]                                          │
└─────────────────────────────────────────────────────────────────┘
```

## 시스템 요구사항

- Windows 10/11 (64-bit)
- MSYS2 MinGW64
- Qt 6.x
- FFmpeg 6.x 이상

## 빌드 방법

### 필수 패키지 설치

```bash
pacman -S mingw-w64-x86_64-qt6-base
pacman -S mingw-w64-x86_64-ffmpeg
pacman -S mingw-w64-x86_64-sqlite3
pacman -S mingw-w64-x86_64-cmake
```

### 빌드

```bash
cd d:/Projects/VMS
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j4
```

### 실행

```bash
./VMS.exe
```

## 프로젝트 구조

```
VMS/
├── CMakeLists.txt          # CMake 빌드 설정
├── README.md               # 이 문서
├── doc/                    # 문서
│   ├── architecture.md     # 아키텍처 설명
│   ├── features.md         # 기능 상세
│   └── development.md      # 개발 가이드
├── src/
│   ├── main.cpp            # 진입점
│   ├── core/               # 핵심 로직
│   │   ├── camera.h/cpp
│   │   ├── camera_manager.h/cpp
│   │   ├── stream_receiver.h/cpp
│   │   ├── video_decoder.h/cpp
│   │   ├── recorder.h/cpp
│   │   ├── onvif_client.h/cpp
│   │   └── playback_controller.h/cpp
│   ├── ui/                 # UI 레이어
│   │   ├── main_window.h/cpp
│   │   ├── video_widget.h/cpp
│   │   ├── video_grid.h/cpp
│   │   ├── camera_tree.h/cpp
│   │   ├── playback_view.h/cpp
│   │   ├── timeline_widget.h/cpp
│   │   ├── ptz_control.h/cpp
│   │   └── add_camera_dialog.h/cpp
│   └── utils/
│       └── database.h/cpp
├── resources/
│   └── resources.qrc
└── rapidvms/               # 참고용 원본 코드 (veyesys)
```

## 기술 스택

| 구성요소 | 기술 | 버전 |
|---------|------|------|
| GUI Framework | Qt6 Widgets | 6.10.1 |
| 비디오 처리 | FFmpeg | 8.0.1 |
| 빌드 시스템 | CMake | 3.21+ |
| 데이터베이스 | SQLite | 3.x |
| 컴파일러 | GCC (MinGW64) | 15.2 |
| 프로토콜 | ONVIF, RTSP | - |

## 참고 프로젝트

이 프로젝트는 [rapidvms](https://github.com/veyesys/rapidvms)의 아키텍처와 기능 구현을 참고하여 개발되었습니다.

- rapidvms는 GNU AGPL v3 라이선스입니다.
- 본 프로젝트는 rapidvms를 참고하되, Qt6 + MinGW64 환경에 맞게 새로 작성되었습니다.

## 라이선스

이 프로젝트는 개인/학습 목적으로 개발되었습니다.

## 연락처

질문이나 제안이 있으시면 이슈를 등록해주세요.
