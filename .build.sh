#!/bin/bash
set -e

## -----------------------------------------
# Get the build requirements with ivy.

pushd ivy/dependencies
ant bootstrap
ant
popd


## -----------------------------------------
#  Build the 32 Bit Version

rm -rf build_windows32
mkdir build_windows32
pushd build_windows32
cmake -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_SHARED_LINKER_FLAGS=-m32 -DCMAKE_EXE_LINKER_FLAGS=-m32 -DCMAKE_MODULE_LINKER_FLAGS=-m32 -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ -DCMAKE_RC_COMPILER=i686-w64-mingw32-windres ..
make
popd


## -----------------------------------------
#  Build the 64 Bit Version

rm -rf build_windows64
mkdir build_windows64
pushd build_windows64
cmake -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64 -DCMAKE_SHARED_LINKER_FLAGS=-m64 -DCMAKE_EXE_LINKER_FLAGS=-m64 -DCMAKE_MODULE_LINKER_FLAGS=-m64 -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres ..
make
popd


## ----------------------------------------
#  Assemble the artifacts

#rm -rf build
#mkdir build
#pushd build
#cmake ../ivy/installer
#make
#make install
#popd


#MBS_IVY=${PWD}/mbs_ivy
#pushd build/coco_gui
#ant -Dmbs.ivy.path=${MBS_IVY} bootstrap
#ant -Dmbs.ivy.path=${MBS_IVY}
