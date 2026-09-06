# ONVIF (`onvif_service`)

ONVIF Device / Media SOAP 스택과 System > API 설정 연동.

> **상태:** 프로세스·Discovery·일부 GET 은 동작.  
> **Phase 1 완료:** Enable 토글이 `onvif_service` start/stop 에 연동됨.  
> **Phase 2 완료:** 프로파일/RTSP URI/스냅샷이 `encode.json`·`rtsp.json`·`video.snapshot` RPC 에 연동됨.  
> **Phase 3 완료:** AuthEnable → WS-Security UsernameToken 검증, `user.json` 계정 공유, Onvif 계정 잠금.  
> **Phase 4 완료:** ReplayAttackProtection → `wsu:Created` ±60초 + Nonce TTL 캐시.  
> WS-Discovery `hardware`/`name` scope 와 `GetDeviceInformation` 은 sysinfo / U-Boot (`model`, `usn`) 를 사용한다.  
> PTZ / Events PullPoint 등은 **미구현 (Phase 6+)**.  
> 브라우저로 웹 포트(80)의 `/onvif/device_service` 를 열면 Vue SPA 404 가 난다. SOAP 은 **TCP 8089**.  
> **⚠ 2026-09-06 VMS 연동 차단 이슈:** 8089 로 오는 **모든 SOAP POST 가 응답 없이 TCP 종료**된다 (NS6104HD-6211). 재현·추정 원인·요청 사항은 [§8](#8-vms-연동-이슈-2026-09-06--펌웨어-수정-요청).

관련:

- 프로세스 분리: [`known_issue_app_seperate_solved.md`](known_issue_app_seperate_solved.md)
- RTSP URL: [`video_core.md`](video_core.md) (`rtsp://<ip>:554/video0`)
- 웹 인증 vs ONVIF Digest: [`web_auth.md`](web_auth.md) — PBKDF2(웹) vs PasswordDigest(ONVIF)
- Network Service VSP_ONVIF: [`network.md`](network.md), [`../web/src/views/network/network_todo.md`](../web/src/views/network/network_todo.md)
- 계정 잠금 Onvif 섹션: [`todo_securiy_attack_defence.md`](todo_securiy_attack_defence.md)

---

## 1. 아키텍처 (현재 설계)

```
VMS / ONVIF Device Manager
        │  WS-Discovery UDP 3702
        │  SOAP HTTP  TCP 8089  /onvif/device_service
        ▼
  onvif_service          (gSOAP, 별도 프로세스)
        │  Unix RPC  /tmp/video_core.sock  (video.snapshot)
        │  encode.json / rtsp.json / user.json / api.json (로컬 읽기)
        ▼
  video_core             RTSP :554  /video0, /video1

http_service :80         REST + Vue SPA. ONVIF 를 프록시하지 않음.
start_app.sh             api.json ONVIF.Enable=true 일 때만 onvif_service 기동.
```

프로세스를 `video_core` / `http_service` 와 분리한 이유: WS-Discovery 폭주나 gSOAP 크래시가 영상 파이프라인을 죽이지 않게 하기 위함.

| 역할 | 프로세스 | 포트 |
|------|----------|------|
| 웹 UI / REST | `http_service` | `port.json` Web (기본 80) |
| ONVIF SOAP | `onvif_service` | **8089** (`ONVIF_SERVICE_PORT`) |
| ONVIF Discovery | `onvif_service` | **UDP 3702** (`ONVIF_LISTEN_PORT`) |
| RTSP | `video_core` | `port.json` RTSP (기본 554) |

SOAP XAddr 예:

```
http://<eth0-ip>:8089/onvif/device_service
http://<eth0-ip>:8089/onvif/media
```

Init 시 `route add -net 239.255.255.250 ... eth0` (WS-Discovery 멀티캐스트). root/`CAP_NET_ADMIN` 필요.

---

## 2. 소스 / 바이너리

빌드: `sdk/app/src/app/onvif/onvif.mk` → `out/$(ARCH)/app/onvif_service`  
보드: `/data/app/onvif_service` (`start_app.sh` 가 기동)

| 파일 | 역할 |
|------|------|
| `onvif/standalone/onvif_main.cpp` | `main()`, SIGINT/TERM → `OnvifService_Stop()` |
| `onvif/onvif_service.cpp` | Init/Stop 래퍼. Discovery + SOAP 스레드 |
| `onvif/onvif_server.cpp` | `MST_ONVIF_*`: UDP 3702 + TCP 8089 gSOAP 루프 |
| `onvif/onvif_server_interface.cpp` | SOAP 핸들러 (구현 + stub) |
| `onvif/onvif_media.cpp` | encode/rtsp → 프로파일·URI·스냅샷 |
| `onvif/onvif_auth.cpp` | WS-Security UsernameToken + Replay(Phase 4) 게이트 |
| `onvif/onvif_function.cpp` | MAC / IPv4 / gateway / 시각 (`eth0`) |
| `onvif/onvif_function.h` | `ONVIF_LISTEN_PORT 3702`, `ONVIF_SERVICE_PORT 8089` |
| `security/stream_auth_secret.cpp` | ONVIF/RTSP용 `userStreamAuthSecret` (AES-256-GCM) |
| `security/auth.cpp` | `Auth_AttemptOnvifDigest`, Onvif 계정 잠금, `GetUsers` |
| `core/api_config.cpp` | `ST_ApiConfig_OnvifAuthEnabled`, `…ReplayProtectionEnabled` |
| `soapC.cpp` / `soapServer.cpp` / `stdsoap2.cpp` / `wsdd.*` | gSOAP 생성물. **수동 수정 금지** |

원본: Maruko V1.2.0 `sdk/verify/mi_demo/common/onvif` (C → `.cpp` 포트, `-fpermissive`).

`onvif_service` 는 **MI 비디오 API 를 직접 링크하지 않는다.** 스냅샷은 Unix RPC(`video.snapshot`) 로 `video_core` 에 요청한다.

---

## 3. System > API 설정

페이지: [`SystemAPIPage.vue`](../web/src/views/system/SystemAPIPage.vue)  
REST: `GET/PUT /api/v1/system/api` (`api_system_api.cpp` + `api_config.cpp`)  
디스크: `/data/config/system/api.json`

```json
{
  "Api": {
    "Genetec": { "Enable": true },
    "ONVIF": {
      "Enable": true,
      "AuthEnable": true,
      "ReplayAttackProtection": false
    },
    "UdpVcaTech": { "Enable": false }
  }
}
```

REST `PUT` body 는 flat `{ "onvif": { ... } }` 형태도 허용 (`api_config.cpp` `MergePatch`).

Apply 시 `network_service.json` 으로 브리지 (파일이 없어질 때까지):

| `api.json` | `network_service.json` |
|------------|------------------------|
| `Api.ONVIF.Enable` | `VSP_ONVIF.ServiceStart` |
| `Api.ONVIF.AuthEnable` | `streamAuthority.OnvifLoginCheck` = `Digest` / `None` |

Enable 변경 시 `killall onvif_service` → supervisor 가 Enable=true 일 때만 재기동 (`start_app.sh` `manage_onvif_service`).

웹 UI (현재):

- Enable ONVIF: 토글 가능, 저장 시 프로세스 start/stop 반영
- URL: `http://<ip>:8089/onvif/device_service` + SOAP/브라우저 안내
- 인증: AuthEnable 토글 → WS-Security UsernameToken (PasswordDigest). `user.json` 계정 공유
- Replay: ReplayAttackProtection 토글 → Created ±60초 + Nonce 재사용 거부 (AuthEnable 필요)
- Genetec / UDP(VCA Tech): 이 문서 범위 밖. Genetec 은 서비스 없음

---

## 4. SOAP 구현 현황

`onvif_server_interface.cpp` 기준. stub 은 `ONVIF_NOTSUPPORTED_FUNC` → SOAP Fault `ter:ActionNotSupported`.

### 4.1 응답하는 연산

**Discovery**

| 연산 | 내용 |
|------|------|
| `__wsdd__Probe` | ProbeMatches. XAddr = `http://<ip>:8089/onvif/device_service`. `hardware`/`name` scope 는 model / friendlyName |

**Device (`tds`)**

| 연산 | 내용 |
|------|------|
| `GetServices` | Device + Media XAddr (Events/PTZ 는 주석) |
| `GetCapabilities` | Device + Media. RTP/TCP, RTSP/TCP = true, Multicast = false |
| `GetDeviceInformation` | Manufacturer / Model / FirmwareVersion / SerialNumber(USN) — sysinfo + U-Boot |
| `GetSystemDateAndTime` | `mst_GetSysDataTime()` — 실제 시각 |
| `GetScopes` | type 고정 + hardware/name 은 Probe 와 동일 |
| `GetUsers` | `user.json` 활성 사용자 목록 (`Auth_OnvifGetUser`) |
| `GetHostname` / `GetDNS` / `GetNTP` | 로컬/고정값 |
| `GetNetworkInterfaces` | `eth0` MAC/IPv4 |
| `GetNetworkDefaultGateway` | 실제 게이트웨이 |
| `GetZeroConfiguration` | 고정 응답 |

**Media (`trt`)**

| 연산 | 내용 |
|------|------|
| `GetProfile` / `GetProfiles` | `encode.json` + `rtsp.json` 기반 (main/sub, enable 시 최대 2) |
| `GetVideoSourceConfiguration` | 센서/프로파일 bounds |
| `GetVideoEncoderConfiguration` | H.264/H.265→H.264 보고, encode 설정 반영 |
| `GetStreamUri` | `rtsp://<ip>:<RTSP.Port>/<StreamName>` 예: `/video0` |
| `GetSnapshotUri` | `http://<ip>:8089/onvif-http/snapshot` → `video.snapshot` RPC JPEG |
| `GetVideoSources` (`trt`) | `VideoSource_1` 토큰, 센서 크기 |

**DeviceIO (`tmd`)**

| 연산 | 내용 |
|------|------|
| `GetVideoSources` | 소스 1개 (`tmd`, `trt` 동일 토큰) |

**Imaging (`timg`)**

| 연산 | 내용 |
|------|------|
| `GetImagingSettings` | Brightness/Contrast 등 로컬 상수 |
| `SetImagingSettings` | `#if 0` 본문. SOAP_OK 만 반환, ISP 미적용 |

### 4.2 의도적으로 stub 인 것

PTZ, Events/PullPoint, Recording, Replay, AnalyticsDevice, Door, AccessControl, OSD Set, User Create/Delete, SetNetwork\*, Firmware upgrade, Certificates, Relay.

하드웨어/기능이 없으면 stub 유지가 맞다.

---

## 5. 알려진 제한 / 미구현

| 항목 | 상태 | 비고 |
|------|------|------|
| 웹 포트(80) `/onvif/*` | 정상 404 | SOAP 은 **8089** 직접 접속. Vue SPA 가 프록시하지 않음 |
| H.265 인코딩 | ONVIF enum H.264 로 보고 | VMS 호환용 |
| PTZ / Events / Recording | stub | GetCapabilities 에 광고하지 않음 (Phase 6) |
| Imaging Get/Set | 구현 (Phase 5) | `visp.get_color` / `visp.apply_color` RPC |
| `SetSystemDateAndTime` / `SystemReboot` / `GetNetworkProtocols` | 구현 (Phase 5) | datetime_config, 2s reboot, port.json |
| SetVideoEncoderConfiguration | stub | `encode.apply_config` RPC 미연동 |
| 스냅샷 HTTP GET | SOAP 인증 미적용 | WS-Security 는 SOAP 경로만. JPEG URL 은 별도 |
| 업그레이드 직후 ONVIF auth | secret 없으면 실패 | 웹에서 비밀번호 1회 변경 필요 (Phase 3) |
| Replay + 시각 오차 | Created ±60s | NTP 미동기화 시 거부될 수 있음 (Phase 4) |

공통 확인:

```bash
pidof onvif_service
netstat -lntu | grep -E '3702|8089'

# Device 정보 (AuthEnable=false 일 때 토큰 없이 가능)
curl -s -X POST "http://<ip>:8089/onvif/device_service" \
  -H 'Content-Type: application/soap+xml' \
  -d '<?xml version="1.0"?><s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:tds="http://www.onvif.org/ver10/device/wsdl"><s:Body><tds:GetDeviceInformation/></s:Body></s:Envelope>'
```

gSOAP 생성 파일(`soapC.cpp` 등)은 재생성 시 사라지므로 손대지 않는다.

---

## 6. Phase별 구현 / 테스트

VMS 연동 체감 순서. 아래 **테스트** 는 보드(또는 WSL+크로스 빌드 바이너리)에서 `onvif_service`·`video_core`·`http_service` 가 기동된 상태를 가정한다.

### Phase 1 — Enable → 프로세스 ✅

#### 구현

| 구성요소 | 동작 |
|----------|------|
| `board/data/start_app.sh` `onvif_service_enabled()` | `/data/config/system/api.json` `Api.ONVIF.Enable` 읽기. 없으면 `network_service.json` `VSP_ONVIF.ServiceStart` fallback |
| `manage_onvif_service()` | Enable=false → `killall onvif_service`. Enable=true → `start_if_missing` |
| `api_config.cpp` + `api_system_api.cpp` | `PUT /api/v1/system/api` 에서 Enable 변경 시 `onvifEnableChanged` → `killall onvif_service` |
| `network_service_config.cpp` | `VSP_ONVIF.ServiceStart` 변경 시 동일 kill |
| `SystemAPIPage.vue` | Enable ONVIF 토글, 저장 시 REST apply |

#### 테스트

1. **프로세스 기동**
   - 웹 **System > API** → Enable ONVIF **ON** → Apply
   - `pidof onvif_service` → PID 출력
   - `netstat -lntu | grep 8089` → LISTEN
   - `netstat -lnu | grep 3702` → UDP 3702

2. **프로세스 중지**
   - Enable ONVIF **OFF** → Apply (또는 `PUT /api/v1/system/api`)
   - `pidof onvif_service` → 출력 없음
   - supervisor(`start_app.sh` 루프) 재실행 후에도 Enable=false 이면 기동하지 않음

3. **REST apply 연동**
   ```bash
   # Enable 끄기 (세션 쿠키/인증은 환경에 맞게)
   curl -s -X PUT "http://<ip>/api/v1/system/api" \
     -H 'Content-Type: application/json' \
     -d '{"onvif":{"Enable":false,"AuthEnable":true,"ReplayAttackProtection":false}}'
   sleep 2 && pidof onvif_service || echo "stopped OK"
   ```

4. **Discovery (선택)**
   - ONVIF Device Manager / `gsoap` Probe 로 장치 검색
   - XAddr = `http://<ip>:8089/onvif/device_service`

---

### Phase 2 — 미디어 연동 (VMS 핵심) ✅

#### 구현

| 구성요소 | 동작 |
|----------|------|
| `onvif_media.cpp` | `encode.json` + `rtsp.json` 경량 파싱. 프로파일 `Profile_1`(video0/main), `Profile_2`(video1/sub, enable 시) |
| `GetStreamUri` | `rtsp://<eth0-ip>:<RTSP.Port>/<StreamName>` |
| `GetSnapshotUri` | `http://<ip>:8089/onvif-http/snapshot` |
| `mst_OnvifTryServeSnapshotHttp()` | HTTP GET 스냅샷 → `video.snapshot` RPC → JPEG 응답 |
| `onvif.mk` | `rpc_client.cpp` / `rpc_proto.cpp` 링크 |
| `GetCapabilities` | Media XAddr `strcat` 대상 버그 수정 (Device → Media) |
| Discovery / GetScopes | `sysinfo.json` manufacturer·model·friendlyName scope |

#### 테스트

1. **프로파일 / RTSP URI**
   ```bash
   # GetProfiles (AuthEnable=false 가정)
   curl -s -X POST "http://<ip>:8089/onvif/media" \
     -H 'Content-Type: application/soap+xml' \
     -d '<?xml version="1.0"?><s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:trt="http://www.onvif.org/ver10/media/wsdl"><s:Body><trt:GetProfiles/></s:Body></s:Envelope>' \
     | grep -E 'Profile_|video0|video1'
   ```
   - ONVIF Device Manager → Live video → Stream URI 가 `rtsp://<ip>:554/video0` (또는 `rtsp.json` StreamName) 인지 확인
   - `encode.json` 에서 sub stream enable 변경 후 `GetProfiles` 에 `Profile_2` 반영 확인

2. **RTSP 재생**
   - VMS 또는 `ffplay rtsp://<ip>:554/video0` 로 실제 영상 확인
   - Phase 3 AuthEnable ON 이후에도 RTSP 자체는 별도 (RTSP auth 미연동 — [`video_core.md`](video_core.md))

3. **스냅샷**
   ```bash
   curl -s -o /tmp/snap.jpg "http://<ip>:8089/onvif-http/snapshot"
   file /tmp/snap.jpg   # JPEG image data
   ```
   - `video_core` 미기동 시 RPC 실패 → 빈/오류 응답

4. **GetDeviceInformation**
   - `sysinfo.json` 의 Manufacturer/Model/Firmware 와 SOAP 응답 일치 확인

---

### Phase 3 — 인증 (AuthEnable) ✅

#### 구현

| 구성요소 | 동작 |
|----------|------|
| `onvif_auth.cpp` `mst_OnvifServeWithAuth()` | `soap_serve` 대신 인증 게이트 후 `soap_serve_request` |
| WS-Security | SOAP Header 에서 `Username` / `Password`(Digest) / `Nonce` / `Created` XML 파싱 |
| PasswordDigest | `Base64(SHA1(nonce + created + password))` — [`auth.cpp`](../src/app/security/auth.cpp) `Auth_AttemptOnvifDigest` |
| `userStreamAuthSecret` | 비밀번호 변경 시 AES-256-GCM(eth0 MAC 키) 로 디스크 저장. PBKDF2 해시와 별도 |
| `AuthEnable=false` | 토큰 없이 모든 SOAP 통과 |
| `GetUsers` | `user.json` 활성 사용자 (`Auth_OnvifUserCount` / `Auth_OnvifGetUser`) |
| `account_lock.json` `Onvif` | `Auth_AttemptOnvifDigest` 실패 시 Onvif 정책 잠금 |
| `onvif_service` Init | `ST_ApiConfig_Init`, `ST_AccountLockConfig_Init`, `Auth_Init` |
| SIGHUP | `ST_ApiConfig_Reload` + `Auth_Reload` (비밀번호 변경 후 재로드) |
| `SystemAPIPage.vue` | AuthEnable 토글 활성 |

#### 테스트

**사전:** 웹 **System > Users** 에서 대상 사용자 비밀번호를 **한 번 변경** (`userStreamAuthSecret` 생성). 업그레이드 직후 secret 없으면 digest 실패.

1. **AuthEnable OFF — 익명 SOAP**
   - System > API → AuthEnable **OFF** → Apply
   - 위 Phase 1 `curl GetDeviceInformation` (토큰 없음) → `SOAP_OK` / Device 정보 XML

2. **AuthEnable ON — 토큰 없으면 거부**
   - AuthEnable **ON** → Apply (`killall onvif_service` 후 재기동 또는 SIGHUP)
   - 동일 curl (토큰 없음) → Fault `ter:NotAuthorized`

3. **AuthEnable ON — VMS 로그인**
   - ONVIF Device Manager (또는 Frigate ONVIF 등)
   - 사용자 = `user.json` `userName` (예: `root`), 비밀번호 = 웹과 동일
   - Live view / 프로파일 조회 성공

4. **GetUsers**
   ```bash
   # AuthEnable=false 일 때
   curl -s -X POST "http://<ip>:8089/onvif/device_service" \
     -H 'Content-Type: application/soap+xml' \
     -d '<?xml version="1.0"?><s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:tds="http://www.onvif.org/ver10/device/wsdl"><s:Body><tds:GetUsers/></s:Body></s:Envelope>' \
     | grep -E '<tds:Username>|Administrator|User'
   ```
   - `user.json` 에 등록된 활성 사용자명과 일치

5. **계정 잠금 (Onvif)**
   - **Security > Account Lock** 에서 Onvif 정책 enable, 임계값 설정 (예: 3회 / 300초)
   - AuthEnable ON 상태에서 VMS/클라이언트로 **의도적 오류 비밀번호** 연속 입력
   - 잠금 시간 동안 올바른 비밀번호도 `NotAuthorized`
   - 잠금 해제 후 정상 로그인

6. **설정 핫 리로드**
   ```bash
   kill -HUP $(pidof onvif_service)
   ```
   - 웹에서 비밀번호 변경 후 HUP → 새 secret 반영 (재기동 없이)

---

### Phase 4 — Replay attack protection ✅

#### 구현

| 구성요소 | 동작 |
|----------|------|
| `ST_ApiConfig_OnvifReplayProtectionEnabled()` | `api.json` `ReplayAttackProtection` |
| `CheckReplayProtection()` | AuthEnable **ON** + PasswordDigest 일 때만 검사 |
| `wsu:Created` | UTC ISO8601 파싱, `timegm`, 현재 시각과 **±60초** |
| Nonce 캐시 | 최대 64 entries, TTL **120초**, 재사용 시 거부 |
| Replay OFF | Created/Nonce 검사 생략 (인증만) |
| `SystemAPIPage.vue` | Replay 토글 활성. **AuthEnable OFF** 이면 UI disabled |

#### 테스트

**사전:** AuthEnable **ON**, Replay **ON**, NTP/시각 대략 정확 (Phase 4 Created 검증).

1. **Replay OFF — Nonce 재사용 허용**
   - Replay **OFF** → Apply
   - VMS 정상 연결 유지 (동일 클라이언트 재접속 문제 없음)

2. **Replay ON — 정상 요청**
   - AuthEnable ON + Replay **ON** → Apply
   - ONVIF Device Manager 로 최초 연결 성공

3. **Replay ON — Nonce 재사용 (캡처 재전송)**
   - Wireshark/tcpdump 로 **성공한** PasswordDigest SOAP 요청 1건 저장
   - 동일 바이트를 `curl --data-binary @capture.xml` 등으로 **재전송**
   - 기대: `ter:NotAuthorized`, 로그 `[onvif] replay: reused Nonce`

4. **Replay ON — Created 만료**
   - 캡처한 요청의 `wsu:Created` 를 **2분 이상 과거** 로 수정 후 전송
   - 기대: `NotAuthorized`, 로그 `Created outside +/-60s window`

5. **Replay ON — AuthEnable OFF**
   - AuthEnable **OFF** (Replay ON 유지 가능) → Apply
   - Replay 검사는 수행되지 않음 (Phase 4는 AuthEnable 에 종속)

6. **UI 연동**
   - AuthEnable OFF → Replay 토글 **disabled**
   - AuthEnable ON → Replay 토글 변경·저장 가능

---

### Phase 5 — Profile S 최소 세트 (완료, Events PullPoint 제외)

일반 VMS (ONVIF Device Manager, Frigate ONVIF 등) 가 추가로 부르는 연산.

- [x] `SetSystemDateAndTime` → `datetime_config` / `ST_WallClock_Apply` (NTP 모드는 `mode=ntp` persist)
- [x] `SystemReboot` → 2초 후 `reboot` (REST `POST /system/reboot` 와 동일)
- [x] `GetNetworkProtocols` → `port.json` HTTP/RTSP + ONVIF 8089 (WSDL 미포함 → 수동 SOAP)
- [x] Imaging Get/Set → `visp.get_color` / `visp.apply_color` RPC
- [ ] Events PullPoint — **의도적 미구현**. GetServices 에 Events 를 광고하기 **전에** 구현

**구현 파일:** `onvif_device.cpp`, `onvif_device.h`

**테스트:**

1. **SetSystemDateAndTime (manual)**
   ```bash
   curl -s -X POST "http://<ip>:8089/onvif/device_service" \
     -H 'Content-Type: application/soap+xml' \
     -d '<?xml version="1.0"?><soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope"
       xmlns:tds="http://www.onvif.org/ver10/device/wsdl" xmlns:tt="http://www.onvif.org/ver10/schema">
       <soap:Body><tds:SetSystemDateAndTime>
         <tt:DateTimeType>Manual</tt:DateTimeType><tt:DaylightSavings>false</tt:DaylightSavings>
         <tt:TimeZone><tt:TZ>KST-9</tt:TZ></tt:TimeZone>
         <tt:UTCDateTime><tt:Time><tt:Hour>6</tt:Hour><tt:Minute>0</tt:Minute><tt:Second>0</tt:Second></tt:Time>
         <tt:Date><tt:Year>2026</tt:Year><tt:Month>9</tt:Month><tt:Day>3</tt:Day></tt:Date></tt:UTCDateTime>
       </tds:SetSystemDateAndTime></soap:Body></soap:Envelope>'
   ```
   이후 `date -u` 또는 REST `GET /api/v1/system/time` 로 시각 확인.

2. **GetNetworkProtocols**
   ```bash
   curl -s -X POST "http://<ip>:8089/onvif/device_service" \
     -H 'Content-Type: application/soap+xml' \
     -d '<?xml version="1.0"?><soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope"
       xmlns:tds="http://www.onvif.org/ver10/device/wsdl"><soap:Body>
       <tds:GetNetworkProtocols/></soap:Body></soap:Envelope>'
   ```
   HTTP/RTSP 포트가 `port.json` 과 일치하는지 확인.

3. **Imaging** — ONVIF Device Manager Imaging 탭 또는 `GetImagingSettings` / `SetImagingSettings` SOAP. 웹 UI 색상 설정과 동기화되는지 확인.

4. **SystemReboot** — 장비 재부팅 테스트는 **주의**. VMS 또는 SOAP `SystemReboot` 호출 시 2초 후 재부팅.

### Phase 6 — 하지 않음 (현재 제품)

PTZ, Door, AccessControl, Recording/Replay, AnalyticsDevice, ONVIF OSD 편집, 펌웨어 SOAP 업그레이드.  
GetCapabilities / GetServices 에 광고하지 말 것.

---

## 7. 파일 레퍼런스

| 역할 | 경로 |
|------|------|
| SOAP 서버 | `sdk/app/src/app/onvif/` |
| ONVIF 인증 / Replay | `sdk/app/src/app/onvif/onvif_auth.cpp` |
| ONVIF 미디어 | `sdk/app/src/app/onvif/onvif_media.cpp` |
| ONVIF 디바이스 (Phase 5) | `sdk/app/src/app/onvif/onvif_device.cpp` |
| Stream auth secret | `sdk/app/src/app/security/stream_auth_secret.cpp` |
| 사용자 / ONVIF digest | `sdk/app/src/app/security/auth.cpp` |
| REST API 설정 | `sdk/app/src/app/http/api/api_system_api.cpp` |
| api.json | `sdk/app/src/app/core/api_config.cpp`, `/data/config/system/api.json` |
| user.json | `/data/config/system/user.json` |
| account_lock | `/data/config/security/account_lock.json` |
| 웹 UI | `sdk/web/src/views/system/SystemAPIPage.vue` |
| 기동 | `board/data/start_app.sh` |
| RPC | `sdk/app/src/app/ipc/rpc_client.cpp` |
| encode / rtsp | `/data/config/camera/encode.json`, `/data/config/network/rtsp.json` |
| 장치 정보 | `/data/config/system/sysinfo.json` |

---

## 8. VMS 연동 이슈 (2026-09-06) — 펌웨어 수정 요청

자체 VMS(C++/Qt6)에서 **Add Camera → Get Profiles** 가 `HTTP error: Connection closed` 로 실패한다. 앱과 무관하게 curl 로 재현되며, **같은 요청이 다른 기종(NS102HD-6117F, ONVIF 포트 80)에서는 정상**이므로 클라이언트 문제가 아니다.

### 8.1 대상

| 항목 | 값 |
|------|----|
| 모델 | NS6104HD-6211 (Discovery scope `hardware/NS6104HD-6211`, `location/city/shenzhen`) |
| IP / 포트 | 192.168.3.42, SOAP 8089, 웹 80, RTSP 554 |
| 펌웨어 버전 / `api.json` ONVIF 상태 | **미확인 — 담당자 확인 필요** (VMS 쪽에서는 알 수 없음) |
| 클라이언트 | Windows 11, curl 8.10 / Qt 6.10 `QNetworkAccessManager` |

### 8.2 증상

8089 포트의 `onvif_service` 는 살아 있고 HTTP 헤더까지는 정상 처리하지만, **POST 본문을 받는 순간 HTTP 응답을 한 바이트도 보내지 않고 소켓을 닫는다.**

| 요청 | 결과 |
|------|------|
| `GET /onvif/device_service` (및 아무 경로) | **405** + gSOAP Fault `HTTP GET method not implemented`, `Server: gSOAP/2.8` → 프로세스·포트 정상 |
| `POST GetSystemDateAndTime` (인증 불필요, 본 문서 §5 예제와 동일 형식) | **응답 0바이트, TCP 종료** (curl exit 52 "Empty reply from server") |
| `POST GetDeviceInformation` (§5 curl 예제 그대로) | 동일 |
| WS-Security UsernameToken 헤더 **있음** / **없음** | 둘 다 동일 (인증 실패라면 `ter:NotAuthorized` Fault 가 와야 하는데 그것도 없음) |
| HTTP Basic 인증 추가 | 동일 |
| HTTP/1.0, `Connection: close`, `Transfer-Encoding: chunked`, `Content-Type: text/xml`, XML 선언 유무, `Upgrade: h2c` 헤더 | 전부 동일 |
| 경로 `/onvif/device_service`, `/onvif/media`, `/onvif/Media`, `/onvif/media_service`, `/` | 전부 동일 |
| `Expect: 100-continue` | **`100 Continue` 는 온다.** 이어서 본문을 보내면 종료 → 헤더 파싱은 통과, **본문 처리 단계에서 죽거나 끊음** |

### 8.3 재현 (PC 에서)

```bash
# 1) 프로세스 확인: 405 + gSOAP Fault 가 오면 정상
curl -s -m 8 -o /dev/null -w "http=%{http_code}\n" http://192.168.3.42:8089/onvif/device_service

# 2) 가장 단순한 SOAP POST: 현재 http=000, exit=52
curl -s -m 8 -o /dev/null -w "http=%{http_code}\n" \
  -H "Content-Type: application/soap+xml; charset=utf-8" \
  --data '<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope"><s:Body><tds:GetSystemDateAndTime xmlns:tds="http://www.onvif.org/ver10/device/wsdl"/></s:Body></s:Envelope>' \
  http://192.168.3.42:8089/onvif/device_service; echo "exit=$?"
```

수정 후 2) 가 `http=200` 이면 VMS Get Profiles 도 동작한다.

