@ECHO OFF

del /s /q *.mnu

SET "prefix=A-"
SETLOCAL EnableDelayedExpansion
for /D /R %%G in (*) do (
    set "folder=%%~nxG"
    set "newFolder=!folder:%prefix%=!"
    if not "!newFolder!" == "!folder!" (
        ren "%%G" "!newFolder!"
    )
)
ENDLOCAL