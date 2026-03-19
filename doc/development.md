# VMS 개발 가이드

## 개발 환경 설정

### 1. MSYS2 설치

[MSYS2](https://www.msys2.org/)를 다운로드하여 설치합니다.

### 2. 필수 패키지 설치

MSYS2 MinGW64 셸에서 다음 명령을 실행합니다:

```bash
# 시스템 업데이트
pacman -Syu

# 개발 도구
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja

# Qt6
pacman -S mingw-w64-x86_64-qt6-base
pacman -S mingw-w64-x86_64-qt6-tools

# FFmpeg
pacman -S mingw-w64-x86_64-ffmpeg

# SQLite
pacman -S mingw-w64-x86_64-sqlite3
```

### 3. IDE 설정

**VS Code 권장 확장**:
- C/C++ (Microsoft)
- CMake Tools
- Qt tools

**Qt Creator**:
- Kit: MinGW 64-bit
- CMake: MSYS2 cmake

---

## 빌드

### Debug 빌드

```bash
mkdir build-debug && cd build-debug
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j4
```

### Release 빌드

```bash
mkdir build-release && cd build-release
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j4
```

### Ninja 사용 (더 빠름)

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

---

## 프로젝트 구조

```
src/
├── main.cpp                 # 애플리케이션 진입점
├── core/                    # 비즈니스 로직
│   ├── camera.h             # 카메라 데이터 구조
│   ├── camera_manager.*     # 카메라 관리
│   ├── stream_receiver.*    # RTSP 스트림 수신
│   ├── video_decoder.*      # 비디오 디코딩
│   ├── recorder.*           # 녹화
│   ├── onvif_client.*       # ONVIF 클라이언트
│   └── playback_controller.*# 재생 컨트롤
├── ui/                      # 사용자 인터페이스
│   ├── main_window.*        # 메인 윈도우
│   ├── video_widget.*       # 비디오 표시 위젯
│   ├── video_grid.*         # 비디오 그리드
│   ├── camera_tree.*        # 카메라 트리뷰
│   ├── playback_view.*      # 재생 화면
│   ├── timeline_widget.*    # 타임라인
│   ├── ptz_control.*        # PTZ 컨트롤
│   └── add_camera_dialog.*  # 카메라 추가 대화상자
└── utils/                   # 유틸리티
    └── database.*           # SQLite 래퍼
```

---

## 코딩 규칙

### 네이밍

| 항목 | 규칙 | 예시 |
|------|------|------|
| 클래스 | PascalCase | `CameraManager` |
| 함수 | camelCase | `startStream()` |
| 멤버 변수 | m_camelCase | `m_cameras` |
| 상수 | UPPER_SNAKE | `MAX_CAMERAS` |
| 파일 | snake_case | `camera_manager.cpp` |

### 헤더 가드

```cpp
#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

// ...

#endif // CAMERA_MANAGER_H
```

### Qt 시그널/슬롯

```cpp
// 헤더
signals:
    void cameraAdded(const QString& id);

private slots:
    void onCameraAdded(const QString& id);

// 연결
connect(m_cameraManager, &CameraManager::cameraAdded,
        this, &MainWindow::onCameraAdded);
```

### 메모리 관리

- Qt 객체는 부모-자식 관계로 관리
- Core 객체는 `std::unique_ptr` 또는 명시적 delete
- FFmpeg 리소스는 반드시 정리 함수 호출

```cpp
// Qt 객체
m_videoWidget = new VideoWidget(this);  // this가 부모

// Core 객체
m_cameraManager = std::make_unique<CameraManager>(this);

// FFmpeg
if (m_formatCtx) {
    avformat_close_input(&m_formatCtx);
}
```

---

## 새 기능 추가

### 1. 새 Core 클래스 추가

1. `src/core/`에 헤더와 소스 파일 생성
2. `CMakeLists.txt`의 `CORE_SOURCES`와 `CORE_HEADERS`에 추가
3. 필요한 Qt 모듈 의존성 확인

### 2. 새 UI 위젯 추가

1. `src/ui/`에 헤더와 소스 파일 생성
2. `CMakeLists.txt`의 `UI_SOURCES`와 `UI_HEADERS`에 추가
3. `MainWindow`에서 인스턴스 생성 및 배치

### 3. 새 데이터베이스 테이블 추가

1. `Database::createTables()`에 CREATE TABLE 추가
2. CRUD 함수 구현
3. 관련 Core 클래스에서 호출

---

## FFmpeg 사용

### 스트림 열기

```cpp
AVFormatContext* formatCtx = avformat_alloc_context();
AVDictionary* options = nullptr;
av_dict_set(&options, "rtsp_transport", "tcp", 0);

int ret = avformat_open_input(&formatCtx, url, nullptr, &options);
av_dict_free(&options);

if (ret < 0) {
    // 에러 처리
}

ret = avformat_find_stream_info(formatCtx, nullptr);
```

### 디코더 초기화

```cpp
AVCodecParameters* codecpar = formatCtx->streams[videoIndex]->codecpar;
const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
AVCodecContext* codecCtx = avcodec_alloc_context3(codec);

avcodec_parameters_to_context(codecCtx, codecpar);
avcodec_open2(codecCtx, codec, nullptr);
```

### 프레임 디코딩

```cpp
AVPacket* packet = av_packet_alloc();
AVFrame* frame = av_frame_alloc();

while (av_read_frame(formatCtx, packet) >= 0) {
    if (packet->stream_index == videoIndex) {
        avcodec_send_packet(codecCtx, packet);
        if (avcodec_receive_frame(codecCtx, frame) >= 0) {
            // 프레임 처리
        }
    }
    av_packet_unref(packet);
}
```

### YUV → RGB 변환

```cpp
SwsContext* swsCtx = sws_getContext(
    width, height, AV_PIX_FMT_YUV420P,
    width, height, AV_PIX_FMT_RGB32,
    SWS_BILINEAR, nullptr, nullptr, nullptr
);

sws_scale(swsCtx,
    frame->data, frame->linesize, 0, height,
    frameRGB->data, frameRGB->linesize
);

QImage image(frameRGB->data[0], width, height, 
             frameRGB->linesize[0], QImage::Format_RGB32);
```

---

## ONVIF 구현

### WS-Discovery Probe

```xml
<?xml version="1.0" encoding="UTF-8"?>
<e:Envelope xmlns:e="http://www.w3.org/2003/05/soap-envelope"
            xmlns:w="http://schemas.xmlsoap.org/ws/2004/08/addressing"
            xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery"
            xmlns:dn="http://www.onvif.org/ver10/network/wsdl">
  <e:Header>
    <w:MessageID>uuid:...</w:MessageID>
    <w:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</w:To>
    <w:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</w:Action>
  </e:Header>
  <e:Body>
    <d:Probe>
      <d:Types>dn:NetworkVideoTransmitter</d:Types>
    </d:Probe>
  </e:Body>
</e:Envelope>
```

### GetProfiles 요청

```xml
<trt:GetProfiles xmlns:trt="http://www.onvif.org/ver10/media/wsdl"/>
```

### GetStreamUri 요청

```xml
<trt:GetStreamUri xmlns:trt="http://www.onvif.org/ver10/media/wsdl">
  <trt:StreamSetup>
    <tt:Stream xmlns:tt="http://www.onvif.org/ver10/schema">RTP-Unicast</tt:Stream>
    <tt:Transport xmlns:tt="http://www.onvif.org/ver10/schema">
      <tt:Protocol>RTSP</tt:Protocol>
    </tt:Transport>
  </trt:StreamSetup>
  <trt:ProfileToken>profile_token</trt:ProfileToken>
</trt:GetStreamUri>
```

### WS-Security (UsernameToken)

```xml
<wsse:Security xmlns:wsse="http://docs.oasis-open.org/wss/...">
  <wsse:UsernameToken>
    <wsse:Username>admin</wsse:Username>
    <wsse:Password Type="...#PasswordDigest">base64(sha1(nonce+created+password))</wsse:Password>
    <wsse:Nonce>base64(nonce)</wsse:Nonce>
    <wsu:Created>2024-01-01T00:00:00Z</wsu:Created>
  </wsse:UsernameToken>
</wsse:Security>
```

---

## 디버깅

### 로그 출력

```cpp
#include <QDebug>

qDebug() << "Camera connected:" << camera.name;
qWarning() << "Stream error:" << error;
qCritical() << "Fatal error:" << message;
```

### FFmpeg 에러 처리

```cpp
char errbuf[256];
av_strerror(ret, errbuf, sizeof(errbuf));
qWarning() << "FFmpeg error:" << errbuf;
```

### Qt 디버그 환경변수

```bash
export QT_DEBUG_PLUGINS=1
export QT_LOGGING_RULES="qt.network.*=true"
```

---

## 테스트

### 테스트 RTSP 스트림

공개 RTSP 테스트 스트림:
- `rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4`

### ONVIF 시뮬레이터

- [ONVIF Device Test Tool](https://www.onvif.org/profiles/conformance/)
- 실제 IP 카메라 없이 ONVIF 기능 테스트 가능

---

## 배포

### 필요 DLL 확인

```bash
ldd VMS.exe | grep mingw
```

### 배포 패키지 구성

```
VMS/
├── VMS.exe
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6Network.dll
├── Qt6Sql.dll
├── Qt6OpenGLWidgets.dll
├── avcodec-*.dll
├── avformat-*.dll
├── avutil-*.dll
├── swscale-*.dll
├── swresample-*.dll
├── platforms/
│   └── qwindows.dll
├── sqldrivers/
│   └── qsqlite.dll
└── styles/
    └── qwindowsvistastyle.dll
```

### windeployqt 사용

```bash
windeployqt --release VMS.exe
```

---

## 향후 개발 계획

### 단기

- [ ] 하드웨어 가속 디코딩 (VAAPI, NVDEC)
- [ ] 오디오 재생 지원
- [ ] 스냅샷 내보내기

### 중기

- [ ] 모션 감지
- [ ] 이벤트 알람
- [ ] 녹화 스케줄링

### 장기

- [ ] 클라이언트-서버 아키텍처
- [ ] 웹 인터페이스
- [ ] AI 분석 (객체 감지)