### 8.4 추정 원인 (담당자 검증 필요)

본 문서 구조상 의심 순서:

1. **`mst_OnvifServeWithAuth()` (Phase 3, `onvif_auth.cpp`)** — `soap_serve` 대신 인증 게이트를 거친 뒤 `soap_serve_request` 를 부른다. 게이트가 WS-Security 헤더를 파싱하려고 **소켓에서 본문을 먼저 읽어 버리면**, 이후 gSOAP 의 `soap_begin_recv` 가 빈 스트림을 만나 실패하고 Fault 도 못 보낸 채 `soap_closesock` 한다. GET 은 `soap_begin_recv` 가 본문 전에 405 를 내므로 영향이 없다는 점과 맞아떨어진다. 게이트는 반드시 gSOAP 이 읽어 둔 버퍼(`soap->buf` / DOM 파싱 후 `soap_serve_request`) 위에서 동작해야 하며, 원시 `recv` 로 소켓을 소비하면 안 된다.
2. **게이트 내 크래시** — Security 헤더가 없거나 `userStreamAuthSecret` 이 없을 때(§5 "업그레이드 직후 ONVIF auth") NULL 참조 등으로 프로세스가 죽고, `start_app.sh` 의 `start_if_missing` 이 재기동해 **다음 GET 은 다시 정상으로 보이는** 시나리오. POST 전후로 `pidof onvif_service` 가 바뀌는지 보면 즉시 구분된다.
3. `AuthEnable=false` 인데도 같은 증상이면 1번, `AuthEnable=true` 에서만 나면 1·2 모두 가능.

