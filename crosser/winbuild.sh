#!/bin/sh

# winbuild.sh: Cross-compiling freeciv from linux to Windows using Crosser dllstack
#
# (c) 2008-2015 Marko Lindqvist
#
# This script is licensed under Gnu General Public License version 2 or later.
# See COPYING available from the same location you got this script.

# Version 1.3.4.2 (13-Aug-15), for freeciv branches S2_6 and later

#
# 1. Adjust hardcoded WINCCPATH + DLLSPATH below to point to dllstack installation directory in your system.
#    Here WINCCPATH is upper level directory, where actual dllstack directories are: "current32" being
#    win32 dllstack, and "current64" being win64 dllstack. In my system "current32" and "current64" are
#    symbolic links to dllstack version in use.
# 2. Take svn checkout of Freeciv code
# 3. Adjust hardcoded TOLUAPATH below to point to tolua program native to build system.
#    If you cannot get it otherwise, you can build it from freeciv sources:
#    3a) Make native freeciv build
#       3a1) Make subdirectory "natbuild" to checkout
#       3a2) cd natbuild
#       3a3) ../autogen && make
#    3b) Copy produced natbuild/dependencies/tolua-5.2/src/bin/tolua to safe place
# 4. Make subdirectory "build" to checkout
# 5. Place this script to "build" directory
# 6. cd build
# 7. ./winbuild.sh
#

if test "x$1" != x && test "x$1" != xwin32 && test "x$1" != xwin64 ; then
  echo "Usage: $0 [setup=win32] [modlist URL]"
  exit 1
fi

if test "x$1" = xwin64 ; then
  SETUP=win64
else
  SETUP=win32
fi

if test "x$2" != "x" ; then
  MLPARAM="--with-modlist=$2"
else
  MLPARAM=""
fi

WINCCPATH=/usr/i686-w64-mingw32/
TOLUAPATH=/bin

VERREV="$(../fc_version)"

if test x$SETUP = xwin64 ; then
  DLLSPATH=/usr/x86_64-w64-mingw32/
  TARGET=x86_64-w64-mingw32
  # sdl-client is built separately
  CLIENTS="gtk3"
  FCMP="gtk3,cli"
  VERREV="win64-$VERREV"
else
  DLLSPATH=/usr/i686-w64-mingw32/
  TARGET=i686-w64-mingw32
  # sdl-client is built separately
  CLIENTS="gtk2 gtk3"
  FCMP="gtk2,gtk3,cli"
  VERREV="win32-$VERREV"
fi

if grep "CROSSER_QT" $DLLSPATH/crosser.txt | grep yes > /dev/null
then
    CLIENTS="$CLIENTS,qt"
    FCMP="$FCMP,qt"
fi

export CC="$TARGET-gcc -static-libgcc -static-libstdc++"
export CXX="$TARGET-g++ -static-libgcc -static-libstdc++"

if ! test -d "$DLLSPATH" ; then
  echo "Dllstack directory \"$DLLSPATH\" not found! Maybe you need to adjust this script?" >&2
  exit 1
fi

if ! ../autogen.sh CPPFLAGS="-I${DLLSPATH}/include" CFLAGS="-Wno-error" PKG_CONFIG_LIBDIR="${DLLSPATH}/lib/pkgconfig" $MLPARAM --enable-sys-tolua-cmd="$TOLUAPATH" --with-magickwand="${DLLSPATH}/bin" --prefix="/" --disable-fcmp --without-ggz-server --without-ggz-client --without-ggz-dir --enable-debug --host=$TARGET --build=$(../bootstrap/config.guess) --with-libiconv-prefix=${DLLSPATH} --with-sdl-prefix=${DLLSPATH} --with-sqlite3-prefix=${DLLSPATH} --with-followtag="crosser" --enable-crosser --enable-ai-static=classic --disable-freeciv-manual --disable-server  --with-tinycthread
then
  echo "Sdl-client configure failed" >&2
  exit 1
fi

if ! make DESTDIR="$(pwd)/freeciv-${VERREV}" clean install
then
  echo "Sdl-client build failed" >&2
  exit 1
fi

if ! ../autogen.sh CPPFLAGS="-I${DLLSPATH}/include" CFLAGS="-Wno-error" PKG_CONFIG_LIBDIR="${DLLSPATH}/lib/pkgconfig" $MLPARAM --enable-sys-tolua-cmd="$TOLUAPATH" --with-magickwand="${DLLSPATH}/bin" --prefix="/" --enable-client=$CLIENTS --enable-fcmp=$FCMP --without-ggz-server --without-ggz-client --without-ggz-dir --enable-debug --host=$TARGET --build=$(../bootstrap/config.guess) --with-libiconv-prefix=${DLLSPATH} --with-sqlite3-prefix=${DLLSPATH} --with-followtag="crosser" --enable-crosser --enable-ai-static=classic --disable-freeciv-manual  --with-qt5-includes=${DLLSPATH}/include --with-qt5-libs=${DLLSPATH}/lib --with-tinycthread
then
  echo "Configure failed" >&2
  exit 1
fi

if ! make DESTDIR="$(pwd)/freeciv-${VERREV}" clean install
then
  echo "Build failed" >&2
  exit 1
fi

if ! 7z a -r freeciv-${VERREV}.7z freeciv-${VERREV}
then
  echo "7z failed" >&2
  exit 1
fi
