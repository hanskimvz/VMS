@echo off
setlocal enabledelayedexpansion

echo ========================================
echo VMS Deployment Script
echo ========================================
echo.

:: 설정
set "PROJECT_DIR=%~dp0"
set "BUILD_DIR=%PROJECT_DIR%build"
set "DEPLOY_DIR=%PROJECT_DIR%deploy"
set "EXE_NAME=VMS.exe"

:: MSYS2/MinGW64 경로 설정
set "MINGW_BIN=C:\msys64\mingw64\bin"
set "WINDEPLOYQT=%MINGW_BIN%\windeployqt.exe"

:: 필요한 도구 확인
if not exist "%WINDEPLOYQT%" (
    echo [ERROR] windeployqt.exe를 찾을 수 없습니다: %WINDEPLOYQT%
    echo MINGW_BIN 경로를 확인하세요.
    pause
    exit /b 1
)

:: 빌드된 exe 확인
if not exist "%BUILD_DIR%\%EXE_NAME%" (
    echo [ERROR] %EXE_NAME%를 찾을 수 없습니다.
    echo 먼저 Release 모드로 빌드하세요:
    echo   cd build
    echo   cmake .. -DCMAKE_BUILD_TYPE=Release
    echo   cmake --build . --config Release
    pause
    exit /b 1
)

:: 배포 폴더 생성
echo [1/6] 배포 폴더 생성...
if exist "%DEPLOY_DIR%" (
    echo 기존 배포 폴더 삭제 중...
    rmdir /s /q "%DEPLOY_DIR%"
)
mkdir "%DEPLOY_DIR%"

:: EXE 복사
echo [2/6] 실행 파일 복사...
copy "%BUILD_DIR%\%EXE_NAME%" "%DEPLOY_DIR%\" >nul

:: windeployqt 실행
echo [3/6] Qt 의존성 복사 (windeployqt)...
"%WINDEPLOYQT%" --release --no-translations --no-system-d3d-compiler --no-opengl-sw "%DEPLOY_DIR%\%EXE_NAME%"
if errorlevel 1 (
    echo [WARNING] windeployqt에서 경고가 발생했습니다.
)

:: FFmpeg 핵심 DLL 복사
echo [4/6] FFmpeg DLL 복사...
set "FFMPEG_DLLS=avcodec-62.dll avformat-62.dll avutil-60.dll swscale-9.dll swresample-6.dll"
for %%f in (%FFMPEG_DLLS%) do (
    if exist "%MINGW_BIN%\%%f" (
        copy "%MINGW_BIN%\%%f" "%DEPLOY_DIR%\" >nul
        echo   - %%f
    ) else (
        echo [WARNING] %%f를 찾을 수 없습니다.
    )
)

:: MinGW 런타임 DLL 복사
echo [5/6] MinGW 런타임 DLL 복사...
set "RUNTIME_DLLS=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll"
for %%f in (%RUNTIME_DLLS%) do (
    if exist "%MINGW_BIN%\%%f" (
        copy "%MINGW_BIN%\%%f" "%DEPLOY_DIR%\" >nul
        echo   - %%f
    ) else (
        echo [WARNING] %%f를 찾을 수 없습니다.
    )
)

:: FFmpeg 의존성 DLL 복사 (avcodec, avformat이 필요로 하는 모든 DLL)
echo [6/6] FFmpeg 의존성 DLL 복사...

