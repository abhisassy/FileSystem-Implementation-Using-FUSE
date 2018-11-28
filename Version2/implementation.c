/*	Implementation file for IOS Project
	All function definitions 		*/


//helper functions used in file_system_main.c


int openDisk(char* filename , int nbytes){

	int file = fopen("MyFileSystem","ab+");
	if(file == NULL){
		printf("ERROR while opening filesystem!!!\n");
		exit(0);
	}
	return file;
}

int readBlock(int disk, int blockn, void* block){

	if(blockn > 100){
		printf("Memeory out of bounds!! \n");
		exit(0);
	}

	int file = fread(block,BLK_SIZE,1,disk+BLK_SIZE*(blockn-1));
	if(file < 1){
		printf("ERROR while reading filesystem!!!\n");
		exit(0);
	}
	else return file;
}

int writeBlock(int disk, int blocknr, void* block){

	if(blocknr > 100){
		printf("Memeory out of bounds!! \n");
		exit(0);
	}

	int n = fwrite(block,1,sizeof(block),disk);

	return n;

}



int initialise_inodes(int* i){					//initialize all to 0 indicating free
	printf("\n");
	printf("Initialising Inode Bitmap..\n");
	printf("\n");
	for(int x = 0; x < N_INODES; x++){
		*i = 0;
		i++;
	}
	return 0;
}

int initialise_data_bitmap(int* map){			//initialize all to 1 indicating unused
	
	printf("Initialising Data Bitmap..\n");
	printf("\n");
	int x;
	for( x = 0; x < DBLKS; x++){
		*(map + x) = 1;
	}
	*(map + x) = -1; // No more datablocks
	return 0;
}

int return_first_unused_inode(int* i){			//return offset of first available inode
	
	printf("Finding free inode \n");
	
	for(int ix = 1; ix <= N_INODES; ix++,i++){
		if(*i == 0){
			*i = 1;
			return ix;
		}
	}
	return -1;
}

int return_offset_of_first_free_datablock(int *data_bitmap){	//self explainatory
	
	printf("Finding free data block\n");

	for(int i = 1; i <= DBLKS; i++){
		if(data_bitmap[i-1] == 1){
			data_bitmap[i-1] = 0; // no more free
			return (i);
		}
	}
	return -1;
}

/*
	helper functions for fuse functions
										*/

void path_to_inode(const char* path, int *ino){  //stores inode offset in ino
	printf("path_to_inode \n");
	
	if(strcmp("/", path) == 0){
		*ino = (root_directory->file_inode);
	}
	else{
		char *token = strtok(path, "/");
		dirent *temp;
		temp = root_directory;
		while (token != NULL){
			
			while((strcmp(temp -> filename, "") != 0) && (strcmp(temp -> filename, token) != 0)){
				temp++;
			}

			if(strcmp(temp -> filename, "") == 0){
				printf("Inode doesnt exist!!\n" );
				*ino = -1;
			}
			else{
				*ino = (temp -> file_inode);
        		inode *temp_ino = inodes + ((*ino) * sizeof(inode));
				if(temp_ino -> directory){
					temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
				}
			}
			token = strtok(NULL, "/");
		}
	}
	printf("OUT\n");
}

