CC=clang++
main:
	$(CC) main.cpp ico.o -o gfimage.exe -lfltk -lfltk_gl -lopengl32 -mwindows