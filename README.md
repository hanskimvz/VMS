# VMS - 지능형 Video Management System

Qt6와 FFmpeg 기반의 16채널 IP 카메라 관리 소프트웨어입니다.

제품 목표는 **지능형 VMS**입니다. 기존 VMS 기능(라이브, ONVIF, 녹화, 재생) 위에, 로컬 VLM(Vision‑Language Model)이 각 채널을 주기적으로 읽어 메타데이터로 남기고 사용자가 자연어로 과거 영상을 검색해 바로 재생하는 기능을 주기능으로 얹습니다.

> "1일 전 노란색 옷 입고 걸어가는 남자 찾아줘" → 채널·시각·썸네일 목록 → 클릭하면 재생

개념과 설계, 조사 결과는 [doc/](doc/README.md)에 있습니다.

## 현재 구현된 기능

- **라이브 뷰**: 1×1, 2×2, 1+7, 3×3, 4×4 레이아웃. 셀 수에 따라 메인/서브 스트림 자동 전환
- **장치 검색**: ONVIF WS‑Discovery + mDNS + SSDP 동시 검색, 결과 병합
- **ONVIF**: 프로필/스트림 URL 자동 조회, 장치 정보, 네트워크 설정 변경, 재부팅
- **RTSP 카메라** 직접 추가, 미리보기
- **재생**: 로컬 파일 재생, 속도 조절, 탐색
- **카메라 관리**: SQLite 저장

## 아직 없는 것

- 녹화와 시각 기반 재생 (클래스만 있고 연결되지 않음)
- PTZ 패널 (위젯만 있음)
- AI 분석·검색 전체

자세한 상태는 [doc/features.md](doc/features.md), 결함 목록은 [doc/known-issues.md](doc/known-issues.md)를 보세요.

## 시스템 요구사항

- Windows 10/11 (64‑bit), MSYS2 MinGW64
- Qt 6.10, FFmpeg 8.0, GCC 15
- (계획) AI 분석용 GPU: 16GB VRAM 이상 권장. [doc/intelligent-vms-research.md](doc/intelligent-vms-research.md) 6절 참고

## 빌드

```bash
pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,qt6-base,qt6-tools,ffmpeg,sqlite3}

cd d:/Projects/VMS
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./build/VMS.exe
```

배포 패키지는 `deploy.bat`으로 만듭니다. 상세는 [doc/development.md](doc/development.md).

## 문서

| 문서 | 내용 |
|------|------|
| [doc/README.md](doc/README.md) | 문서 목차 |
| [doc/intelligent-vms-concept.md](doc/intelligent-vms-concept.md) | 제품 개념, 요구사항, 목표 아키텍처, 데이터 스키마, Analyzer API, 검색·UI 설계 |
| [doc/intelligent-vms-research.md](doc/intelligent-vms-research.md) | VLM 엔진 조사, Ollama 연동, 처리량·용량·하드웨어 계산, 리스크, 로드맵, 의견 |
| [doc/architecture.md](doc/architecture.md) | 현재 코드 아키텍처 |
| [doc/features.md](doc/features.md) | 현재 기능 상세 |
| [doc/development.md](doc/development.md) | 개발 가이드 |
| [doc/known-issues.md](doc/known-issues.md) | 코드 리뷰 결함 목록 |

## 기술 스택

| 구성요소 | 기술 | 버전 |
|----------|------|------|
| GUI | Qt6 Widgets | 6.10 |
| 비디오 | FFmpeg | 8.0.1 |
| 빌드 | CMake | 3.21+ |
| DB | SQLite (+ 계획: FTS5, sqlite‑vec) | 3.x |
| 컴파일러 | GCC (MinGW64) | 15.2 |
| 프로토콜 | ONVIF, RTSP, WS‑Discovery, mDNS, SSDP | |
| AI (계획) | Ollama + Gemma 4 (교체 가능), bge‑m3 임베딩 | |

## 참고 프로젝트

[rapidvms](https://github.com/veyesys/rapidvms)(AGPL v3)의 구조를 참고했으며, Qt6 + MinGW64 환경에 맞게 새로 작성했습니다. 원본 코드는 복사하지 않습니다.

## 라이선스

개인/학습 목적으로 개발 중입니다.
