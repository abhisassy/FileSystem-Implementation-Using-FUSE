/*
	IOS Project..
	Implementation of a Simple File System

	Abhishek Sinha 		01FB16ECS014
	Aditya Pandey		01FB16ECS029
	Abhishek Saseendran 01FB16ECS018

gcc -D_FILE_OFFSET_BITS=64 file_system_main.c `pkg-config fuse --cflags --libs` -o filesystem -w

*/


#include "header.h"
#include "implementation.c"

int main(int argc, char* argv[]){
	
	fs_file = open("MyFileSystem",O_RDWR); //stored values in ext file
	
	struct stat buf;
	fstat(fs_file, &buf);
	printf("File System size = %d\n", buf.st_size);
	
	fs = calloc(1, FS_SIZE);

	if(buf.st_size != 0){
		char choice;
		printf("\n");
		printf("Previous File System Detected\n");
		printf("DO YOU WANT TO RESTORE ? (y/n)\n");
		fflush(stdin);
		scanf("%c",&choice);
		if(choice=='y')
			read(fs_file, fs, FS_SIZE);
		else
			{
				fopen("MyFileSystem","w"); // cleans file 
				fs_file = open("MyFileSystem",O_RDWR);
			}
	}
	
	fstat(fs_file, &buf);
	inode_map = (int *)fs;
	inodes = (inode *)(inode_map + INODE_BLKS * BLK_SIZE);
	freemap = (int *)(inodes + N_INODES * sizeof(inode));
	datablks = (char *)(freemap + FREEMAP_BLKS * BLK_SIZE);
	printf("fs = %p\n", fs);
	printf("inode_map = %p\n", inode_map);
	printf("inodes = %p\n", inodes);
	printf("freemap = %p\n", freemap);
	printf("datablks = %p\n", datablks);

	if(buf.st_size == 0){
		initialise_inodes(inodes);
		initialise_freemap(freemap);
	}

	root_directory = (dirent *)datablks; // Initialising the root directory
	sleep(1);
	printf("\n");
	printf("....initilalization complete....\n\n");
	sleep(1);
	printf("root_directory = %p\n", root_directory);
	sleep(1);
	if(buf.st_size == 0){
		strcpy(root_directory -> filename, "RootFile");
		root_directory -> file_inode = return_first_unused_inode(inode_map);

		inode *temp;
		temp = inodes + ((root_directory -> file_inode) * sizeof(inode));
		temp -> id = 1;
		temp -> size = 30;
		temp -> data = return_offset_of_first_free_datablock(freemap);
		temp -> directory = false;
		temp -> last_accessed = 0;
		temp -> last_modified = 0;
		temp -> link_count = 1;

		
		char *data_temp = (datablks + ((temp -> data)*BLK_SIZE));
		strcpy(data_temp, "Welcome To Our File System!!!\n");
		// write the filesystem data onto ext file for persistence 
		write(fs_file, fs, FS_SIZE); 
	}
	else{
		printf("\n");
		printf("FileSystem successfully restored from previous mount!!! \n");
		sleep(2);
	}

	return fuse_main(argc, argv, &fs_oper, NULL);
}