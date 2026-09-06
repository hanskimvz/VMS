# VMS 개발 가이드

마지막 검토: 2026‑09‑06.

## 1. 개발 환경

### 1.1 도구 체인

| 구성요소 | 버전 (검증됨) |
|----------|---------------|
| OS | Windows 10/11 64‑bit |
| MSYS2 MinGW64 GCC | 15.2 |
| Qt6 | 6.10 |
| FFmpeg | 8.0.1 (`avcodec-62`, `avformat-62`, `avutil-60`, `swscale-9`, `swresample-6`) |
| CMake | 3.21+ |
| SQLite | 3.x (Qt SQL 드라이버 경유) |

### 1.2 설치

MSYS2 MinGW64 셸에서:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,qt6-base,qt6-tools,ffmpeg,sqlite3}
```

FFmpeg는 `pkg-config`로 찾는다. MinGW64 셸이 아니면 `PKG_CONFIG_PATH`에 `C:/msys64/mingw64/lib/pkgconfig`가 있어야 한다.

### 1.3 IDE

- VS Code: C/C++, CMake Tools 확장. 키트는 MSYS2 MinGW64 GCC.
- Qt Creator: Kit = MinGW 64‑bit, CMake = MSYS2 cmake.

## 2. 빌드

```bash
cd d:/Projects/VMS

# Debug (저장소의 build/는 이 설정으로 생성됨)
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4

# Release
cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j4

# Ninja (더 빠름)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build
```

실행: `./build/VMS.exe`. 로그는 `%APPDATA%/VMS/VMS/vms_debug.log`에 쓰인다. 빌드는 `-Wall -Wextra`에서 경고 0을 유지한다.

테스트 스위트와 린터는 없다. 검증은 빌드 → 실행 → 로그 확인이다.

## 3. 프로젝트 구조

```
VMS/
├── CMakeLists.txt
├── CLAUDE.md                # AI 코딩 도구용 요약
├── README.md
├── deploy.bat               # Release 빌드 → deploy/ 폴더 생성
├── cam_device_manager.bat   # Camera/ Python 도구 실행
├── doc/                     # 이 문서들 (doc/README.md가 목차)
├── src/
│   ├── main.cpp             # 로그 핸들러 설치, FFmpeg 네트워크 초기화, MainWindow
│   ├── core/                # CameraManager, StreamReceiver, OnvifClient, DeviceDiscovery,
│   │                        # PlaybackController, Recorder(미연결), VideoDecoder(미연결)
│   ├── ui/                  # MainWindow, VideoGrid, VideoWidget, CameraTree, DeviceManageWidget,
│   │                        # AddCameraDialog, NetworkSettingsDialog, PlaybackView, TimelineWidget,
│   │                        # PtzControl(미연결)
│   ├── utils/database.*     # SQLite 래퍼
│   └── image/logo.png
├── resources/resources.qrc
├── Camera/                  # 별도 PyQt5 도구: 특정 벤더 HTTP JSON API(/api/v1/...) 설정 도구. CMake 빌드와 무관
├── OnViF/discovery.py       # WS-Discovery 실험 스크립트
├── python-3.8.10-embed-amd64/  # Camera/ 도구용 임베디드 파이썬 (git 제외)
└── rapidvms/                # 참고용 원본 (git 제외, AGPL). 코드 복사 금지
```

## 4. 코딩 규칙

### 4.1 네이밍

| 항목 | 규칙 | 예 |
|------|------|----|
| 클래스 | PascalCase | `CameraManager` |
| 함수 | camelCase | `startStream()` |
| 슬롯 | `onXxx()` | `onCameraAdded()` |
| 멤버 변수 | `m_camelCase` | `m_cameras` |
| 상수 | UPPER_SNAKE | `MAX_RETRIES` |
| 파일 | snake_case | `camera_manager.cpp` |

### 4.2 헤더

`#ifndef CAMERA_MANAGER_H` 스타일 가드. `#pragma once`는 쓰지 않는다. FFmpeg 헤더는 반드시 `extern "C" { }`로 감싼다.

인클루드 경로는 평평하다. `src/`, `src/core`, `src/ui`, `src/utils`가 모두 include 경로라 `#include "camera_manager.h"`로 어디서든 접근한다.

### 4.3 새 파일 추가

`CMakeLists.txt`의 `CORE_SOURCES`/`CORE_HEADERS`, `UI_SOURCES`/`UI_HEADERS`, `UTILS_*`에 **헤더까지** 추가한다. AUTOMOC이 `Q_OBJECT` 헤더를 찾으려면 목록에 있어야 한다.

### 4.4 Qt 시그널/슬롯

```cpp
signals:
    void cameraAdded(const QString& id);
private slots:
    void onCameraAdded(const QString& id);

connect(m_cameraManager, &CameraManager::cameraAdded,
        this, &MainWindow::onCameraAdded);
```

스레드를 넘는 시그널은 값 타입(`VideoFrame`, `StreamStats`)만 실어 보낸다. 복사 가능한 구조체면 별도 등록 없이 큐드 커넥션이 된다. 원시 포인터를 스레드 너머로 보내지 않는다.

