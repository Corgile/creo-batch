@ECHO OFF

for %%I in (%CD%) do set "cwd=%%~nI"

if /I "%cwd%" EQU "GBLib" (
    goto start_processing
) else (
    echo .
    echo .    This cmd file must be inside GBLib folder
    echo .    只能在 GBLib 文件夹内运行此脚本
    echo .

    pause
    exit /b
)

:start_processing

SETLOCAL EnableDelayedExpansion

SET "prefix=A-"
del /s /q *.mnu

for /d /r %%d in (*) do (
    set "name=%%~nxd"
    if NOT "!name:~0,1!" == "." (
        if NOT "!name:~0,2!" == "%prefix%" (
           ren "%%d" "%prefix%!name!"
        )
    )

)

for /r %%d in (.) do (
    pushd "%%d"
    set count=0
    for /f "delims=" %%c in ('dir /b ^| findstr /r /i /c:"\.asm[.0-9]*$" /c:"\.prt[.0-9]*$" /c:"^[^.]*$"') do (
        set "myList[!count!]=%%~nxc"
        set /a count+=1
    )
    set /a count-=1
    for %%I in ("!CD!") do set "folderName=%%~nxI"
    if "!folderName:~0,1!" == "." goto skip_folder_processing
    set "menuFile=!folderName!.mnu"
    cmd /a /c echo !folderName!>!menuFile!
    cmd /a /c echo #>>!menuFile!
    for /l %%d in (0, 1, !count!) do (
        set "item=!myList[%%d]!"
        set "comment=!item:.prt=!"
        cmd /a /c echo #>>!menuFile!
        if "!item!" == "!comment!" (
            cmd /a /c echo /!item!>>!menuFile!
        ) else (
            cmd /a /c echo !item!>>!menuFile!
        )
        cmd /a /c echo !comment!>>!menuFile!
    )
    cmd /a /c echo #>>!menuFile!
    :skip_folder_processing
    popd
)

rem .\pro_build_library_ctg.exe