:: 코덱 관련
set "CODEC_DLLS=libaom.dll libdav1d-7.dll libgsm.dll libjxl.dll libjxl_threads.dll liblc3-1.dll"
set "CODEC_DLLS=%CODEC_DLLS% libmp3lame-0.dll libopencore-amrnb-0.dll libopencore-amrwb-0.dll"
set "CODEC_DLLS=%CODEC_DLLS% libopenjp2-7.dll libopus-0.dll librav1e.dll libshaderc_shared.dll"
set "CODEC_DLLS=%CODEC_DLLS% libspeex-1.dll libSvtAv1Enc-4.dll libtheoradec-2.dll libtheoraenc-2.dll"
set "CODEC_DLLS=%CODEC_DLLS% libvorbis-0.dll libvorbisenc-2.dll libvpx-1.dll"
set "CODEC_DLLS=%CODEC_DLLS% libwebp-7.dll libwebpmux-3.dll libx264-165.dll libx265-215.dll xvidcore.dll"
set "CODEC_DLLS=%CODEC_DLLS% libzvbi-0.dll"

:: 그래픽/렌더링 관련
set "GFX_DLLS=libcairo-2.dll librsvg-2-2.dll libva.dll libvpl-2.dll"

:: 포맷/프로토콜 관련
set "FORMAT_DLLS=libbluray-3.dll libgme.dll libgnutls-30.dll libmodplug-1.dll"
set "FORMAT_DLLS=%FORMAT_DLLS% librtmp-1.dll libsrt.dll libssh.dll libxml2-16.dll"

:: 기본 라이브러리
set "BASE_DLLS=libbz2-1.dll libiconv-2.dll liblzma-5.dll zlib1.dll"
set "BASE_DLLS=%BASE_DLLS% libglib-2.0-0.dll libgobject-2.0-0.dll"
set "BASE_DLLS=%BASE_DLLS% libssl-3-x64.dll libcrypto-3-x64.dll"

:: 추가 의존성 (위 DLL들이 필요로 하는)
set "EXTRA_DLLS=libintl-8.dll libffi-8.dll libpcre2-8-0.dll libpng16-16.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libfreetype-6.dll libharfbuzz-0.dll libfontconfig-1.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libpixman-1-0.dll libexpat-1.dll libbrotlidec.dll libbrotlicommon.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libgdk_pixbuf-2.0-0.dll libgio-2.0-0.dll libgmodule-2.0-0.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangoft2-1.0-0.dll libpangowin32-1.0-0.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libogg-0.dll libhogweed-6.dll libnettle-8.dll libgmp-10.dll libp11-kit-0.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libtasn1-6.dll libunistring-5.dll libidn2-0.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libsoxr.dll libgomp-1.dll libdovi.dll libSPIRV.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libjpeg-8.dll libtiff-6.dll libdeflate.dll libjbig-0.dll libLerc.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libstdc++-6.dll libzstd.dll liblcms2-2.dll libhwy.dll libbrotlienc.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libsharpyuv-0.dll libfribidi-0.dll libthai-0.dll libdatrie-1.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libglslang.dll libspirvd.dll"
:: 추가 발견된 의존성
set "EXTRA_DLLS=%EXTRA_DLLS% libva_win32.dll libgraphite2.dll libjxl_cms.dll libcairo-gobject-2.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libb2-1.dll libdouble-conversion.dll libicuin78.dll libicuuc78.dll libicudt78.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libpcre2-16-0.dll libmd4c.dll"
set "EXTRA_DLLS=%EXTRA_DLLS% libsqlite3-0.dll"

:: 모든 DLL 복사
for %%f in (%CODEC_DLLS% %GFX_DLLS% %FORMAT_DLLS% %BASE_DLLS% %EXTRA_DLLS%) do (
    if exist "%MINGW_BIN%\%%f" (
        copy "%MINGW_BIN%\%%f" "%DEPLOY_DIR%\" >nul
        echo   - %%f
    )
)

echo.
echo ========================================
echo 배포 완료!
echo ========================================
echo 배포 폴더: %DEPLOY_DIR%
echo.
echo 다른 PC에서 실행하기 전에:
echo 1. deploy 폴더 전체를 복사하여 사용하세요.
echo 2. 실행 시 오류가 나면 알려주세요.
echo.

:: 배포 폴더 열기
explorer "%DEPLOY_DIR%"

pause
