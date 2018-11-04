/*
Implementation file for IOS Project
All function definitions
*/

/*

helper functions used in file_system_main.c

*/

int initialise_inodes(int* i){
	printf("\n");
	printf("Initialising inodes..\n");
	printf("\n");
	for(int x = 0; x < N_INODES; x++){
		*i = 0;
		i++;
	}
	return 0;
}

int initialise_freemap(int* map){
	printf("\n");
	printf("Initialising freemap..\n");
	printf("\n");
	int x;
	for( x = 0; x < DBLKS; x++){
		*(map + x) = 1;
	}
	*(map + x) = -1; // No more datablocks
	return 0;
}

int return_first_unused_inode(int* i){
	
	for(int ix = 1; ix < N_INODES; ix++,i++){
		if(*i == 0){
			*i = 1;
			return ix;
		}
	}
	return -1;
}

int return_offset_of_first_free_datablock(int *freemap){
	for(int i = 1; i < DBLKS; i++){
		if(freemap[i] == 1){
			freemap[i] = 0; // no more free
			return (i);
		}
	}
	return -1;
}

/*

	helper functions for fuse functions

*/
void path_to_inode(const char* path, int *ino){
	// Given the path name it will return the inode address if it exists 
	// else it returns NULL
	printf("path_to_inode - path - %s\n", path);
	
	if(strcmp("/", path) == 0){
		*ino = (root_directory->file_inode);
	}
	else{
		char *token = strtok(path, "/");
		dirent *temp;
		temp = root_directory;
		printf("temp = %u\n", temp);
		printf("root_directory -> filename = %s\n", (root_directory) -> filename);
	  	printf("temp -> filename = %s\n", temp->filename);
    	while (token != NULL){
			printf("token = %s\n", token);
			while((strcmp(temp -> filename, "") != 0) && (strcmp(temp -> filename, token) != 0)){
        	//printf("I am here\n");
				temp++;
				printf("temp = %u\n", temp);
				printf("temp -> filename = %s\n", temp->filename);
			}
			if(strcmp(temp -> filename, "") == 0){
				printf("Inode doesnt exist!! for the path %s\n", path);
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
}

int isDir(char *path){
	//checks if it a valid directory or path
	printf("isDir - %s", path);
	if (strcmp("/", path) == 0){
		return 1;
	}
	else{
		char *token = strtok(path, "/");
		dirent *temp;
		temp = root_directory;
		int dir = -1;
    	while (token != NULL){
    		while(((strcmp(temp -> filename, "") != 0) || (temp -> file_inode) != 0)){
				if((strcmp(temp -> filename, "") != 0)){
					if((strcmp(temp -> filename, token) == 0)){
						break;
					}
				}
				temp ++;

			}
			if((strcmp(temp -> filename, "") == 0) && temp -> file_inode == 0){
				printf("Invalid Path - %s\n", path);
				return -1; // Invalid path	
			}
			else{
        inode *temp_ino = inodes + ((temp -> file_inode) * sizeof(inode));
				if(temp_ino -> directory){
					dir = 1;
				}
				else{
					dir = 0;
				}
				if(dir == 1){// jump to next directory address 
					temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
				}
			}
      token = strtok(NULL, "/");
  	}
		
		if(dir == 0){
			printf("%s - FILE\n", path);
		}
		else if(dir == 1){
			printf("%s - DIRECTORY\n", path);
		}
		else{
			printf("%s - ERROR PATH\n", path);
		}
  return dir;
	}
}


void allocate_inode(char *path, int *ino, bool dir){
	
	printf("ALLOCATING INODE\n");
	printf("inode address to allocate : %u\n", *ino);
	
  	inode *temp_ino = inodes + ((*ino) * sizeof(inode));
	temp_ino -> id = rand() % 5000;
	temp_ino -> size = 0;
	temp_ino -> data = return_offset_of_first_free_datablock(freemap);
	temp_ino -> directory = dir;
	temp_ino -> last_accessed = 0;
	temp_ino -> last_modified = 0;
	if(dir){
		temp_ino -> link_count = 2;
	}
	else{
		temp_ino -> link_count = 1;
 	}
}

void print_inode(inode *i){
	printf("used : %d\n", i -> used);
	printf("id : %d\n", i -> id);
	printf("size : %d\n", i -> size);
	printf("data : %u\n", i -> data);
	printf("directory : %d\n", i -> directory);
	printf("last_accessed : %d\n", i -> last_accessed);
	printf("last_modified : %d\n", i -> last_modified);
	printf("link_count : %d\n", i -> link_count);
}

/* 
 
 function definition for fuse functions 

*/

static int fs_getattr(const char *path, struct stat *stbuf,struct fuse_file_info *fi){

  	printf("getattribute - ");
  	printf("%s\n", path);

  	int res = 0;
  	char *path2;
  	path2 = (char *)malloc(sizeof(path));
  	strcpy(path2, path);
  	memset(stbuf, 0, sizeof(struct stat));

  	int ino;
  	path_to_inode(path, &ino);

  	//printf("before isDir path - %s\n", path2);
  	int directory_flag = isDir(path2);
  	if (directory_flag == 1) {
  		//printf("getattr - Directory\n");
  		stbuf->st_mode = S_IFDIR | 0777;
  		stbuf->st_nlink = 2;
  	}
  	else if (directory_flag == 0){
      inode *temp_ino = inodes + (ino * sizeof(inode));
  		stbuf->st_mode = S_IFREG | 0777;
  		stbuf->st_nlink = 1;
  		stbuf->st_size = temp_ino -> size;
  	}
  	else{
  		res = -ENOENT; // no suc file ordirecory error
  	}
  	//printf("return valueof getdir = %d\n", res);
  	return res;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi){

  	printf("ReadDir - %s\n", path);
  	(void) offset;
  	(void) fi;

  	int ino;
  	path_to_inode(path, &ino);

  	printf("ino returned for %s - %d\n", path, ino);

  	if (ino == -1){
  		return -ENOENT;
  	}
  	else{
  		dirent *temp;
  		filler(buf, ".", NULL, 0);
  		filler(buf, "..", NULL, 0);
  		//printf("filled some stuff\n");

  		if(strcmp(path, "/") == 0){
  			temp = root_directory;
  		}
  		else{
        	inode *temp_ino = inodes + (ino * sizeof(inode));
  			temp = (dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE));
  		}
  		while((strcmp(temp -> filename, "") != 0)){
  			filler(buf, temp -> filename, NULL, 0);
  			temp++;
  		}
  	}
  	return 0;
  }


static int fs_mkdir(const char *path, mode_t mode)
{
	printf("mkdir\n");
	printf("%s\n", path);
	printf("%d", mode);

	int ino = return_first_unused_inode(inode_map);
	
	allocate_inode(path, &ino, true);
	inode *temp_ino = inodes + (ino * sizeof(inodes));
	print_inode(temp_ino);
	
	//printf("inode address for %s - %u\n", path, ino);

	char *token = strtok(path, "/");
	dirent *temp;
	temp = root_directory;
	while(token != NULL){
		//printf("token = %s\n", token);
		//printf("temp -> filename = %s\n", temp->filename);
		while((strcmp(temp -> filename, "") != 0) && (strcmp((temp -> filename), token) != 0)){
			temp++;
		}
		if((strcmp(temp -> filename, "") == 0)){
			temp = (dirent *)temp;
			strcpy((temp -> filename), token);
			temp -> file_inode = ino;
			temp_ino->link_count++;
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
}

static int fs_rmdir(const char *path)
{
	/*
	  remove a directory only if the directory is empty
	 */

	
	printf("\trmdir\n");
	printf("path : %s\n", path);
	

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
	
	//printf("Subpath = %s\n", subpath);
	//printf("Name of directory to delete = %s\n", path + (i+1));
	

	int ino;
	dirent *temp;
	dirent *temp_data;
	inode *temp_ino;
	if(strcmp(subpath, "/") == 0){
	temp = root_directory;
	}
	else{
	path_to_inode(subpath, &ino);
	//printf("Inode for the path - %s - %d\n", path, ino);

	temp_ino = inodes + (ino * sizeof(inode));

	temp = datablks + ((temp_ino->data) * BLK_SIZE);
	}

	//printf("ino -> data = %u\n", temp);
	//printf("root_directory = %u\n", root_directory);
	while(((strcmp(temp -> filename, "") != 0) || (temp -> file_inode) != 0) && strcmp(path + (i+1), temp -> filename) != 0){
		temp ++;
	}
	if((strcmp(temp -> filename, "") != 0)){
		//printf("temp -> file_inode = %d\n", temp -> file_inode);
		if(temp -> file_inode != 0){
			temp_ino = inodes + ((temp -> file_inode) * sizeof(inode));
			temp_data = ((dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE)));
			if(strcmp((temp_data -> filename), "") != 0){
				//printf("Directoy isnt empty!!\n");
				return -1;
			}
			strcpy(temp ->filename, "");
			freemap[(temp_ino -> data)] = 1;
			inode_map[(temp -> file_inode)] = 0;
			temp_ino->link_count--;
		}
	}
	printf("\n\nPersisting the directory deletion\n\n");
	lseek(fs_file, 0, SEEK_SET);
	write(fs_file, fs, FS_SIZE);
	return 0;
}
/*
static int fs_rename(const char *from, const char *to, unsigned int flags)
{
	return -1;
}

static int fs_truncate(const char *path, off_t size,
			struct fuse_file_info *fi)
{
	return -1;
}
*/
static int fs_create(const char *path, mode_t mode,struct fuse_file_info *fi)
{
	printf("\tCreate called\n");

	int ino = return_first_unused_inode(inode_map);
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
	//printf("%d\n", write(fs_file, fs, FS_SIZE));
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
	printf("Opening File - %s\n", path);
	int ino;
	path_to_inode(path, &ino);
	//printf("inode returned = %u\n", ino);
	if(ino == -1){
		return -1;
	}
	printf("Successful open\n");
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
	int ino;
	path_to_inode(path, &ino);
	size_t len;
	(void) fi;
	if(ino == -1)
		return -ENOENT;

	inode *temp_ino = (inodes + (ino * sizeof(inode)));
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

	printf("Write called!!\n");

	int ino;
	path_to_inode(path, &ino);
	inode *temp_ino = inodes + (ino * sizeof(inode));
	memcpy(((datablks + ((temp_ino -> data) * BLK_SIZE)) + offset), (buf), size);
	temp_ino -> size = (temp_ino -> size) +  size;
	temp_ino->link_count++;
	printf("\n\nPersisting the write\n\n");
	lseek(fs_file, 0, SEEK_SET);
	printf("%d\n", write(fs_file, fs, FS_SIZE));
	return 0;

}

static int fs_rm(const char *path){
	printf("rm called\n");

	int i = strlen(path);
	while(path[i] != '/'){
		i--;
	}
	char *subpath;
	subpath = (char *)malloc(strlen(path)+ 1);
	strncpy(subpath, path, i);
	subpath[i] = '/';
	subpath[i+1] = '\0';
	
	//printf("Subpath = %s\n", subpath);
	//printf("Name of directory to delete = %s\n", path + (i+1));


	int ino;
	dirent *temp;
	dirent *temp_data;
	inode *temp_ino;
	if(strcmp(subpath, "/") == 0){
  		temp = root_directory;
	}	
	else{
  		path_to_inode(subpath, &ino);
  		//printf("Inode for the path - %s - %d\n", path, ino);

  		temp_ino = inodes + (ino * sizeof(inode));

		temp = datablks + ((temp_ino->data) * BLK_SIZE);
	}

	
	//printf("ino -> data = %u\n", temp);
	//printf("root_directory = %u\n", root_directory);
	while(((strcmp(temp -> filename, "") != 0) || (temp -> file_inode) != 0) && strcmp(path + (i+1), temp -> filename) != 0){
		temp ++;
	}
	if((strcmp(temp -> filename, "") != 0)){
  		printf("temp -> file_inode = %d\n", temp -> file_inode);
		if(temp -> file_inode != 0){
    	temp_ino = inodes + ((temp -> file_inode) * sizeof(inode));
    	temp_data = ((dirent *)(datablks + ((temp_ino -> data) * BLK_SIZE)));
		strcpy(temp ->filename, "");
		
		inode_map[(temp -> file_inode)] = 0;
		temp_ino->link_count--;
		}
	}
	printf("\n\nPersisting the file deletion\n\n");
	lseek(fs_file, 0, SEEK_SET);
	write(fs_file, fs, FS_SIZE);
	return 0;

}