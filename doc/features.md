# VMS 기능 상세 (현재 구현 기준)

마지막 검토: 2026‑09‑06. 미구현 항목은 마지막 절에 모아 두었다. 계획 중인 지능형 기능은 [intelligent-vms-concept.md](intelligent-vms-concept.md)를 본다.

## 1. 라이브 뷰 (Main View 탭)

### 1.1 레이아웃

하단 레이아웃 바의 버튼으로 전환한다. 단축키는 없다.

| 레이아웃 | 셀 수 | 비고 |
|----------|-------|------|
| 1×1 | 1 | 메인 스트림 사용 |
| 2×2 | 4 | 서브 스트림 |
| 1+7 | 8 | 좌상단 3×3 크기 메인 + 우측 3 + 하단 4. 서브 스트림 |
| 3×3 | 9 | 서브 스트림 |
| 4×4 | 16 | 서브 스트림 |

- **메인/서브 자동 전환**: 셀 수가 1보다 크면 모든 실행 중 스트림을 서브 스트림으로, 1×1이면 메인 스트림으로 재접속한다. 서브 URL이 비어 있으면 `rtsp://ip:554/stream2`를 추측한다.
- **최대화 토글**: 셀 더블클릭으로 1×1 최대화 ↔ 이전 레이아웃 복귀.
- **통계 오버레이**: `F2`로 해상도, FPS, 평균 디코드 시간, 에러 수 표시.
- 마지막 레이아웃은 종료 시 저장되어 다음 실행에 복원된다.

### 1.2 비디오 위젯

- 프레임 표시(종횡비 유지, 중앙 정렬), 카메라 이름 오버레이, 선택 테두리.
- 신호 없을 때 "No Signal" 또는 카메라 이름 표시.
- 33ms 간격 갱신(약 30fps 상한).

### 1.3 카메라 트리 (좌측 패널)

- "Default" 그룹 아래 카메라 목록. 온라인/오프라인 아이콘.
- 온라인 판정: 30초마다 카메라 `ip:port`에 TCP 접속 시도(2초 타임아웃).
- 더블클릭: 현재 레이아웃에 맞는 스트림으로 접속하고 그리드의 빈 셀에 배치, Main View 탭으로 전환.
- 우클릭: Start/Stop Stream(실제 스트리밍 여부 기준), Edit(편집 대화상자), Delete.
- 상단 검색 상자는 자리만 있고 동작하지 않는다.

## 2. 장치 관리 (Device Manage 탭)

### 2.1 검색 (Start Search)

세 가지 검색을 5초간 동시에 실행하고 결과를 IP 기준으로 병합한다.

**인터페이스 선택**: 버튼 줄의 "Interface" 콤보에서 프로브를 내보낼 NIC를 고른다. 기본값 "All interfaces"는 사용 가능한 모든 IPv4 인터페이스(Up, 루프백·링크로컬 제외)마다 소켓을 만들어 각각 보낸다. 소켓을 `0.0.0.0`에 바인드하면 OS가 기본 경로 인터페이스(VPN, Hyper‑V 가상 어댑터일 수 있음)로만 멀티캐스트를 내보내 카메라 LAN에 프로브가 도달하지 않기 때문이다. 검색 중 상태 줄에 사용 중인 인터페이스와 결과 수가 표시된다. WS‑Discovery 프로브는 1초 간격으로 2회, SSDP/mDNS 질의는 0.9초마다 재전송한다.

| 방식 | 프로토콜 | 결과 |
|------|----------|------|
| ONVIF WS‑Discovery | UDP 멀티캐스트 239.255.255.250:3702 | 서비스 URL, 이름(scope name), 모델(scope hardware) |
| mDNS | UDP 224.0.0.251:5353, `_rtsp._tcp` / `_axis-video._tcp` / `_http._tcp`. QU 비트로 유니캐스트 응답 요청 + 5353 그룹 수신 | IP만. 이름은 "mDNS Camera (ip)" |
| SSDP/UPnP | UDP 239.255.255.250:1900, `ssdp:all` 등. 공유기(IGD)·미디어 기기는 휴리스틱으로 제외 | IP, LOCATION, SERVER 헤더 |

