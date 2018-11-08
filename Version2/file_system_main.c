/*
	IOS Project..
	Implementation of a File System using FUSE

	Abhishek Sinha 		01FB16ECS014
	Aditya Pandey		01FB16ECS029
	Abhishek Saseendran 01FB16ECS018

gcc -D_FILE_OFFSET_BITS=64 file_system_main.c `pkg-config fuse --cflags --libs` -o filesystem -w

*/


#include "header.h"
#include "implementation.c"

int main(int argc, char* argv[]){

	printf("\033[1;34m");
	time_t t; 
	
	fs_file = open("MyFileSystem",O_RDWR | O_CREAT,0700); //stored values in ext file
	if(fs_file < 0){
		printf("ERROR while opening filesystem!!!");
		printf("\033[0m");
		exit(0);
	}
	
	struct stat buf;
	fstat(fs_file, &buf);
	printf("File System size = %d Bytes\n", buf.st_size);
	
	fs = calloc(1, FS_SIZE);

	if(buf.st_size != 0){
		char choice;
		printf("\n");
		printf("Previous File System Mount Detected\n");
		printf("DO YOU WANT TO RESTORE ? (y/n) ");
		fflush(stdin);
		scanf("%c",&choice);
		if(choice=='y')
			read(fs_file, fs, FS_SIZE);
		else
			{				
				fs_file = open("MyFileSystem",O_RDWR | O_TRUNC); // cleans file 
			}
	}
	
	fstat(fs_file, &buf);
	superblk     = (sblock *)fs;
	inode_bitmap = (int *)(superblk + SBLK_SIZE); 
	inodes 	  	 = (inode *)(inode_bitmap + INODE_BITMAP_BLKS * BLK_SIZE);
	data_bitmap  = (int *)(inodes + N_INODES * sizeof(inode));
	datablks   	 = (char *)(data_bitmap + DATA_BITMAP_BLKS * BLK_SIZE);
	
	printf("\n\tBLOCK LOCATIONS\n");
	printf("\n");
	printf("Super Block\t=\t %p\n",superblk );
	printf("Inode Bitmap\t=\t %p\n", inode_bitmap);
	printf("Inode Block\t=\t %p\n", inodes);
	printf("Data Bitmap\t=\t %p\n", data_bitmap);
	printf("Data Block\t=\t %p\n", datablks);

	if(buf.st_size == 0){
		initialise_inodes(inode_bitmap);
		initialise_data_bitmap(data_bitmap);
	}

	root_directory = (dirent *)datablks; // Initialising the root directory to firt data block
	
	sleep(1);
	printf("\n");
	printf("\033[1;32m");
	printf("\t      Initilalization Complete\n\n");
	printf("\033[1;34m");
	printf("root_directory\t=\t %p\n", root_directory);
	sleep(1);
	if(buf.st_size == 0){
		strcpy(root_directory -> filename, "Help");
		root_directory -> file_inode = return_first_unused_inode(inode_bitmap);

		inode *temp;
		temp = inodes + ((root_directory -> file_inode) * sizeof(inode));
		temp -> id = 1;
		temp -> size = 300;
		temp -> data = return_offset_of_first_free_datablock(data_bitmap);
		temp -> directory = false;
		t = time(0);
		temp -> last_accessed = localtime(&t);
		temp -> last_modified = localtime(&t);
		temp -> link_count = 1;
		
		char *data_temp = (datablks + ((temp -> data)*BLK_SIZE));
		strcpy(data_temp, "Welcome To Our File System!!!\n");
		strcat(data_temp, "Here are some commands you can run :\n");
		strcat(data_temp, "ls , ls -a, ls -l \n");
		strcat(data_temp, "cd <file/folder>, cd . , cd ..\n");
		strcat(data_temp, "echo <string>  >> <filename>\n");
		strcat(data_temp, "cat <filename>\n");
		strcat(data_temp, "touch <filename>\n");
		strcat(data_temp, "mkdir <folder name>\n");
		strcat(data_temp, "rmdir <folder name>\n");
		strcat(data_temp, "rm <filename>\n");
		strcat(data_temp, "stat <file/folder>\n");
		// write the filesystem data onto ext file for persistence 
		write(fs_file, fs, FS_SIZE); 
	}
	else{
		printf("\n");
		printf("FileSystem successfully restored from previous mount!!! \n");
		//sleep(1);
	}

	printf("\n");
	printf("\n");
	printf("\033[1;32m");
	printf("\t\tFile System Ready. \n");
	printf("\n");
	printf("\t\t   Mount? (y/n)\n");
	printf("\t\t        ");
	char choice;// = getchar();
	scanf(" %c",&choice);
	if(choice == 'n'){
		printf("\033[0m");
		exit(0);
	}


	printf("\033[0m"); 
	printf("\n");
	

	return fuse_main(argc, argv, &fs_oper, NULL);
	
}