
                     =================================
                       PS2CLIENT - NOTES FOR MINGW32
                     =================================

 ----------------------------------
  BUILDING PS2CLIENT USING MINGW32
 ----------------------------------

  Since now, ps2client uses the pthread library to work, you need the
  pthreads-win32 native library to compile ps2client using a mingw32
  compiler. You want to download the prebuilt binaries and headers
  from ftp://sources.redhat.com/pub/pthreads-win32 and put them
  in the ../pthreads-win32 directory. You need the following files:

    libpthreadGC2.a
    pthread.h
    pthreadGC2.dll
    sched.h
    semaphore.h

  Note that if you're using a linux host to cross compile, you probably
  want to dos2unix *.h before compiling.

 ------------------
  MORE INFORMATION
 ------------------

  If you want to get into the wonderful world of homebrew ps2
  development, there's a few places you can check out:

  http://www.ps2dev.org     - News, tutorials and documentation.

  http://forums.ps2dev.org  - The official ps2dev forums.

  #ps2dev on efnet (IRC)    - Come banter in realtime.
