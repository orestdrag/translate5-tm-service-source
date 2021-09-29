#!/bin/sh

BUILD_TYPE=Debug
CORES=8

do_install() {
  echo "Installing..."
  make install -j${CORES}
}

do_clean() {
  echo "Cleaning..."
  for file in *; do
    if [ "$file" != "build.sh" ]; then
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
    do_install
  else
    do_build
  fi
}

do_main "$*"
