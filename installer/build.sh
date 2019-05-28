#!/bin/bash

mkdir -p ROOT/tmp/SestoSenso_X2/
cp "../SestoSenso.ui" ROOT/tmp/SestoSenso_X2/
cp "../SestoCalibrate.ui" ROOT/tmp/SestoSenso_X2/
cp "../PrimaLuceLab.png" ROOT/tmp/SestoSenso_X2/
cp "../focuserlist PrimaLuceLab.txt" ROOT/tmp/SestoSenso_X2/
cp "../build/Release/libSestoSenso.dylib" ROOT/tmp/SestoSenso_X2/

if [ ! -z "$installer_signature" ]; then
# signed package using env variable installer_signature
pkgbuild --root ROOT --identifier org.rti-zone.SestoSenso_X2 --sign "$installer_signature" --scripts Scripts --version 1.0 SestoSenso_X2.pkg
pkgutil --check-signature ./SestoSenso_X2.pkg
else
pkgbuild --root ROOT --identifier org.rti-zone.SestoSenso_X2 --scripts Scripts --version 1.0 SestoSenso_X2.pkg
fi

rm -rf ROOT