ONVIF 결과가 우선이며, mDNS/UPnP 결과는 ONVIF 검색이 끝난 뒤 중복이 아닌 것만 표에 추가된다. 이미 추가된 카메라의 IP는 검색 표에 표시하지 않는다.

### 2.2 검색 결과 조작

| 버튼/동작 | 기능 |
|-----------|------|
| Add Device | 사용자명/비밀번호를 입력받아 AddCameraDialog를 자격 증명이 채워진 상태로 열고 접속 테스트를 자동 실행. 프로필과 스트림 URL을 확인한 뒤 저장 |
| 행 더블클릭 | AddCameraDialog를 검색 정보로 채워서 열기. ONVIF면 프로필까지 자동 조회. 정상 경로 |
| Modify IP | ONVIF 장치만. NetworkSettingsDialog 열기 |
| Manual Add | 빈 AddCameraDialog |

### 2.3 추가된 장치 표

이름, IP, 모델, 시리얼, 타입(ONVIF/RTSP), 연결 상태, (파일 시스템 버전: 비어 있음). Delete, Edit(더블클릭과 동일하게 AddCameraDialog 편집 모드).

**연결 상태**는 라이브 뷰와 무관하게 `StreamHealthChecker`가 메인/서브 RTSP URL을 각각 실제로 열어 본 결과다(시작 시, 카메라 추가·수정 시, 이후 60초마다, 끊김 직후). 이미 라이브로 열려 있는 스트림은 다시 열지 않는다.

| 표시 | 뜻 |
|------|----|
| Online | 메인·서브 모두 열림(또는 한쪽은 아직 확인 전) |
| Online (main stream failed) / Online (sub stream failed) | 한쪽만 열림. 툴팁에 실패 사유 |
| Offline | 둘 다 실패 |
| Checking... | 아직 결과 없음 |

라이브 뷰를 닫아도 상태는 바뀌지 않는다. 확인은 접속 + 스트림 정보 읽기까지 하므로, 인증은 통과하지만 패킷이 오지 않는 스트림도 실패로 잡힌다.

## 3. 카메라 추가/편집 (AddCameraDialog)

### 3.1 ONVIF 타입

1. IP, 포트(기본 80), 사용자명, 비밀번호, ONVIF 경로(기본 `/onvif/device_service`) 입력.
2. **Test Connection**: GetServices → 미디어 URL 없으면 GetCapabilities → 그래도 없으면 device_service를 미디어 URL로 사용. 이어서 GetDeviceInformation(모델·시리얼·제조사·펌웨어 자동 채움), GetProfiles.
3. 프로필을 해상도순으로 정렬해 **메인=최고 해상도, 서브=최저 해상도**를 기본 선택. 각 프로필의 GetStreamUri로 RTSP URL 표시. 콤보로 변경 가능.
4. 메인 URL로 우측 미리보기 시작. 연결 로그 표시.
5. OK → 검증(이름, IP) → 저장.

인증: SOAP 본문의 WS‑Security UsernameToken(PasswordDigest). HTTP 401 챌린지가 오면 Basic/Digest로 응답한다. Basic 헤더를 선제적으로 보내지는 않는다.

### 3.2 RTSP 타입

메인/서브 URL, 사용자명, 비밀번호 직접 입력. IP와 포트는 URL에서 추출. Test Connection으로 미리보기.

## 4. ONVIF 네트워크 설정 (NetworkSettingsDialog)

- GetNetworkInterfaces로 인터페이스 목록, MAC, DHCP 여부, 수동 IP/프리픽스 표시.
- DHCP 토글, IP, 서브넷(프리픽스 콤보), 게이트웨이 편집 후 Apply → SetNetworkInterfaces(+1초 후 SetNetworkDefaultGateway).
- RebootNeeded 응답 시 재부팅 여부 확인 → SystemReboot.
- DHCP로 할당된 현재 주소도 표시된다. 게이트웨이는 조회하지 않아 항상 비어 있다.

