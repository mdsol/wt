#
# Builds Wt as a framework for iPhone/iPad devices.
# Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
#
# Inspired by Pete Goodliffe's boost.sh script
#

: ${XCODE_ROOT:=`xcode-select -print-path`}
: ${BOOST_FRAMEWORK_PATH:=/Users/koen/project/wt/ipad/boostoniphone/framework}
: ${IOS_SDK:=/Developer/Platforms/iPhoneOS.platform/Developer}
: ${ISIM_SDK:=/Developer/Platforms/iPhoneSimulator.platform/Developer}
: ${SDK_VER:=5.0}

CONCURRENT_JOBS=4
BUILD_DBO_ONLY=false
BUILD_DBO_COMMAND=``
BOOST_FRAMEWORK_PATH=/Users/tho/external-libs/boost/ios/framework
SDK_VER=8.1
IOS_SDK=$XCODE_ROOT/Platforms/iPhoneOS.platform/Developer
ISIM_SDK=$XCODE_ROOT/Platforms/iPhoneSimulator.platform/Developer
IOS_DEV=$XCODE_ROOT/Toolchains/XcodeDefault.xctoolchain
IOS_DEV=$XCODE_ROOT/Toolchains/XcodeDefault.xctoolchain

while getopts "j:d" opt; do
    case "$opt" in
        j) CONCURRENT_JOBS=$OPTARG;;
        d) BUILD_DBO_ONLY=true; BUILD_DBO_COMMAND="cd ./src/Wt/Dbo ";;
        *) printf '%s\n' "I don't know what that argument is!" ;;
    esac 
done



echo "Number of concurrent jobs: ${CONCURRENT_JOBS}"
echo "Build DBO only? ${BUILD_DBO_ONLY}: ${BUILD_DBO_COMMAND}"

: ${CMAKE:=cmake}

: ${BUILD_DIR:=`pwd`}
: ${WT_DIR:=`pwd`/..}
: ${WT_VERSION:=3_2_0}
: ${FRAMEWORKDIR:=`pwd`}

BUILD_ARMV6_DIR=$BUILD_DIR/build-armv6
BUILD_ARMV7_DIR=$BUILD_DIR/build-armv7
BUILD_I386_DIR=$BUILD_DIR/build-i386
BUILD_X8664_DIR=$BUILD_DIR/build-x8664
STAGE_DIR=$BUILD_DIR/stage
CONFIG_DIR=$BUILD_DIR/etc
TMP_DIR=$BUILD_DIR/tmp

: ${COMMON_CMAKE_FLAGS=-DUSE_BOOST_FRAMEWORK=ON \
       -DCMAKE_FRAMEWORK_PATH:FILEPATH=$BOOST_FRAMEWORK_PATH \
       -DBOOST_PREFIX=$BOOST_FRAMEWORK_PATH \
       -DCMAKE_INSTALL_PREFIX=$STAGE_DIR \
       -DCONFIGDIR=$CONFIG_DIR \
       -DSHARED_LIBS=OFF \
       -DCONNECTOR_FCGI=OFF \
       -DBUILD_TESTS=OFF \
       -DBUILD_EXAMPLES=OFF \
       -DENABLE_GM=OFF \
       -DENABLE_HARU=OFF \
       -DENABLE_QT4=OFF \
       -DENABLE_SSL=OFF \
       -DENABLE_POSTGRES=OFF \
       -DENABLE_FIREBIRD=OFF \
       -DHTTP_WITH_ZLIB=OFF}

abort()
{
    echo
    echo "Aborted: $@"
    exit 1
}

clean()
{
    rm -rf $BUILD_ARMV6_DIR
    rm -rf $BUILD_ARMV7_DIR
    rm -rf $BUILD_I386_DIR
    rm -rf $BUILD_X8664_DIR
    rm -rf $TMP_DIR
}

build-armv6()
{
    rm -rf $BUILD_X8664_DIR
   [ -d $BUILD_ARMV6_DIR ] || mkdir -p $BUILD_ARMV6_DIR
   ( cd $BUILD_ARMV6_DIR; ${CMAKE} \
       $COMMON_CMAKE_FLAGS \
       -DCMAKE_C_COMPILER:FILEPATH=$IOS_DEV/usr/bin/clang \
       -DCMAKE_CXX_COMPILER:FILEPATH=$IOS_DEV/usr/bin/clang++ \
       -DCMAKE_C_FLAGS:STRING="-mthumb -fvisibility=hidden -isysroot $IOS_SDK/SDKs/iPhoneOS${SDK_VER}.sdk -arch armv6 -pipe" \
       -DCMAKE_CXX_FLAGS:STRING="-mthumb -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $IOS_SDK/SDKs/iPhoneOS${SDK_VER}.sdk -arch armv6 -pipe -DWT_NO_SPIRIT" \
       ../../ && make ) || abort "Failed building for arm6 architecture"
}

