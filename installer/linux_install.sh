#!/bin/bash

TheSkyX_Install=`/usr/bin/find ~/Library/Application\ Support/Software\ Bisque/ -name TheSkyXInstallPath.txt`
echo "TheSkyX_Install = $TheSkyX_Install"

if [ ! -f "$TheSkyX_Install" ]; then
    echo TheSkyXInstallPath.txt not found
    TheSkyX_Path=`/usr/bin/find ~/ -maxdepth 3 -name TheSkyX`
    if [ -d "$TheSkyX_Path" ]; then
		TheSkyX_Path="${TheSkyX_Path}/Contents"
    else
	   echo TheSkyX application was not found.
    	exit 1
	 fi
else
	TheSkyX_Path=$(<"$TheSkyX_Install")
fi

echo "Installing to $TheSkyX_Path"


if [ ! -d "$TheSkyX_Path" ]; then
    echo TheSkyX Install dir not exist
    exit 1
fi

cp "./focuserlist PrimaLuceLab.txt" "$TheSkyX_Path/Resources/Common/Miscellaneous Files/"
cp "./SestoSenso.ui" "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/"
cp "./SestoCalibrate.ui" "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/"
cp "./PrimaLuceLab.png" "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/"
cp "./libSestoSenso.so" "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/"

app_owner=`/usr/bin/stat -c "%u" "$TheSkyX_Path" | xargs id -n -u`
if [ ! -z "$app_owner" ]; then
	chown $app_owner "$TheSkyX_Path/Resources/Common/Miscellaneous Files/focuserlist PrimaLuceLab.txt"
	chown $app_owner "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/SestoSenso.ui"
	chown $app_owner "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/SestoCalibrate.ui"
	chown $app_owner "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/PrimaLuceLab.png"
	chown $app_owner "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/libSestoSenso.so"
fi
chmod  755 "$TheSkyX_Path/Resources/Common/PlugIns/FocuserPlugIns/libSestoSenso.so"