> 인증 실패는 **응답 없는 종료가 아니라 SOAP Fault(`ter:NotAuthorized`) 또는 HTTP 401** 이어야 한다. 현재는 클라이언트가 "인증 오류" 와 "서버 죽음" 을 구분할 수 없다.

### 8.5 확인·회신 요청

보드에서 아래를 확인해 회신 바란다.

1. 펌웨어 버전, `/data/config/system/api.json` 의 `Api.ONVIF` 세 값.
2. §8.3 의 2) 를 보내면서 보드에서:
   ```bash
   pidof onvif_service            # 전/후 PID 비교 (바뀌면 크래시)
   logread | grep -i onvif        # 또는 stderr / dmesg segfault 로그
   ```
3. `AuthEnable=false` 로 바꾸고 재시도했을 때도 동일한지.
4. 가능하면 `strace -f -p $(pidof onvif_service) -e trace=network,read,write` 로 POST 1건 캡처.

### 8.6 VMS 가 요구하는 동작 (수정 후 검증 기준)

VMS 는 카메라 추가 시 다음 순서로 호출한다. 각 단계는 **반드시 HTTP 응답**(200 + SOAP, 또는 SOAP Fault, 또는 401)을 받아야 다음으로 진행한다.

| 순서 | 호출 | 대상 | 필요 응답 |
|------|------|------|-----------|
| 1 | `tds:GetCapabilities` (Category=All) | `/onvif/device_service` | Media XAddr (§4.1 대로 `http://<ip>:8089/onvif/media` 등) |
| 2 | `tds:GetDeviceInformation` | 〃 | Manufacturer / Model / Serial |
| 3 | `trt:GetProfiles` | 1 에서 받은 Media XAddr | `Profile_1`(video0), `Profile_2`(video1) 과 해상도·fps·코덱 |
| 4 | `trt:GetStreamUri` × 프로파일 수 | 〃 | `rtsp://<ip>:554/video0`, `/video1` |

