#!/bin/tcsh

# builds libraries from scratch
#
# Currently builds:
# 1) StEfficiencyMaker

echo "[i] loading embedding library"
starver SL17d_embed

echo "[i] Remove any existing libs"
rm -v libStEfficiencyMaker.so
rm -v StEfficiencyMaker.so
rm -v libStEfficiencyMakerLite.so
rm -v StEfficiencyMakerLite.so
rm -v libs/libStEfficiencyMaker.so
rm -v libs/StEfficiencyMaker.so
rm -v libs/libStEfficiencyMakerLite.so
rm -v libs/StEfficiencyMakerLite.so
rm -v sandbox/libStEfficiencyMaker.so
rm -v sandbox/StEfficiencyMaker.so
rm -v sandbox/libStEfficiencyMakerLite.so
rm -v sandbox/StEfficiencyMakerLite.so

setenv CXXFLAGSNEW "-pipe -fPIC -Wall -Woverloaded-virtual -ansi -Wno-long-long -pthread -m32 -std=c++11"
#LDFLAGS       += -m32

echo "[i] Changing CXXFLAGS to: "${CXXFLAGSNEW}

echo "[i] Running cons for StEfficiencyMaker"

cons CXXFLAGS="${CXXFLAGSNEW}" +StEfficiencyMaker +StEfficiencyMakerLite


# places copies of the libraries into the local lib directory
# as well as into sandbox/
echo "[i] Copying libraries to the lib & sandbox"
find .sl*/lib -name "libStEfficiencyMaker.so" -exec cp -v {} ./libs/ \;
find .sl*/lib -name "libStEfficiencyMaker.so" -exec cp -v {} ./sandbox/ \;
find .sl*/lib -name "libStEfficiencyMakerLite.so" -exec cp -v {} .libs/ \;
find .sl*/lib -name "libStEfficiencyMakerLite.so" -exec cp -v {} ./sandbox/ \;

