#!/bin/sh

set -e

readonly version="4.0.7"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="dba36a8e7898e02c3a66dbe169dc776ffc81c2ee6d5bcab3c526d3f3976b4e91"
        platform="linux-x86_64"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="ad77fb21981b774f5d33b5ec0987071d93fee1d080599f7f61e59545f4cb0b50"
        platform="linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="dba00f8ad22a832dad0f6754e62384780684a4c9d127de472fb0e92df719f1c4"
        platform="macos-universal"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
$shatool --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" "cmake$CMAKE_SUFFIX"
rm "$tarball" cmake.sha256sum

if [ "$( uname -s )" = "Darwin" ]; then
    ln -s CMake.app/Contents/bin "cmake$CMAKE_SUFFIX/bin"
fi
