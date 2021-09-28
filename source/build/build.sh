#!/bin/sh

BUILD_TYPE=Debug
CORES=8

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
  else
    do_build
  fi
}

do_main "$*"