요청 형식: HTTP/1.1, `Content-Type: application/soap+xml; charset=utf-8`, `SOAPAction` 헤더 포함, SOAP 1.2 Envelope, WS-Security UsernameToken **PasswordDigest** (`Nonce` Base64, `Created` UTC `...Z`), HTTP Basic 은 선제 전송하지 않고 401 챌린지에만 응답. Replay 보호 ON 을 대비해 매 요청 새 Nonce 를 쓴다.

### 8.7 부수 발견 (차단 아님, 여유 있을 때)

- WS-Discovery `ProbeMatch` 의 `XAddrs` 가 `"http://192.168.3.42:8089/onvif/device_service "` 처럼 **끝에 공백**이 있어 공백 분리 시 빈 항목이 생긴다. 공백 제거 권장.
- 웹 포트(80) `/onvif/device_service` 는 §5 표의 "404" 가 아니라 **200 + SPA `index.html`** 을 돌려준다. 동작에는 무해하나 ONVIF 클라이언트가 포트 80 을 먼저 찔러 보면 "XML 아님" 오류로 보인다. 405 나 404 가 더 명확하다.
- 이 기종의 RTSP 경로는 `/video0`·`/video1` 이라 VMS 가 DB 에 URL 이 없을 때 쓰는 기본값(`/stream1`·`/stream2`)과 다르다. 따라서 이 기종은 **`GetStreamUri` 가 반드시 동작해야** 재생이 된다.

