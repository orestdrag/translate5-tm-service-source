#!/bin/sh

BUILD_TYPE=Debug
CORES=8

do_package() {
  do_build
  echo "Packaging..."
  cpack -G TXZ -C ${BUILD_TYPE}
}

do_install() {
  echo "Installing..."
  make install -j${CORES}
}

do_clean() {
  echo "Cleaning..."
  for file in *; do
    if [ "$file" != "build.sh"  AND "$file" != "vs_build.sh"]; then
      rm -rf $file
    fi
  done
}

do_build() {
  echo "Building..."
  cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
  make -j${CORES}
}

do_main() {
  if [ "$1" = "clean" ]; then
    do_clean
  elif [ "$1" = "install" ]; then
	do_build
    do_install
  elif [ "$1" = "package" ]; then
	do_build
    do_package
  elif [ "$1" = "rebuild" ]; then
    do_clean
    do_build
    #do_package
  elif [ "$1" = "build"   ]; then
	do_build
  else
	do_build
    #do_package
  fi
}

do_main "$*"
