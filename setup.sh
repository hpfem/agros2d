#/bin/sh

case "$1" in
--clang)  echo "Clang"
    rm -rf CMakeFiles
    rm -f CMakeCache.txt

    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++

    cmake .
    ;;
*) echo "Unknown option"
   ;;
esac