### 8.8 RTSP 이슈 (같은 기종 2호기, 192.168.3.43, RTSP 인증 ON)

ONVIF 와 별개로 `video_core` RTSP(LIVE555 v2019.08.12) 쪽에서 2026-09-06 확인된 증상. 1호기(192.168.3.42, 인증 OFF)에서는 재현되지 않는다.

| 요청 (`ffprobe -rtsp_transport tcp -timeout 5000000`) | 결과 |
|------|------|
| `rtsp://192.168.3.43:554/video1` (인증 없음) | 401 Digest 챌린지 → 정상 |
| `rtsp://root:***@192.168.3.43:554/video0` | **간헐적**. 성공(H.264 2560×1440)과 실패가 10~30초 주기로 번갈아 나온다. 실패 시 TCP 는 붙지만 OPTIONS/DESCRIBE 응답이 5초 안에 오지 않는다 (`Failed reading RTSP data: -138 ETIMEDOUT`) |
| `rtsp://root:***@192.168.3.43:554/video1` | Digest 통과, DESCRIBE/SETUP/PLAY 모두 200, SDP 정상 (H.264 + `vnd.stcam.vca-meta`). **그러나 RTP 패킷이 한 개도 오지 않는다** (15초 대기해도 동일) |

확인 요청:

1. 2호기 `encode.json` 의 서브 스트림(video1) enable 여부와 실제 인코더 동작 여부. PLAY 200 뒤 RTP 가 0 이면 인코더가 프레임을 내지 않거나 RTSP 세션에 소스가 붙지 않은 것이다.
2. RTSP 서버가 주기적으로 멈추는 원인. 클라이언트 세션이 모두 끊긴 상태에서도(이 PC 에서 세션 0) 응답 없음이 반복되므로 세션 누적이 아니라 서버 자체 문제로 보인다. 인증 ON 과의 상관관계(1호기는 OFF) 확인 요망.
3. 인증 실패나 소스 없음은 응답 없는 대기가 아니라 4xx/5xx 로 즉시 돌려주길 바란다. VMS 는 5초 타임아웃으로 기다렸다가 실패 처리한다.

