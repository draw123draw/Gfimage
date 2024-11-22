CC=clang++
ifeq ($(OS),Windows_NT)
SOURCE=main.cpp ico/ico.o
WORD_FLAGS=-lfltk -lfltk_gl -lopengl32 -DUNICODE -D_UNICODE -D_FILE_OFFSET_BITS=64 -mwindows
EXE=Gfimage.exe
else ifeq ($(shell uname),Linux)
SOURCE=main.cpp
WORD_FLAGS=	-I/usr/include/cairo \
            -I/usr/include/libpng16 \
            -I/usr/include/freetype2 \
            -I/usr/include/pixman-1 \
            -I/usr/include/freetype2 \
            -I/usr/include/libpng16 \
            -I/usr/include/cairo \
            -I/usr/include/libpng16 \
            -I/usr/include/freetype2 \
            -I/usr/include/pixman-1 \
            -g -O2 -fno-omit-frame-pointer \
            -mno-omit-leaf-frame-pointer \
            -fstack-protector-strong \
            -fstack-clash-protection \
            -Wformat -Werror=format-security \
            -fcf-protection \
            -fdebug-prefix-map=/build/fltk1.3-XAIL3V/fltk1.3-1.3.8=/usr/src/fltk1.3-1.3.8-6.1build2 \
            -fvisibility-inlines-hidden \
            -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT \
            -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now -lfltk_gl -lfltk -lX11 -lGL
EXE=Gfimage
endif
main:
	$(CC) $(SOURCE) -o $(EXE) $(WORD_FLAGS)