int find_link_count(char* path){ // returns no of files in direcotry
	printf("Finding link count\n");
	int count = 2;
	int ino;
	dirent *temp;
  	path_to_inode(path, &ino);
  	if(strcmp(path, "/") == 0){
  			temp = root_directory;
  		}
  	else{
        	inode *temp_ino = inodes + (ino * sizeof(inode));
  			temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
  		}
  	while((strcmp(temp -> filename, "") != 0)){
  			count++;
  			temp++;
  		}
  	return count;

}
int isDir(char *path){	// checks if path given is a directory 
	
	printf("Is Directory Called\n");
	if (strcmp("/", path) == 0){
		return 1;
	}
	int ino;
	//dirent *temp;
  	path_to_inode(path, &ino);
  	if(ino == -1){
  		printf("Invalid path\n");
  		return -1;
  	}
	inode *temp = inodes + (ino * sizeof(inode));
	if(temp->directory)
		return 1;
	else 
		return 0;
	
}
void allocate_inode(char *path, int *ino, bool dir){ // initiliazes inode struct values 
	
	printf("ALLOCATING INODE\n");
	
  	inode *temp_ino 	  = inodes + ((*ino) * sizeof(inode));
	temp_ino -> id 		  = ino;
	temp_ino -> size      = 0;
	temp_ino -> data 	  = return_offset_of_first_free_datablock(data_bitmap);
	temp_ino -> directory = dir;
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));
	if(dir){
		temp_ino -> link_count = 2;
	}
	else{
		temp_ino -> link_count = 1;
 	}
}

void print_inode(inode *i){ // prints inode struct values for a given file
	printf("used : %d\n", i -> used);
	printf("id : %d\n", i -> id);
	printf("size : %d\n", i -> size);
	printf("data : %u\n", i -> data);
	printf("directory : %d\n", i -> directory);
	printf("last_accessed : %d\n", i -> ta);
	printf("last_modified : %d\n", i -> tm);
	printf("last_status_change : %d\n", i -> tc);
	printf("link_count : %d\n", i -> link_count);
}

/* 
 
 function definition for fuse functions 

*/

static int fs_getattr(const char *path, struct stat *stbuf,struct fuse_file_info *fi){

  	printf("\nGet Attribute called\n");
  	printf("--- %s\n",path );

  	int res = 0;
  	char *path2;
  	path2 = (char *)malloc(sizeof(path));
  	strcpy(path2, path);
  	memset(stbuf, 0, sizeof(struct stat));

  	int ino;
  	path_to_inode(path, &ino);
  	inode *temp_ino = inodes + (ino * sizeof(inode));

  	stbuf->st_ino = ino;
  	stbuf->st_blksize = 4096;
  	stbuf->st_blocks  = temp_ino -> nblocks;

  	

  	int directory_flag = isDir(path2);
  	if (directory_flag > 0) {

  		stbuf->st_mode  = S_IFDIR | 0770;
  		stbuf->st_nlink = temp_ino -> link_count;
  		stbuf->st_size  = temp_ino -> size; 
  	}
  	else if (directory_flag == 0){
      	
  		stbuf->st_mode  = S_IFREG | 0770;
  		stbuf->st_nlink = 1;
  		stbuf->st_size  = temp_ino -> size;
  	}
  	else{
  		res = -ENOENT; // no suc file ordirecory error
  	}
  	
  	
  	stbuf->st_atim = temp_ino -> ta;
  	stbuf->st_mtim = temp_ino -> tm;
  	stbuf->st_ctim = temp_ino -> tm;

  	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();

  	return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi){

	
  	printf("\nReadDirectory called\n");
  
  	int ino;
  	path_to_inode(path, &ino);

  	if (ino == -1){
  		return -ENOENT;
  	}
  	else{
  		dirent *temp;
  		filler(buf, ".", NULL, 0);
  		filler(buf, "..", NULL, 0);

  		inode *temp_ino = inodes + (ino * sizeof(inode));
  		temp_ino -> size = 0;

  		clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));

  		if(strcmp(path, "/") == 0){
  			temp = root_directory;
  		}
  		else{
  			temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
  		}
  		while((strcmp(temp -> filename, "") != 0)){
  			if(strcmp(temp -> filename ,"$") == 0)
  				temp++;
  			else
  			{	
  				inode *tempvar = inodes + (temp -> file_inode * sizeof(inode));
  				temp_ino -> size = temp_ino -> size + tempvar -> size;
  				printf("%d\t %d\n",tempvar -> size , temp_ino ->size );
  				printf("%s\n",temp -> filename );
  				filler(buf, temp -> filename, NULL, 0);
  				temp++;
  			}

  		}
  	}
  	return 0;
}