VMS 쪽에서는 이 두 증상이 장치 관리 표에 각각 "Online (sub stream failed)" 와 간헐적 "Offline" 으로 나타난다.

---

## 변경 이력

| 날짜 | 내용 |
|------|------|
| 2026-09-06 | §8 추가: VMS 연동 차단 이슈 (8089 SOAP POST 무응답 종료) 재현·추정 원인·수정 요청. |
| 2026-09-03 | Phase 5: SetSystemDateAndTime, SystemReboot, GetNetworkProtocols, Imaging RPC. Events PullPoint 제외. |
| 2026-09-03 | Phase 1–4 구현·테스트 절 추가. §4/§5 현행화. |
| 2026-09-03 | Phase 4: Replay (`Created` ±60s, Nonce 120s 캐시), UI 토글. |
| 2026-09-03 | Phase 3: WS-Security, `userStreamAuthSecret`, GetUsers, Onvif 계정 잠금. |
| 2026-09-03 | Phase 2: encode/rtsp 연동, RTSP URI, 스냅샷 HTTP, GetProfiles/VideoSources, Capabilities 버그 수정. |
| 2026-09-03 | Phase 1: Enable 토글 → `onvif_service` start/stop (`start_app.sh`, REST apply). |
| 2026-08-30 | 초안. 현재 설계(별도 프로세스·8089·데모 핸들러)와 Phase 1–6 TODO 정리. |
