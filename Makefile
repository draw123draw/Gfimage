CC=clang++
ifeq ($(OS),Windows_NT)
SOURCE=main.cpp ico/ico.o
WORD_FLAGS=-lfltk -lfltk_gl -lopengl32 -DUNICODE -D_UNICODE -D_FILE_OFFSET_BITS=64 -mwindows
EXE=Gfimage.exe
else ifeq ($(shell uname),Linux)
SOURCE=main.cpp
WORD_FLAGS=-lfltk -lGL -lfltk_gl -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT -O2
EXE=Gfimage
endif
main:
	$(CC) $(SOURCE) -o $(EXE) $(WORD_FLAGS)
