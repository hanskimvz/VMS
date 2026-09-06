# 알려진 이슈

마지막 갱신: 2026‑09‑06. 리뷰(2026‑09‑06)에서 나온 결함 중 **수정된 것은 아래 "수정 완료"로 옮기고, 남은 것만 표에 둔다.**

## 남은 이슈

### 설계 재고

| # | 이슈 | 위치 | 설명 |
|---|------|------|------|
| R1 | `StreamReceiver::open()`이 호출 스레드에서 블로킹 | `stream_receiver.cpp` `open()`, `camera_manager.cpp` `startStream()` | 타임아웃(5초)과 interrupt callback 으로 상한은 생겼지만 여전히 GUI 스레드에서 접속한다. 레이아웃 전환 시 카메라 수 × 최대 5초 프리즈. "상시 채널 세션" 작업에서 워커로 옮긴다 |
| R2 | 레이아웃 전환마다 전 채널 RTSP 재접속 | `main_window.cpp` `updateStreamsForLayout()` | 메인↔서브 전환을 위해 모든 세션을 끊고 다시 연다. 그리드 슬롯은 유지되도록 고쳤지만 재접속 자체는 남아 있다. 카메라 세션 한도(503) 유발 가능 |
| R3 | 자동 재접속 없음 | `camera_manager.cpp` | 끊김은 감지해 해당 스트림을 Failed 로 표시하고 상태바에 알리지만 라이브 뷰는 다시 붙지 않는다 (헬스 체크만 재실행) |
| R4 | 스케일링이 GUI 스레드 | `video_widget.cpp` `updateDisplay()` | 위젯당 33ms마다 `SmoothTransformation`. 16채널에서 UI 반응성 저하. 리시버 스레드에서 목표 크기로 `sws_scale` 하거나 GPU 렌더링으로 |
| R5 | DB 비밀번호 평문 | `database.cpp` | 개인용이면 감수 가능. 제품화 시 DPAPI 등 검토 |
| R6 | 게이트웨이 미조회 | `onvif_client.cpp`, `network_settings_dialog.cpp` | `GetNetworkDefaultGateway` 응답을 처리하지 않아 게이트웨이 칸이 항상 빈다 (DHCP 주소 표시는 고침) |
| R7 | Media2 전용 카메라 | `onvif_client.cpp` `parseServicesResponse()` | ver10 media 가 없으면 Media2 URL 로 폴백하지만 ver10 `GetProfiles` 가 실패할 수 있다. Media2 `GetProfiles` 구현 필요 |
| R8 | Basic 인증 선제 전송 제거는 동작 변경 | `onvif_client.cpp` | WS‑Security 다이제스트와 401 챌린지 응답만 남겼다. 401 을 보내지 않고 Basic 만 받는 카메라가 있다면 실카메라로 확인 필요 |
| R9 | 카메라 트리의 온라인 아이콘은 "도달 가능" 뜻 | `camera_tree.cpp` | 스트리밍 여부는 우클릭 메뉴(Start/Stop)에만 반영. 두 상태를 UI 에서 구분해 보여주는 것은 미구현 |

### 컴파일되지만 연결되지 않은 코드

| 클래스/함수 | 상태 |
|-------------|------|
| `Recorder` | 어디서도 인스턴스화되지 않음. `writePacket` 구조는 remux 녹화에 재사용 가능 |
| `VideoDecoder` | 미사용. `StreamReceiver`가 자체 디코드 |
| `PtzControl` | 미사용. `MainWindow`에 배치되지 않음 |
| `Database::saveRecording` 등 recordings 관련 | 호출 없음 |
| `MainWindow::onAddCamera`, `onDiscoverCameras` | 메뉴바가 숨겨져 호출 경로 없음 |

## 수정 완료 (2026‑09‑06)

