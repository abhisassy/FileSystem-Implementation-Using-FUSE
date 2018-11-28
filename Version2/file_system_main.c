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
	
	fs_file = open("MyFileSystem",O_RDWR | O_CREAT,0700); //stored values in ext file
	if(fs_file < 0){
		printf("ERROR while opening filesystem!!!");
		printf("\033[0m");
		exit(0);
	}
	
	struct stat buf;
	fstat(fs_file, &buf);
	printf("\nFile System Capacity = %d Bytes\n", FS_SIZE);
	
	fs = calloc(1, FS_SIZE);	// allocate and initialize the total memory needed

	if(buf.st_size != 0){
		char choice;
		printf("\n");
		printf("Previous File System Mount Detected\n");
		printf("DO YOU WANT TO RESTORE ? (y/n) ");
		printf("\033[1;32m");
		fflush(stdin);
		scanf("%c",&choice);
		printf("\033[0m");
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
	data_bitmap  = (int *)(inodes + INODE_BLKS * BLK_SIZE);
	datablks   	 = (char *)(data_bitmap + DATA_BITMAP_BLKS * BLK_SIZE);

	// filling meta data of filesystem 
	superblk -> filesystem_capacity = FS_SIZE;
    strcpy(superblk -> filesystem_name , "FUSE DRIVE");
    superblk -> no_of_blocks = 1 + INODE_BITMAP_BLKS + INODE_BLKS + DATA_BITMAP_BLKS+ DBLKS;
    superblk -> no_of_inodes = N_INODES;
    superblk -> no_of_datablocks = DBLKS;
    //

	
	printf("\n\tBLOCK LOCATIONS & SIZE\n");
	printf("\n");
	printf("Super Block\t=\t %p\t%dbytes\n",superblk,SBLK_SIZE );
	printf("Inode Bitmap\t=\t %p\t%dbytes\n", inode_bitmap, BLK_SIZE);
	printf("Inode Block\t=\t %p\t%dbytes\n", inodes,BLK_SIZE);
	printf("Data  Bitmap\t=\t %p\t%dbytes\n", data_bitmap,BLK_SIZE);
	printf("Data  Block\t=\t %p\t%dbytes\n", datablks,BLK_SIZE*100);

	
	

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
		
		clock_gettime(CLOCK_REALTIME, &(temp -> ta));
		clock_gettime(CLOCK_REALTIME, &(temp -> tm));
		clock_gettime(CLOCK_REALTIME, &(temp -> tc));

		temp -> link_count = 1;
		
		char *data_temp = (datablks + ((temp -> data)*BLK_SIZE));
		strcpy(data_temp, "\t\tWelcome To Our File System!!!\n\n");
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
		printf("Help File Created\n");
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
	

	return fuse_main(argc, argv, &user_defined_func, NULL);
	
}