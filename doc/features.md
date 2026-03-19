# VMS 기능 상세

## 1. 라이브 뷰

### 1.1 비디오 그리드

다중 카메라를 동시에 모니터링할 수 있는 그리드 레이아웃을 제공합니다.

| 레이아웃 | 카메라 수 | 단축키 |
|----------|----------|--------|
| 1x1 | 1 | `1` |
| 2x2 | 4 | `2` |
| 3x3 | 9 | `3` |
| 4x4 | 16 | `4` |

**구현 파일**: `src/ui/video_grid.cpp`

```cpp
void VideoGrid::setLayout(int cellCount) {
    // 1, 4, 9, 16 중 하나
    m_cellCount = cellCount;
    createWidgets(cellCount);
    arrangeWidgets();
}
```

### 1.2 비디오 위젯

각 카메라 스트림을 표시하는 위젯입니다.

**기능**:
- 비디오 프레임 표시
- 카메라 이름 오버레이
- 선택 상태 표시 (파란색 테두리)
- 컨텍스트 메뉴 지원
- 더블클릭으로 스트림 시작

**구현 파일**: `src/ui/video_widget.cpp`

### 1.3 카메라 트리

좌측 도킹 패널에 카메라 목록을 트리 구조로 표시합니다.

**기능**:
- Online/Offline 카메라 분류
- 드래그 앤 드롭 지원
- 컨텍스트 메뉴 (시작/중지/편집/삭제)

**구현 파일**: `src/ui/camera_tree.cpp`

---

## 2. 카메라 관리

### 2.1 카메라 추가

카메라를 수동으로 추가하거나 ONVIF 검색으로 자동 추가할 수 있습니다.

**수동 추가**:
- 카메라 이름
- IP 주소 / 포트
- 사용자명 / 비밀번호
- RTSP URL (선택)

**ONVIF 검색**:
1. "Discover" 버튼 클릭
2. 네트워크에서 ONVIF 카메라 자동 검색
3. 검색된 카메라 선택
4. 프로필 및 스트림 URL 자동 획득

**구현 파일**: `src/ui/add_camera_dialog.cpp`

### 2.2 카메라 정보 저장

카메라 정보는 SQLite 데이터베이스에 저장됩니다.

**저장 경로**: `%APPDATA%/VMS/vms.db`

