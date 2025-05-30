@echo off
setlocal enabledelayedexpansion

:: 根路径设置
set SRC_DIR=%~dp0ixwebsocket_src
set REPO_URL=https://github.com/machinezone/IXWebSocket.git

:: 平台与配置列表
set PLATFORMS=x86 x64
set CONFIGS=Debug Release

:: 克隆仓库
echo ----------------------------------
echo 克隆 ixwebsocket 仓库...
echo ----------------------------------
if exist "%SRC_DIR%" (
    echo 已存在，跳过 clone。
) else (
    git clone %REPO_URL% "%SRC_DIR%"
)

:: 遍历平台和配置
for %%P in (%PLATFORMS%) do (
    for %%C in (%CONFIGS%) do (
        echo ----------------------------------
        echo 编译: 平台=%%P 配置=%%C
        echo ----------------------------------

        set BUILD_DIR=%SRC_DIR%\build-%%P-%%C
        set INSTALL_DIR=%SRC_DIR%\install-%%P-%%C

        :: 清理旧目录
        if exist "!BUILD_DIR!" rmdir /s /q "!BUILD_DIR!"
        mkdir "!BUILD_DIR!"
        pushd "!BUILD_DIR!"

        :: 设置编译参数
        if %%C==Release (
            set RUNTIME=/MT
        ) else (
            set RUNTIME=/MTd
        )

        :: 配置 CMake
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
            echo  配置失败: %%P %%C
            popd
            exit /b 1
        )

        cmake --build . --config %%C

        if errorlevel 1 (
            echo  编译失败: %%P %%C
            popd
            exit /b 1
        )

        echo  编译完成: %%P %%C
        popd
    )
)

echo ----------------------------------
echo 所有版本构建完成
echo build-x86-Debug / Release
echo build-x64-Debug / Release
echo ----------------------------------

pause
endlocal
