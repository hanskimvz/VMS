# VMS 문서 목차

이 폴더는 **지능형 VMS(Intelligent VMS)** 프로젝트의 문서입니다.
제품 방향은 "기존 VMS 기능 위에 VLM 기반 영상 의미 검색을 주기능으로 얹는 16채널 제품"입니다.

## 읽는 순서

| 순서 | 문서 | 내용 | 대상 |
|------|------|------|------|
| 1 | [intelligent-vms-concept.md](intelligent-vms-concept.md) | 제품 개념, 사용자 시나리오, 요구사항, 목표 아키텍처, 메타데이터 스키마, Analyzer 인터페이스, 검색 설계, UI | 기획, 설계, 개발 전원 |
| 2 | [intelligent-vms-research.md](intelligent-vms-research.md) | VLM/엔진 선택지 조사, Ollama 연동 상세, 처리량·저장량·하드웨어 계산, 검색 방식 비교, 리스크, 단계별 로드맵, 의견 | 설계, 의사결정 |
| 3 | [architecture.md](architecture.md) | **현재 코드**의 레이어, 클래스, 데이터 흐름, 스레딩 모델. 구현 상태 표기 포함 | 개발 |
| 4 | [features.md](features.md) | **현재 구현된** 기능 상세와 미구현 항목 | 개발, QA |
| 5 | [development.md](development.md) | 개발 환경, 빌드, 코딩 규칙, FFmpeg/ONVIF 사용 패턴, 배포 | 개발 |
| 6 | [known-issues.md](known-issues.md) | 2026‑09 코드 리뷰에서 확인된 결함과 설계 이슈, 수정 우선순위 | 개발 |

## 문서 원칙

- `architecture.md`, `features.md`, `development.md`는 **코드가 실제로 하는 일**만 적습니다. 계획은 `intelligent-vms-*.md`로 보냅니다.
- 각 문서 상단에 마지막 검토일을 적습니다. 코드와 어긋난 부분을 발견하면 문서를 고치거나 `known-issues.md`에 남깁니다.
- 수치(처리량, 지연, 용량)는 실측값과 추정값을 구분해서 적습니다.
