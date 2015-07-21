#!/bin/bash
NDK=$1
if [ "${NDK}" == "" ] || [ ! -e ${NDK}/build/tools/make-standalone-toolchain.sh ]
then
  echo "Please specify a valid NDK path."
  exit 1
fi
BOOST_ROOT=$2
if [ "${NDK}" == "" ]
then
  echo "Please specify a valid BOOST_ROOT path."
  exit 1
fi

if ["${ANDROID_ABI}" == ""]
then
  echo "Please set the ANDROID_ABI variable to one of 'armeabi, armeabi-v7a, x86'"
  exit 1
fi

export ANDTOOLCHAIN="../target/android/android.toolchain.cmake"
export ANDROID_NDK="${NDK}"

echo "ANDTOOLCHAIN: ${ANDTOOLCHAIN}"
echo "ANDROID_NDK: ${ANDROID_NDK}"

CMAKE_CMD="cmake \
  -DCMAKE_TOOLCHAIN_FILE=${ANDTOOLCHAIN} \
  -DHTTP_WITH_ZLIB=OFF \
  -DENABLE_POSTGRES=OFF \
  -DENABLE_SSL=OFF \
  -DBOOST_PREFIX=${BOOST_ROOT} \
  -DBOOST_VERSION=1_52_0 \
  -DWT_NO_BOOST_RANDOM=ON \
  -DCMAKE_INSTALL_PREFIX=${BOOST_ROOT} \
  -DBoost_ADDITIONAL_VERSIONS=1.52 \
  -DSHARED_LIBS=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DANDROID_API_LEVEL=14 \
  -DANDROID_USE_STLPORT=OFF \
  -DANDROID_NO_UNDEFINED=ON \
  -DANDROID_ABI=${ANDROID_ABI} ../"

echo $CMAKE_CMD
eval $CMAKE_CMD || echo "CMake failed"; exit

cd src/Wt/Dbo
make -9
#make install