## 5. 스트리밍

### 5.1 RTSP 수신

- FFmpeg `libavformat`, RTSP over TCP 고정.
- 옵션: `timeout=5s`(소켓 I/O), `max_delay=100ms`, `buffer_size=1MB`, `fflags=nobuffer`. 디먹서가 소비하지 않은 옵션은 경고 로그로 드러난다.
- 접속·읽기 중 중단은 FFmpeg interrupt callback으로 즉시 빠져나온다. 스트림이 끊기면 그 스트림을 실패로 표시하고 상태바에 알린 뒤 헬스 체크를 다시 돌린다. 자동 재접속은 없다.
- 열기 실패 사유(`error` 시그널)는 `open()` 전에 연결되므로 로그와 상태바에 남는다.
- 인증은 URL에 `user:pass@` 형태로 삽입.

### 5.2 디코딩

- `libavcodec` 소프트웨어 디코드. 코덱은 스트림에서 자동 감지(H.264, H.265, MJPEG 등 FFmpeg가 지원하는 것 전부).
- 디코더 스레드: 720p 초과 4개, 이하 2개 (FRAME|SLICE), `LOW_DELAY`, `FLAG2_FAST`.
- YUV → RGB32는 `sws_scale`(BILINEAR), 원본 해상도 그대로 `QImage`로 복사. 스트림 도중 해상도나 픽셀 포맷이 바뀌면 스케일러를 재생성한다.

### 5.3 통계

수신/디코드/에러 프레임 수, FPS, 평균 디코드 시간, 수신 바이트. 30프레임마다 갱신. 에러율 5% 초과 시 경고 로그.

## 6. 재생 (Playback 탭)

- "Open File..."로 mp4/mkv/avi/ts 파일을 열어 재생. 재생/일시정지/정지, 0.25×~4× 속도, 슬라이더와 타임라인 클릭으로 탐색.
- 파일을 연 직후 자동 재생.
- 녹화 목록이나 카메라·시각 기반 재생은 없다.

## 7. 설정과 영속화

| 항목 | 저장 위치 |
|------|-----------|
| 카메라 정보 | `%APPDATA%/VMS/VMS/vms.db` (SQLite) |
| 윈도우 위치·크기, 마지막 레이아웃 | 레지스트리 `HKCU\Software\VMS\VMS` |
| 로그 | `%APPDATA%/VMS/VMS/vms_debug.log` |

다크 테마(Fusion 스타일 + 커스텀 팔레트)가 항상 적용된다.

## 8. 단축키

| 키 | 기능 |
|----|------|
| `F2` | 라이브 그리드 통계 오버레이 토글 |

이것이 전부다. 메뉴바는 숨겨져 있다.

## 9. 미구현 / 플레이스홀더

| 항목 | 상태 |
|------|------|
| 녹화 | `Recorder` 클래스는 있으나 어디서도 호출되지 않음. `recordings` 테이블도 미사용 |
| 녹화 스케줄 (Record Schedule 탭) | 안내 문구만 있는 빈 페이지 |
| 시스템 설정 (Sys. Settings 탭) | 안내 문구만 있는 빈 페이지 |
| PTZ 제어 | `OnvifClient::ptzMove`와 `PtzControl` 위젯은 있으나 UI에 배치되지 않음 |
| 카메라 그룹, 드래그 앤 드롭 배치 | 트리는 DragOnly로 설정만 되어 있고 드롭 처리 없음 |
| 카메라 트리 검색 상자 | 동작 없음 |
| 자동 재접속 | 없음. 끊김은 감지해 해당 스트림을 실패로 표시하고 상태바에 알린다 |
| 오디오 | 없음 |
| 하드웨어 가속 디코딩 | 없음 |