| 리뷰 # | 이슈 | 수정 내용 |
|--------|------|-----------|
| 1 | 스트림 중지 후 dangling 포인터 (UAF) | `VideoWidget`, `VideoGrid::StreamInfo` 가 `QPointer<StreamReceiver>` 사용. `CameraManager::stopStream` 은 `deleteLater`. `MainWindow` 가 `streamStopped` 를 받아 그리드 슬롯을 비움(메인/서브 전환 중엔 유지) |
| 2 | `terminate()` + 뮤텍스 데드락 | `AVFormatContext::interrupt_callback` 도입. `stop()` 은 플래그를 세우고 `wait()` 만 한다. 기존 `stop()` 은 `isRunning()` 을 자체 멤버로 가려서 `wait()` 조차 실행되지 않던 문제도 함께 해결 |
| 3 | RTSP 타임아웃 옵션명 (`stimeout`) | `timeout` 으로 변경. 디먹서가 소비하지 않은 옵션은 경고 로그 |
| 4 | "Add Device" 로 추가한 카메라 재생 불가 | 자격 증명을 채운 `AddCameraDialog` 를 열고 접속 테스트를 자동 실행. `onvifPath` 에 전체 URL 이 들어가던 문제 해소 |
| 5 | ONVIF 서비스 네임스페이스 느슨한 매칭 | 정확한 네임스페이스 비교. Media2 는 폴백으로만 사용 |
| 6 | Basic 인증 평문 선제 전송 | 제거 (R8 참고) |
| 8 | "Online" 의미 충돌 | 우클릭 메뉴는 실제 스트리밍 여부로 결정. 트리의 Start Stream 은 `MainWindow` 를 거쳐 그리드에 배치. 핑 결과를 `refreshCameras` 가 덮어쓰지 않음 |
| 9 | 에러/끊김 미처리 | `CameraManager` 가 `disconnected` 를 받아 `stopStream` → Offline + 그리드 정리. `error` 는 `streamError` 로 상태바에 표시. EOF 도 끊김으로 처리(무한 루프 제거) |
| 10 | WS‑Discovery 중복 전송 | `DeviceDiscovery` 에서 제거. `OnvifClient` 만 담당 |
| 11 | 디코더 스레드 과다 | 720p 초과 4개, 이하 2개 |
| 13 | 로그 핸들러 비스레드안전, 로그 위치 | 뮤텍스 추가. `%APPDATA%/VMS/VMS/vms_debug.log` 로 이동 |
| 15 | `stream->duration == AV_NOPTS_VALUE` | 컨테이너 duration 으로 폴백. pts 는 `best_effort_timestamp` 우선 |
| 16 | 네트워크 설정 변경 시 대화상자 중복 | 게이트웨이 응답까지 기다렸다가 한 번만 안내 |
| 17 | DHCP 주소 미표시 | `FromDHCP` 파싱 |
| 18 | 재시도 판단이 문자열 매칭 | `HttpStatusCodeAttribute` 사용 |
| 19 | GUI 스레드 `wait(1000)`, `msleep(100)` | 제거 |
| 20 | 프레임 스레딩 + `receive_frame` 1회 | `EAGAIN` 까지 반복. 초기 EAGAIN 을 오류로 세지 않음 |
| 21 | `CameraTree::onEditCamera` 빈 함수 | `cameraEditRequested` → `MainWindow` 가 편집 대화상자 열기 |
| 22 | 경고 플래그 없음 | `-Wall -Wextra` 추가, 경고 0 |
| 23 | 미사용 링크 | `Qt6::OpenGLWidgets`, `Qt6::Concurrent` 제거 |
| (신규) | 스트림 도중 해상도/픽셀 포맷 변경 시 버퍼 오버런 | 프레임마다 스케일러 입력 형식을 확인하고 바뀌면 재생성 |
| (신규) | `VideoGrid::addStream` 이 꽉 찬 슬롯을 빼앗을 때 이전 매핑이 남음 | 빼앗긴 스트림을 목록에서 제거 |
| (신규) | 카메라 편집 후 트리/장치표 미갱신 | `cameraUpdated` 연결 |
| (신규) | **장치 검색이 아무것도 못 찾음** | 원인: 소켓을 `0.0.0.0`에 바인드해 멀티캐스트를 보내면 OS가 기본 경로 인터페이스로만 내보내는데, 개발 PC는 VPN 어댑터(172.17.x)가 기본 경로라 카메라 LAN(192.168.1.x)에 프로브가 안 나감. 수정: `local_interfaces` 도입, 인터페이스마다 소켓 바인드 + `setMulticastInterface`, 프로브 재전송, Device Manage 탭에 NIC 선택 콤보. 검증: LAN NIC 로 ONVIF 카메라 5대 발견, VPN NIC 로는 0대 |
| (신규) | 공유기가 UPnP 카메라로 오탐 | SSDP 제외 규칙에 `igd.xml`, `vxWorks` 추가 |
