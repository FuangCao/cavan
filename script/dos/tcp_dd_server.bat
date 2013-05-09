:loop
adb wait-for-device || goto label_pause
adb push ..\..\out\arm\static\cavan-main /data/internal_memory/ || goto label_pause
adb shell chmod 777 /data/internal_memory/cavan-main || goto label_pause
adb shell /data/internal_memory/cavan-main tcp_dd_server || goto label_pause
goto loop

:label_pause
pause
goto loop