### 4.5 메모리

- Qt 객체는 부모‑자식으로 관리. `MainWindow`의 `CameraManager`, `OnvifClient`는 `unique_ptr`이면서 부모도 `this`다(먼저 `unique_ptr`이 지우고, 지워진 자식은 부모 목록에서 빠지므로 이중 해제는 없다).
- FFmpeg 컨텍스트를 가진 클래스는 `cleanup()`을 두고 소멸자와 재오픈 시 호출한다.
- `QObject`를 다른 객체가 참조 중일 때 지우려면 `deleteLater()`를 쓰고, 참조하는 쪽은 `QPointer`나 `destroyed` 시그널로 방어한다(known‑issues 1의 교훈).

### 4.6 로깅

`qDebug/qWarning/qCritical`만 쓴다. `main.cpp`의 핸들러가 stderr와 `%APPDATA%/VMS/VMS/vms_debug.log`로 보내며 뮤텍스로 직렬화되어 있어 워커 스레드에서 불러도 된다. 비밀번호가 포함된 URL이나 SOAP 본문을 로그에 남기지 않는다.

## 5. FFmpeg 패턴

### 5.1 RTSP 열기 (FFmpeg 8 옵션명)

```cpp
AVDictionary* options = nullptr;
av_dict_set(&options, "rtsp_transport", "tcp", 0);
av_dict_set(&options, "timeout", "5000000", 0);   // µs. FFmpeg 5+에서 stimeout 대신
av_dict_set(&options, "max_delay", "100000", 0);
int ret = avformat_open_input(&fmt, url, nullptr, &options);
av_dict_free(&options);   // 소비되지 않은 옵션이 남아 있으면 이름이 틀린 것
```

옵션명이 맞는지는 `ffmpeg -h demuxer=rtsp`로 확인한다.

### 5.2 블로킹 읽기 중단

```cpp
static int interruptCb(void* ctx) {
    return static_cast<StreamReceiver*>(ctx)->shouldStop() ? 1 : 0;
}
fmt->interrupt_callback.callback = interruptCb;
fmt->interrupt_callback.opaque = this;
```

`avformat_open_input` 전에 설정한다. 이것이 있으면 `QThread::terminate()`가 필요 없다.

### 5.3 디코드

```cpp
avcodec_send_packet(codecCtx, packet);
while (avcodec_receive_frame(codecCtx, frame) == 0) {
    // 프레임 처리
}
```

프레임 스레딩을 켰다면 `receive_frame`을 `EAGAIN`이 나올 때까지 반복해야 프레임을 잃지 않는다.

### 5.4 YUV → RGB

```cpp
sws_scale(swsCtx, frame->data, frame->linesize, 0, height,
          frameRGB->data, frameRGB->linesize);
QImage img(frameRGB->data[0], width, height, frameRGB->linesize[0], QImage::Format_RGB32);
emit frameReady(img.copy());   // 버퍼가 재사용되므로 반드시 복사
```

## 6. ONVIF 패턴

SOAP 본문은 `OnvifClient::create*Request()`에서 문자열로 만들고 `createSoapEnvelope()`가 WS‑Security 헤더를 붙인다. 새 명령을 추가하려면:

1. `createXxxRequest()` 작성, `SOAPAction` 헤더 값 확인(WSDL의 `wsa:Action`).
2. `m_networkManager->post()` 후 `reply->setProperty("requestType", "Xxx")`.
3. `onHttpFinished()`의 분기에 `parseXxxResponse()` 추가.
4. 응답은 `QXmlStreamReader`로 로컬 이름만 비교한다(네임스페이스 접두사는 카메라마다 다르다).

WS‑Security PasswordDigest = `Base64(SHA1(nonce + created + password))`, `created`는 UTC `yyyy-MM-ddTHH:mm:ssZ`. 카메라와 PC의 시계가 5분 이상 어긋나면 인증이 실패하는 카메라가 있다.

## 7. 디버깅

```bash
export QT_DEBUG_PLUGINS=1                       # 플러그인 로딩 문제
export QT_LOGGING_RULES="qt.network.*=true"     # HTTP/소켓 상세
```

FFmpeg 에러 코드는 `av_strerror(ret, buf, sizeof buf)`로 문자열화한다.

공개 테스트 RTSP: `rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4`
ONVIF 시뮬레이터: ONVIF Device Test Tool (onvif.org)

## 8. 배포

1. Release 빌드를 `build/`에 만든다(`deploy.bat`이 `build\VMS.exe`를 찾는다).
2. `deploy.bat` 실행 → `windeployqt` + FFmpeg/MinGW DLL을 `deploy/`로 복사.
3. FFmpeg를 업그레이드하면 `deploy.bat`의 `FFMPEG_DLLS` 버전 접미사를 갱신한다. 누락 DLL은 MinGW64 셸에서 `ldd build/VMS.exe | grep mingw`로 확인.

## 9. 다음 작업

우선순위와 로드맵은 [known-issues.md](known-issues.md)와 [intelligent-vms-research.md](intelligent-vms-research.md) 14절을 본다.