build-armv7()
{
   [ -d $BUILD_ARMV7_DIR ] || mkdir -p $BUILD_ARMV7_DIR
   ( cd $BUILD_ARMV7_DIR; ${CMAKE} \
       $COMMON_CMAKE_FLAGS \
       -DCMAKE_C_COMPILER:FILEPATH=$IOS_DEV/usr/bin/clang \
       -DCMAKE_CXX_COMPILER:FILEPATH=$IOS_DEV/usr/bin/clang++ \
       -DCMAKE_C_FLAGS:STRING="-mthumb -fvisibility=hidden -isysroot $IOS_SDK/SDKs/iPhoneOS${SDK_VER}.sdk -arch armv7 -pipe" \
       -DCMAKE_CXX_FLAGS:STRING="-mthumb -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $IOS_SDK/SDKs/iPhoneOS${SDK_VER}.sdk -arch armv7 -pipe -DWT_NO_SPIRIT" \
       -DCMAKE_OSX_SYSROOT=${IOS_SDK}/SDKs/iPhoneOS${SDK_VER}.sdk \
       ../../ && eval ${BUILD_DBO_COMMAND} && make -j${CONCURRENT_JOBS}) || abort "Failed building for arm7 architecture"
}

build-i386()
{
   [ -d $BUILD_I386_DIR ] || mkdir -p $BUILD_I386_DIR
   ( cd $BUILD_I386_DIR; ${CMAKE} \
       $COMMON_CMAKE_FLAGS \
       -DCMAKE_C_COMPILER:FILEPATH=$ISIM_DEV/usr/bin/clang \
       -DCMAKE_CXX_COMPILER:FILEPATH=$ISIM_DEV/usr/bin/clang++ \
       -DCMAKE_C_FLAGS:STRING="-arch i386 -fvisibility=hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator$SDK_VER.sdk -miphoneos-version-min=7.0" \
       -DCMAKE_CXX_FLAGS:STRING="-arch i386 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator$SDK_VER.sdk -DWT_NO_SPIRIT -miphoneos-version-min=7.0" \
       -DCMAKE_OSX_SYSROOT=${ISIM_SDK}/SDKs/iPhoneSimulator${SDK_VER}.sdk \
       ../../ && eval ${BUILD_DBO_COMMAND} && make -j${CONCURRENT_JOBS} && make install) || abort "Failed building for simulator"

	# Backup C/CXX FLAGS
       #-DCMAKE_C_FLAGS:STRING="-arch i386 -fvisibility=hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator${SDK_VER}.sdk -miphoneos-version-min=7.0" \
       #-DCMAKE_CXX_FLAGS:STRING="-arch i386 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator${SDK_VER}.sdk -DWT_NO_SPIRIT -miphoneos-version-min=7.0" \
}

build-x8664()
{
   [ -d $BUILD_X8664_DIR ] || mkdir -p $BUILD_X8664_DIR
   ( cd $BUILD_X8664_DIR; ${CMAKE} \
       $COMMON_CMAKE_FLAGS \
       -DCMAKE_C_COMPILER:FILEPATH=$ISIM_DEV/usr/bin/clang \
       -DCMAKE_CXX_COMPILER:FILEPATH=$ISIM_DEV/usr/bin/clang++ \
       -DCMAKE_C_FLAGS:STRING="-arch x86_64 -fvisibility=hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator$SDK_VER.sdk -miphoneos-version-min=7.0" \
       -DCMAKE_CXX_FLAGS:STRING="-arch x86_64 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator$SDK_VER.sdk -DWT_NO_SPIRIT -miphoneos-version-min=7.0" \
       -DCMAKE_OSX_SYSROOT=${ISIM_SDK}/SDKs/iPhoneSimulator${SDK_VER}.sdk \
       ../../ && eval ${BUILD_DBO_COMMAND} && make -j${CONCURRENT_JOBS} && make install) || abort "Failed building for simulator"

	# Backup C/CXX FLAGS
       #-DCMAKE_C_FLAGS:STRING="-arch x86_64 -fvisibility=hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator${SDK_VER}.sdk -miphoneos-version-min=7.0" \
       #-DCMAKE_CXX_FLAGS:STRING="-arch x86_64 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot $ISIM_SDK/SDKs/iPhoneSimulator${SDK_VER}.sdk -DWT_NO_SPIRIT -miphoneos-version-min=7.0" \
}

