@echo off

set PHONE_TEMP_PATH=/data/internal_memory
set CAVAN_MAIN=%PHONE_TEMP_PATH%/cavan-main

:label_loop
@echo wait for adb device
adb wait-for-device || goto label_pause
adb shell "mkdir %PHONE_TEMP_PATH% 2>/dev/null" || goto label_pause
adb push ..\..\out\arm\static\cavan-main %CAVAN_MAIN% || goto label_pause
adb shell "chmod 777 %CAVAN_MAIN%" || goto label_pause
adb shell "%CAVAN_MAIN% tcp_dd_server" && goto label_loop

:label_pause
pause
goto label_loop
