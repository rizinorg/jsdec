echo off
set ARCH="%1"
set PKG_CONFIG_PATH=%CD%\..\rizin\lib\pkgconfig
set CFLAGS="-I%CD%\..\rizin\include\librz -I%CD%\..\rizin\include\librz\sdb"
set LDFLAGS=-L%CD%\..\rizin\lib
set PATH=%CD%\..\rizin\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" "%ARCH%"
meson --buildtype=release --prefix="%CD%\..\rizin" build || exit /b 666
ninja -C build install || exit /b 666
rizin.exe -e log.level=2 -Qc "Lc" || exit /b 0
rizin.exe -Qc "af ; pdd" "%CD%\..\rizin\bin\rizin.exe" || exit /b 0