static int fs_mkdir(const char *path, mode_t mode){
	
	printf("\nMake Directory called\n");

	int ino = return_first_unused_inode(inode_bitmap);
	
	allocate_inode(path, &ino, true);
	inode *temp_ino = inodes + (ino * sizeof(inodes));

	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));

	char *token = strtok(path, "/");
	dirent *temp;
	temp = root_directory;
	while(token != NULL){
		while((strcmp(temp -> filename, "") != 0) && (strcmp((temp -> filename), token) != 0)){
			temp++;
		}
		if((strcmp(temp -> filename, "") == 0)){
			temp = (dirent *)temp;
			printf("%s\n",token );
			if(strcmp(token,"$") == 0)
				return -1;
			strcpy((temp -> filename), token);
			temp -> file_inode = ino;
			temp_ino -> link_count = temp_ino -> link_count + 1;

    		printf("\n\nPersisting the directory creation\n\n");
    		lseek(fs_file, 0, SEEK_SET);
    		write(fs_file, fs, FS_SIZE);
			return 0;
		}
		else{
    		temp_ino = (inodes + ((temp -> file_inode) * sizeof(inode)));
			if(temp_ino -> directory){
				temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
			}
		}
		token = strtok(NULL, "/");
	}
	return -1;
}

static int fs_rmdir(const char *path){
	/*
	  remove a directory only if the directory is empty
	 */
	
	printf("\nRemove directory\n");	

	// Get the last directory
	int i = strlen(path);
	while(path[i] != '/'){
		i--;
	}
	char *subpath;
	subpath = (char *)malloc(strlen(path)+ 1);
	strncpy(subpath, path, i);
	subpath[i] = '/';
	subpath[i+1] = '\0';

	int ino;
	dirent *temp;
	dirent *temp_data;
	inode *temp_ino;

	path_to_inode(subpath, &ino);

	temp_ino = inodes + (ino * sizeof(inode));

	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));

	temp_ino -> link_count = temp_ino -> link_count - 1;


	if(strcmp(subpath, "/") == 0){
	temp = root_directory;
	}
	else{

	temp = datablks + ((temp_ino->data) * BLK_SIZE);
	}

	while(((strcmp(temp -> filename, "") != 0) || (temp -> file_inode) != 0) && strcmp(path + (i+1), temp -> filename) != 0){
		temp ++;
	}
	if((strcmp(temp -> filename, "") != 0)){
		
		if(temp -> file_inode != 0){
			temp_ino = inodes + ((temp -> file_inode) * sizeof(inode));
			temp_data = ((dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE)));
			if(strcmp((temp_data -> filename), "") != 0 && strcmp((temp_data -> filename), "$") != 0){
		
				return -1;
			}
			strcpy(temp ->filename, "$");
			data_bitmap[(temp_ino -> data)] = 1;
			inode_bitmap[(temp -> file_inode)] = 0;
		}
	}
	printf("\n\nPersisting the directory deletion\n\n");
	lseek(fs_file, 0, SEEK_SET);
	write(fs_file, fs, FS_SIZE);
	return 0;
}