combineLibs()
{
    : ${1:?}
    BUILD_DIR=$1
    [ -d $TMP_DIR ] || mkdir -p $TMP_DIR
    if [ $BUILD_DBO_ONLY != true ] ; then
    ( mkdir -p $TMP_DIR/http &&
	cd $TMP_DIR/http &&
	ar x $BUILD_DIR/src/http/libwthttp.a ) || abort "Extract libwthttp.a failed"
    fi
    ( mkdir -p $TMP_DIR/dbosqlite3 &&
	cd $TMP_DIR/dbosqlite3 &&
        ar x $BUILD_DIR/src/Wt/Dbo/backend/libwtdbosqlite3.a sqlite3.o &&
        mv sqlite3.o db_sqlite3.o &&
        ar x $BUILD_DIR/src/Wt/Dbo/backend/libwtdbosqlite3.a Sqlite3.o) || abort "Extract libwtdbosqlite3.a failed"
    ( mkdir -p $TMP_DIR/dbo &&
	cd $TMP_DIR/dbo &&
        ar x $BUILD_DIR/src/Wt/Dbo/libwtdbo.a) || abort "Extract libwtdbo.a failed"
    if [ $BUILD_DBO_ONLY != true ] ; then
    ( cd $TMP_DIR
	ar x $BUILD_DIR/src/libwt.a &&
	ar -rcs $BUILD_DIR/Wt.a *.o */*.o)  || abort "Combine libs $1 failed."
    else
    ( cd $TMP_DIR
	ar -rcs $BUILD_DIR/Wt.a */*.o)  || abort "Combine libs $1 failed."
    fi
    rm -rf $TMP_DIR
}

                    VERSION_TYPE=Alpha
                  FRAMEWORK_NAME=Wt
               FRAMEWORK_VERSION=A

       FRAMEWORK_CURRENT_VERSION=$WT_VERSION
 FRAMEWORK_COMPATIBILITY_VERSION=$WT_VERSION

createFramework()
{
    FRAMEWORK_BUNDLE=$FRAMEWORKDIR/$FRAMEWORK_NAME.framework

    rm -rf $FRAMEWORK_BUNDLE

    mkdir -p $FRAMEWORK_BUNDLE
    mkdir -p $FRAMEWORK_BUNDLE/Versions
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Resources
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Headers
    mkdir -p $FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/Documentation

    ln -s $FRAMEWORK_VERSION               $FRAMEWORK_BUNDLE/Versions/Current
    ln -s Versions/Current/Headers         $FRAMEWORK_BUNDLE/Headers
    ln -s Versions/Current/Resources       $FRAMEWORK_BUNDLE/Resources
    ln -s Versions/Current/Documentation   $FRAMEWORK_BUNDLE/Documentation
    ln -s Versions/Current/$FRAMEWORK_NAME $FRAMEWORK_BUNDLE/$FRAMEWORK_NAME

    FRAMEWORK_INSTALL_NAME=$FRAMEWORK_BUNDLE/Versions/$FRAMEWORK_VERSION/$FRAMEWORK_NAME

    echo "Framework: Creating final library..."
    lipo -create \
	-arch armv7 build-armv7/Wt.a \
	-arch i386 build-i386/Wt.a \
	-arch x86_64 build-x8664/Wt.a \
	-o $FRAMEWORK_INSTALL_NAME || abort "lipo failed"

    echo "Framework: Copying headers..."
    cp -r stage/include/Wt/* $FRAMEWORK_BUNDLE/Headers/

    echo "Framework: Creating plist..."
    cat > $FRAMEWORK_BUNDLE/Resources/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>${FRAMEWORK_NAME}</string>
	<key>CFBundleIdentifier</key>
	<string>org.boost</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundlePackageType</key>
	<string>FMWK</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>${FRAMEWORK_CURRENT_VERSION}</string>
</dict>
</plist>
EOF
}

clean
#build-armv6
build-armv7
build-i386
build-x8664
#combineLibs $BUILD_ARMV6_DIR
combineLibs $BUILD_ARMV7_DIR
combineLibs $BUILD_I386_DIR
combineLibs $BUILD_X8664_DIR
createFramework
