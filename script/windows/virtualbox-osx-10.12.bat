cd C:\Program Files\Oracle\VirtualBox
VBoxManage modifyvm "OSX-10.12" --cpuidset 00000001 000106e5 00100800 0098e3fd bfebfbff
VBoxManage setextradata "OSX-10.12" "VBoxInternal/Devices/efi/0/Config/DmiSystemProduct" "iMac11,3"
VBoxManage setextradata "OSX-10.12" "VBoxInternal/Devices/efi/0/Config/DmiSystemVersion" "1.0"
VBoxManage setextradata "OSX-10.12" "VBoxInternal/Devices/efi/0/Config/DmiBoardProduct" "Iloveapple"
VBoxManage setextradata "OSX-10.12" "VBoxInternal/Devices/smc/0/Config/DeviceKey" "ourhardworkbythesewordsguardedpleasedontsteal(c)AppleComputerInc"
VBoxManage setextradata "OSX-10.12" "VBoxInternal/Devices/smc/0/Config/GetKeyFromRealSMC" 1
pause