**테이블 구조**:
```sql
CREATE TABLE cameras (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    ip TEXT NOT NULL,
    port INTEGER DEFAULT 80,
    username TEXT,
    password TEXT,
    rtsp_url TEXT,
    rtsp_url_sub TEXT,
    onvif_path TEXT DEFAULT '/onvif/device_service',
    type INTEGER DEFAULT 0,
    recording INTEGER DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

**구현 파일**: `src/utils/database.cpp`

---

## 3. ONVIF 지원

### 3.1 WS-Discovery

네트워크에서 ONVIF 호환 카메라를 자동으로 검색합니다.

**프로토콜**:
- 멀티캐스트 주소: `239.255.255.250`
- 포트: `3702`
- 메시지: SOAP Probe

**구현 파일**: `src/core/onvif_client.cpp`

```cpp
void OnvifClient::discover(int timeout) {
    // UDP 소켓으로 Probe 전송
    QString probeMsg = createWsDiscoveryProbe();
    m_discoverySocket->writeDatagram(
        probeMsg.toUtf8(), 
        QHostAddress("239.255.255.250"), 
        3702
    );
}
```

### 3.2 프로필 획득

카메라의 비디오 프로필 목록을 가져옵니다.

**획득 정보**:
- 프로필 토큰
- 해상도 (Width x Height)
- 프레임 레이트
- 인코딩 (H.264, H.265, MJPEG)

### 3.3 스트림 URL 획득

선택한 프로필의 RTSP URL을 가져옵니다.

```cpp
void OnvifClient::getStreamUri(const QString& serviceUrl, 
                                const QString& profileToken) {
    // SOAP GetStreamUri 요청
    // -> rtsp://camera_ip:554/profile_path
}
```

### 3.4 PTZ 제어

팬(Pan), 틸트(Tilt), 줌(Zoom)을 제어합니다.

**지원 동작**:
| 동작 | 설명 |
|------|------|
| Up | 위로 이동 |
| Down | 아래로 이동 |
| Left | 왼쪽 이동 |
| Right | 오른쪽 이동 |
| UpLeft | 왼쪽 위 대각선 |
| UpRight | 오른쪽 위 대각선 |
| DownLeft | 왼쪽 아래 대각선 |
| DownRight | 오른쪽 아래 대각선 |
| ZoomIn | 확대 |
| ZoomOut | 축소 |
| Stop | 정지 |

**ONVIF PTZ 명령**:
- `ContinuousMove`: 연속 이동 시작
- `Stop`: 이동 정지

**구현 파일**: `src/ui/ptz_control.cpp`, `src/core/onvif_client.cpp`

---

## 4. 스트리밍

### 4.1 RTSP 수신

FFmpeg의 `libavformat`을 사용하여 RTSP 스트림을 수신합니다.

**지원 프로토콜**:
- RTSP over TCP
- RTSP over UDP

**설정 옵션**:
```cpp
av_dict_set(&options, "rtsp_transport", "tcp", 0);  // TCP 사용
av_dict_set(&options, "stimeout", "5000000", 0);    // 5초 타임아웃
av_dict_set(&options, "max_delay", "500000", 0);    // 최대 지연
```

**구현 파일**: `src/core/stream_receiver.cpp`

### 4.2 비디오 디코딩

FFmpeg의 `libavcodec`을 사용하여 비디오를 디코딩합니다.

**지원 코덱**:
- H.264 (AVC)
- H.265 (HEVC)
- MJPEG
- MPEG4

**디코딩 흐름**:
```
AVPacket → avcodec_send_packet → avcodec_receive_frame → AVFrame
AVFrame → sws_scale → RGB32 → QImage
```

### 4.3 프레임 렌더링

`QPainter`를 사용하여 비디오 프레임을 화면에 그립니다.

**최적화**:
- 33ms 간격으로 화면 갱신 (약 30fps)
- 화면 크기에 맞게 스케일링
- 비율 유지 (Aspect Ratio)

---

## 5. 녹화

### 5.1 녹화 시작/중지

카메라 스트림을 파일로 저장합니다.

**지원 컨테이너**:
- MP4
- MKV
- TS

**녹화 정보**:
- 카메라 ID
- 파일 경로
- 시작/종료 시간
- 파일 크기

### 5.2 녹화 파일 관리

녹화 정보는 SQLite에 저장됩니다.

```sql
CREATE TABLE recordings (
    id TEXT PRIMARY KEY,
    camera_id TEXT NOT NULL,
    file_path TEXT NOT NULL,
    start_time DATETIME NOT NULL,
    end_time DATETIME,
    file_size INTEGER DEFAULT 0,
    has_audio INTEGER DEFAULT 0
);
```

**구현 파일**: `src/core/recorder.cpp`

---

## 6. 재생

### 6.1 파일 재생

녹화된 비디오 파일을 재생합니다.

**컨트롤**:
- 재생 / 일시정지
- 정지
- 탐색 (시간 이동)
- 재생 속도 (0.25x ~ 4x)

### 6.2 타임라인

타임라인 위젯으로 녹화 구간을 시각화합니다.

**기능**:
- 녹화 구간 표시 (파란색)
- 알람 구간 표시 (빨간색)
- 현재 위치 표시 (노란색)
- 마우스 호버 시간 표시
- 휠 줌 (확대/축소)
- Shift + 드래그로 스크롤

**구현 파일**: `src/ui/timeline_widget.cpp`, `src/ui/playback_view.cpp`

---

## 7. 설정

### 7.1 윈도우 레이아웃 저장

윈도우 위치, 크기, 도킹 상태가 자동으로 저장됩니다.

**저장 위치**: Windows Registry
- `HKEY_CURRENT_USER\Software\VMS\VMS`

### 7.2 다크 테마

모던한 다크 테마가 기본 적용됩니다.

**색상 팔레트**:
- 배경: `#2d2d30`
- 전경: `#ffffff`
- 강조: `#007acc`

---

## 8. 단축키

| 단축키 | 기능 |
|--------|------|
| `Ctrl+N` | 카메라 추가 |
| `Ctrl+D` | 카메라 검색 |
| `Ctrl+,` | 설정 |
| `F1` | 라이브 뷰 |
| `F2` | 재생 |
| `F11` | 전체 화면 |
| `1` | 1x1 레이아웃 |
| `2` | 2x2 레이아웃 |
| `3` | 3x3 레이아웃 |
| `4` | 4x4 레이아웃 |
| `Alt+F4` | 종료 |

---

## 9. rapidvms 참고 항목

본 프로젝트는 rapidvms의 다음 파일들을 참고하여 개발되었습니다.

| 기능 | rapidvms 참고 파일 |
|------|-------------------|
| 카메라 관리 | `veuilib/src/server/camera.cpp` |
| ONVIF 클라이언트 | `veuilib/onvifcpplib/include/onvifclient*.hpp` |
| PTZ 제어 | `veuilib/src/vvidonvif/vvidonvifc.cpp` |
| 비디오 그리드 | `veuilib/src/vvidwidget/vscvideowall.cpp` |
| 비디오 위젯 | `veuilib/src/vvidwidget/vscvwidget.cpp` |
| 녹화 | `velib/include/vdb/recordsession.hpp` |
| 재생 | `velib/include/vdb/pbsession.hpp` |
| 타임라인 | `veuilib/src/cmnui/vvidtimeline.h` |
| FFmpeg 래퍼 | `xcmnlib/src/ffkit/source/*.cpp` |
