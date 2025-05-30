@echo off
setlocal enabledelayedexpansion

:: ��·������
set SRC_DIR=%~dp0ixwebsocket_src
set REPO_URL=https://github.com/machinezone/IXWebSocket.git

:: ƽ̨�������б�
set PLATFORMS=x86 x64
set CONFIGS=Debug Release

:: ��¡�ֿ�
echo ----------------------------------
echo ��¡ ixwebsocket �ֿ�...
echo ----------------------------------
if exist "%SRC_DIR%" (
    echo �Ѵ��ڣ����� clone��
) else (
    git clone %REPO_URL% "%SRC_DIR%"
)

:: ����ƽ̨������
for %%P in (%PLATFORMS%) do (
    for %%C in (%CONFIGS%) do (
        echo ----------------------------------
        echo ����: ƽ̨=%%P ����=%%C
        echo ----------------------------------

        set BUILD_DIR=%SRC_DIR%\build-%%P-%%C
        set INSTALL_DIR=%SRC_DIR%\install-%%P-%%C

        :: �����Ŀ¼
        if exist "!BUILD_DIR!" rmdir /s /q "!BUILD_DIR!"
        mkdir "!BUILD_DIR!"
        pushd "!BUILD_DIR!"

        :: ���ñ������
        if %%C==Release (
            set RUNTIME=/MT
        ) else (
            set RUNTIME=/MTd
        )

        :: ���� CMake
        cmake .. ^
          -G "Visual Studio 17 2022" ^
          -A %%P ^
          -DCMAKE_BUILD_TYPE=%%C ^
          -DIXWEBSOCKET_USE_TLS=OFF ^
          -DIXWEBSOCKET_ENABLE_TESTS=OFF ^
          -DIXWEBSOCKET_ENABLE_HTTP_SERVER=OFF ^
          -DCMAKE_CXX_FLAGS="!RUNTIME!" ^
          -DCMAKE_INSTALL_PREFIX="!INSTALL_DIR!"

        if errorlevel 1 (
            echo  ����ʧ��: %%P %%C
            popd
            exit /b 1
        )

        cmake --build . --config %%C

        if errorlevel 1 (
            echo  ����ʧ��: %%P %%C
            popd
            exit /b 1
        )

        echo  �������: %%P %%C
        popd
    )
)

echo ----------------------------------
echo ���а汾�������
echo build-x86-Debug / Release
echo build-x64-Debug / Release
echo ----------------------------------

pause
endlocal
