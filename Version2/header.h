/*
	Header file for IOS Project.
*/
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> 	//for posix os api related functions 
#include <fcntl.h>		//for file discriptor related functions
#include <sys/stat.h> 	//file stats 		
#include <errno.h>
#include <sys/time.h>
#include <time.h>


// our file system implementation function prototype for FUSE

static void *fs_init(struct fuse_conn_info *conn,struct fuse_config *cfg){
   
    printf("File System Initializing...\n");
    return NULL;
}
static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int fs_mkdir(const char *path, mode_t mode);
static int fs_rmdir(const char *path);
static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int fs_open(const char *path, struct fuse_file_info *fi);
static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int fs_rm(const char *path);
static int fs_truncate (const char *path, off_t length, struct fuse_file_info *fi);



static struct fuse_operations user_defined_func={
	.init       = fs_init,
	.getattr    = fs_getattr,
	.readdir	= fs_readdir,
	.mkdir		= fs_mkdir,
	.rmdir		= fs_rmdir,
	.open       = fs_open,
	.create     = fs_create,
	.read       = fs_read,
	.write      = fs_write,
	.unlink		= fs_rm,
	.truncate	= fs_truncate,
};

// our user defined file system data structures

//super block
typedef struct{
    
    size_t  filesystem_capacity;
    char 	filesystem_name[30];
    int 	no_of_blocks;
    int 	no_of_inodes;
    int 	no_of_datablocks;
    
}sblock;

//inode block
typedef struct {
	bool used;                  // valid inode or not
	int id;						// ID for the inode
	size_t size;				// Size of the file
	int nblocks;				// number 512 byte blocks used 
	int data;					// offset of data block
	bool directory;				// true if its a directory else false
	int link_count; 			// 2 in case its a directory, 1 if its a file
	struct timespec ta;			// Last accessed time
	struct timespec tm;			// Last modified time
	struct timespec tc;			// Last change of state time

}inode;

//directory
typedef struct{
	char filename[12]; 
	int file_inode;
}dirent;

/*
	size of dirent = 12+4 =16bytes ;  4kb block :: hence around 200 directory entries
	sizeof inode = 66bytes :: hence 4kb/66b = 62 files max 
*/

// block size specifications

#define SBLK_SIZE 		  50	//super block size
#define BLK_SIZE 		  4096  //4KB data blocks

#define N_INODES 		  60	//62 files limit  
#define DBLKS_PER_INODE   1		//each file gets one data block
#define DBLKS 			  100 	//100 data blocks available	

#define DATA_BITMAP_BLKS  1		//1 block for databitmap 
#define INODE_BLKS 		  1		//1 block for inodes
#define INODE_BITMAP_BLKS 1		//1 block for inodebitmap

#define FS_SIZE SBLK_SIZE + (INODE_BITMAP_BLKS + INODE_BLKS + DATA_BITMAP_BLKS+ DBLKS) * BLK_SIZE  //file system size

// other data and functions used 

int   initialise_inodes(int* i);
int   initialise_data_bitmap(int* map);
int   return_first_unused_inode(int* i);
int   return_offset_of_first_free_datablock(int *data_bitmap);
int   isDir(char *path);
void  path_to_inode(const char* path, int *ino);
void  allocate_inode(char *path, int *ino, bool dir);
void  print_inode(inode *i);
int   find_link_count(char* path);

int 	fs_file;		//file pointer to drive(large file)
char 	*fs;			//start of file system in memeory
sblock  *superblk;		//points to super block
int 	*inode_bitmap;	//points to inode bitmap block
inode 	*inodes;		//points to inode block
int 	*data_bitmap;	//points to data bitmap block	
char 	*datablks;		//points to data block
dirent 	*root_directory;//points to the direct structure of root in datablock 