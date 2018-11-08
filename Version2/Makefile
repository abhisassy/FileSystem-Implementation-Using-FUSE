run: file_system_main.c
	gcc -D_FILE_OFFSET_BITS=64 file_system_main.c `pkg-config fuse --cflags --libs` -o filesystem -w

clean:
	rm *.o