static int fs_create(const char *path, mode_t mode,struct fuse_file_info *fi){
	printf("\nCreate called\n");
	

	int ino = return_first_unused_inode(inode_bitmap);
	allocate_inode(path, &ino, false);

	char *token = strtok(path, "/");
	dirent *temp;
	temp = root_directory;
	char *file = (char *)malloc(12);
	while (token != NULL){
		while((strcmp(temp -> filename, "") != 0) && (strcmp(temp -> filename, token) != 0)){
			temp++;
		}
		if((strcmp(temp -> filename, "") != 0)){
      		inode *temp_ino = inodes + ((temp -> file_inode) * sizeof(inode));
			if(temp_ino -> directory){
					temp = datablks + ((temp_ino -> data) * BLK_SIZE);
				}
			else{
				return -1;
				}
		}
		strcpy(file, token);
		token = strtok(NULL, "/");
	}
	strcpy(temp -> filename, file);
	temp -> file_inode = ino;
	printf("\n\nPersisting the create!!\n\n");
	lseek(fs_file, 0, SEEK_SET);
	write(fs_file, fs, FS_SIZE);
	
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi){
	
	printf("\nOpening File\n");
	int ino;
	path_to_inode(path, &ino);

	if(ino == -1){
		return -1;
	}

	inode *temp_ino = inodes + (ino * sizeof(inode));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));

	printf("Successful open\n");
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
	
	printf("\nRead called\n");

	int ino;
	path_to_inode(path, &ino);
	size_t len;
	
	if(ino == -1)
		return -ENOENT;

	inode *temp_ino = (inodes + (ino * sizeof(inode)));

	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));

	len = temp_ino->size;
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
 	char *temp_data = datablks + ((temp_ino -> data) * BLK_SIZE);
		memcpy(buf, temp_data + offset, size);
	} 
	else
		size = 0;

	return size;
}

static int fs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi){
	
	printf("\nWrite called\n");

	int ino;
	path_to_inode(path, &ino);

	printf("out of path to inode \n");
	inode *temp_ino = inodes + (ino * sizeof(inode));

	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));

	if(temp_ino -> size + size > BLK_SIZE)
		return -1;

	memcpy(((datablks + ((temp_ino -> data) * BLK_SIZE)) + offset), (buf), size);
	temp_ino -> size = (temp_ino -> size) +  size;
	
	
	if(temp_ino -> size / 512 + 1 > temp_ino -> nblocks)
		temp_ino -> nblocks = temp_ino -> size / 512 + 1;
	else if(temp_ino -> size ==0 )
		temp_ino -> nblocks = 1;
	

	printf("\n\nPersisting the write\n\n");
	lseek(fs_file, 0, SEEK_SET);
	printf("%d\n", write(fs_file, fs, FS_SIZE));
	
	return size;

}

static int fs_rm(const char *path){
	
	printf("\nremove/delete called\n");

	int i = strlen(path);
	while(path[i] != '/'){
		i--;
	}
	char *subpath;
	subpath = (char *)malloc(strlen(path)+ 1);
	strncpy(subpath, path, i);
	subpath[i] = '/';
	subpath[i+1] = '\0';


	int ino;
	dirent *temp;
	dirent *temp_data;
	inode  *temp_ino;

	path_to_inode(subpath, &ino);
	temp_ino = inodes + (ino * sizeof(inode));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));


	if(strcmp(subpath, "/") == 0){
  		temp = root_directory;
	}	
	else{
  		
		temp = datablks + ((temp_ino->data) * BLK_SIZE);
	}

	
	while(((strcmp(temp -> filename, "") != 0) || (temp -> file_inode) != 0) && strcmp(path + (i+1), temp -> filename) != 0){
		temp ++;
	}
	if((strcmp(temp -> filename, "") != 0)){
  		
		if(temp -> file_inode != 0){
    	
		strcpy(temp ->filename, "$");
		
		inode_bitmap[(temp -> file_inode)] = 0;
		}
	}
	printf("\n\nPersisting the file deletion\n\n");
	lseek(fs_file, 0, SEEK_SET);
	write(fs_file, fs, FS_SIZE);
	return 0;
}

static int fs_truncate (const char *path, off_t length, struct fuse_file_info *fi){
	printf("TRUNC CALLED\n");
	int ino;
	path_to_inode(path, &ino);
	
	inode *temp_ino = inodes + (ino * sizeof(inode));

	clock_gettime(CLOCK_REALTIME, &(temp_ino -> ta));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tm));
	clock_gettime(CLOCK_REALTIME, &(temp_ino -> tc));

	if(temp_ino -> size > length){
		printf("\t\t\t\t\t>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		memset((datablks + ((temp_ino -> data) * BLK_SIZE)) + length, "\0", temp_ino -> size - length);
		temp_ino -> size = length;
	}
	
	return 0;


}