:loop
adb wait-for-device || exit 1
adb push ..\..\out\arm\static\cavan-main /data/internal_memory/ || exit 1
adb shell chmod 777 /data/internal_memory/cavan-main || exit 1
adb shell /data/internal_memory/cavan-main tcp_dd_server
goto loop