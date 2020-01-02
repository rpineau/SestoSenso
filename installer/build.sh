#!/bin/bash

PACKAGE_NAME="SestoSenso_X2.pkg"
BUNDLE_NAME="org.rti-zone.SestoSensoX2"

if [ ! -z "$app_id_signature" ]; then
    codesign -f -s "$app_id_signature" --verbose ../build/Release/libSestoSenso.dylib
fi

mkdir -p ROOT/tmp/SestoSenso_X2/
cp "../SestoSenso.ui" ROOT/tmp/SestoSenso_X2/
cp "../SestoCalibrate.ui" ROOT/tmp/SestoSenso_X2/
cp "../PrimaLuceLab.png" ROOT/tmp/SestoSenso_X2/
cp "../focuserlist PrimaLuceLab.txt" ROOT/tmp/SestoSenso_X2/
cp "../build/Release/libSestoSenso.dylib" ROOT/tmp/SestoSenso_X2/


if [ ! -z "$installer_signature" ]; then
	# signed package using env variable installer_signature
	pkgbuild --root ROOT --identifier $BUNDLE_NAME --sign "$installer_signature" --scripts Scripts --version 1.0 $PACKAGE_NAME
	pkgutil --check-signature ./${PACKAGE_NAME}

else
    pkgbuild --root ROOT --identifier $BUNDLE_NAME --scripts Scripts --version 1.0 $PACKAGE_NAME
fi

rm -rf ROOT
