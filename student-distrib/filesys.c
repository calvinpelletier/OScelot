// filesys.c

#include "filesys.h"
#include "lib.h"




// GLOBAL VARIABLES
// each process will have its own filearray when we implement that
static void* FILESYS_START;
static void* FILESYS_END;
static file_t filearray[FILEARRAY_SIZE]; // 0 = stdin, 1 = stdout, 2-7 = free to use
static bootblock_t bootblock;
static inode_t* inodes;
static void* FS_DATA_START;
static unsigned int dirs_read;


// FUNCTION DECLARATIONS
int filesys_init(void* start, void* end);
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry);
int read_dentry_by_index(unsigned int index, dentry_t* dentry);
int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length);
int fs_copy(const unsigned char* fname, unsigned char * mem_location); 
int fs_open (const unsigned char* filename);
int fs_close(int fd);
int fs_read (int fd, unsigned char * buf, int nbytes);
int fs_write (int fd, unsigned char * buf, int nbytes);
int file_read (int fd, unsigned char * buf, int nbytes);
int dir_read (int fd, unsigned char * buf, int nbytes);
int test(void);

static fileops_t fs_jumptable = {fs_open, fs_read, fs_write, fs_close};

// EXTERNAL FUNCTIONS
int filesys_init(void* start, void* end) {
    FILESYS_START = start;
    FILESYS_END = end;

    // initialize internal fd (for testing purposes)
    int i;
    for (i = 0; i < FILEARRAY_SIZE; i++) {
    	if (i < 2) 
    		filearray[i].flags.in_use = 1;
    	else
    		filearray[i].flags.in_use = 0;
    }

    // populate bootblock
    bootblock = *((bootblock_t*)start);

    // initialize base of array of inodes (starts at absolute block number 1)
    inodes = (inode_t *)(FILESYS_START + FS_BLOCK_SIZE);

    // initialize start of data blocks (starts after boot block and inode blocks)
    FS_DATA_START = FILESYS_START + FS_BLOCK_SIZE + (bootblock.n_inodes) * FS_BLOCK_SIZE;

    if (DEBUG_ALL) {
        test();
    }

    return 0;
}


// HELPER FUNCTIONS
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry) {
    int len = 0;
    while (len <= MAX_FNAME_LEN && fname[len]) {
        len++;
    }

    int i = 0;
    while (i < bootblock.n_dentries && i < MAX_DENTRIES) {
        if (!strncmp( (int8_t*) fname, (int8_t *) bootblock.dentries[i].name, len)) {
            // found match
            *dentry = bootblock.dentries[i];
            return 0;
        }
        i++;
    }

    return -1;
}

int read_dentry_by_index(unsigned int index, dentry_t* dentry) {

	// Return error if invalid index
	if (index >= MAX_DENTRIES || index >= bootblock.n_dentries)
		return -1;

    *dentry = bootblock.dentries[index];

    return 0;
}

int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length) {
	// Initialize local variables
	unsigned int bytes_read = 0; 								// current number of bytes read
	unsigned int file_data_block = offset/FS_BLOCK_SIZE;		// current data block within inode (indexes into inode's array of data block numbers)
	unsigned int block_offset = offset % FS_BLOCK_SIZE;			// offset into the current data block
	unsigned int fs_data_block = inodes[inode].datablocks[file_data_block];										  	// current filesytem data block number 
	unsigned char * curr_data_loc = (unsigned char *)(FS_DATA_START + fs_data_block*FS_BLOCK_SIZE + block_offset); // ptr to current byte to be read

	// Error checking
    if (inode >= bootblock.n_inodes) 
    	return -1; 	// inode out of range
    if (offset >= inodes[inode].length)
    	return 0; 	// offset past file length, end of file reached
    if (fs_data_block >= bootblock.n_datablocks)
    	return -1;	// data block number out of range 

    while (bytes_read < length) {

    	// end of file reached
    	if (offset + bytes_read >= inodes[inode].length)
    		return bytes_read;

    	// end of current data block reached 
    	if (block_offset + bytes_read >= FS_BLOCK_SIZE) {
    		file_data_block++; // increment to next data block in inode's data block array
    		fs_data_block = inodes[inode].datablocks[file_data_block]; 	// update actual filesystem datablock number
    		block_offset = 0;  // reset offset into block to start of new data block

    		if (fs_data_block >= bootblock.n_datablocks)
    			return -1; 	 	// new data block number out of range

    		curr_data_loc = (unsigned char *)(FS_DATA_START + fs_data_block*FS_BLOCK_SIZE + block_offset); // update current byte ptr to start of new block
    	}

    	// else, read current byte into buf
    	buf[bytes_read] = *curr_data_loc;
    	bytes_read++;
    	curr_data_loc++;
    }

    return bytes_read;
}

int fs_copy(const unsigned char* fname, unsigned char * mem_location) {
	dentry_t file_dentry;
	unsigned int inode;
	int bytes_read;
	unsigned char buf[bootblock.n_datablocks*FS_BLOCK_SIZE]; //maximum file size - if 1 file used all available datablocks

	if (!fname)
		return -1; // invalid file name

	// get dentry for filename
	if (read_dentry_by_name(fname, &file_dentry))
		return -1; // function returned -1

	// read file data into buf
	inode = file_dentry.inode;
	bytes_read = read_data(inode, 0, buf, inodes[inode].length);
	if (bytes_read == -1)
		return -1; 

	// write the file into physical memory
	int i;
	for (i = 0; i < bytes_read; i++) {
		mem_location[i] = buf[i];
	}

	return 0;
}

int fs_open (const unsigned char* filename) {
	dentry_t dentry;
	if (read_dentry_by_name(filename, &dentry))
		return -1;

	int i;
	for (i = 0; i < FILEARRAY_SIZE; i++) {
		if (filearray[i].flags.in_use == 0) {
			filearray[i].jumptable = fs_jumptable;
			filearray[i].inode = dentry.inode;
			filearray[i].position = 0;
			filearray[i].filetype = dentry.type;
			filearray[i].flags.read_only = 1;
			filearray[i].flags.write_only = 0;
			filearray[i].flags.in_use = 1;
			return i;
		}
	}

	return -1;
}


int fs_close(int fd) {
	if (fd == 0 || fd == 1)
		return -1;
	filearray[fd].flags.in_use = 0;
	if (filearray[fd].filetype == 1)
		dirs_read = 0;
	return 0;
}


int fs_read (int fd, unsigned char * buf, int nbytes) {
	if (fd < 0 || fd >= FILEARRAY_SIZE)
		return -1;
	if (filearray[fd].filetype == 2) // regular file
		return file_read (fd, buf, nbytes);
	else if (filearray[fd].filetype == 1) // dir
		return dir_read(fd, buf, nbytes);
	else // RTC
		return -1;
}

int fs_write (int fd, unsigned char * buf, int nbytes) {
	return -1; // file system is read only
}


int file_read (int fd, unsigned char * buf, int nbytes) {
	int bytes_read = read_data(filearray[fd].inode, filearray[fd].position, buf, nbytes);
	if (bytes_read != -1)
		filearray[fd].position += bytes_read;
	return bytes_read;
}


int dir_read (int fd, unsigned char * buf, int nbytes) {
	if (dirs_read >= bootblock.n_dentries) 
		return 0;

	strncpy( (char *) buf, (char *) bootblock.dentries[dirs_read].name, MAX_FNAME_LEN); // copy full 32 bytes of filename into buf
	dirs_read++;

	return MAX_FNAME_LEN; 
}

// TESTING FUNCTIONS
int test(void) {
    int ret = 0;
    printf("~~~FILE SYSTEM TEST~~~\n");
    printf("n_dentries: %d\n", bootblock.n_dentries);
    printf("n_inodes: %d\n", bootblock.n_inodes);
    printf("n_datablocks: %d\n", bootblock.n_datablocks);

    // test read_dentry_by_index, read_dentry_by_name too (if it isn't commented out)
    dentry_t temp;
    int result;
    int i;
    for (i = 0; i < bootblock.n_dentries; i++) {
	    result = read_dentry_by_index(i, &temp);
	    if (result) {
	        printf("FAIL: did not find the %d directory entry\n", i);
	        ret = -1;
	    } else {
	        printf("dentry name: %s, type: %d, inode: %d\n", temp.name, temp.type, temp.inode);
	  //    result = read_dentry_by_name(temp.name, &temp);
	  //    if (result) {
      //   		printf("FAIL: did not find %s directory entry\n", temp.name);
     //  		ret = -1;
     //     } else {
     //   		printf("dentry name: %s, type: %d, inode: %d\n", temp.name, temp.type, temp.inode);
   	// 		}
	    }
	}

	// test read_data
	// tested with inode 16, 13
	// int bytes_read;
	// unsigned char buf[bootblock.n_datablocks*FS_BLOCK_SIZE];
	// bytes_read = read_data(13, 0, buf, bootblock.n_datablocks*FS_BLOCK_SIZE);
	// if (bytes_read == -1) {
	// 	printf("Read_data returned an error\n");
	// 	ret = -1;
	// }
	// else {
	// 	for (i = 0; i < bytes_read; i++)
	// 		printf("%c", buf[i]);
	// 	printf("\n");
	// }

	// test fs_copy by writing frame1.txt to memory and printing from this location
	// int bytes_read;
	// unsigned char buf[bootblock.n_datablocks*FS_BLOCK_SIZE];
	// bytes_read = read_data(13, 0, buf, bootblock.n_datablocks*FS_BLOCK_SIZE);
	// unsigned char * mem_location = 0x8000;
	// if (fs_copy("frame1.txt", mem_location)) {
	// 	printf("fs_copy returned an error\n");
	// 	ret = -1;
	// }
	// else {
	// 	for (i = 0; i < bytes_read; i++)
	// 	printf("%c", mem_location[i]);
	// 	printf("\n");
	// }

	// test OCRW
	// int bytes_read;
	// unsigned char buf[bootblock.n_datablocks*FS_BLOCK_SIZE];
	// int fd = fs_open("frame1.txt");
	// if (fd == -1) {
	// 	printf("fs_open has failed.\n");
	// 	return -1;
	// }
	// bytes_read = fs_read(fd, buf, inodes[filearray[fd].inode].length);
	// if (bytes_read == -1) {
	// 	printf("fs_read has failed.\n");
	// 	return -1;
	// }
	// for (i = 0; i < bytes_read; i++)
	// 	printf("%c", buf[i]);
	// printf("\n");
	// if(fs_close(fd))
	// 	printf("fs_close has failed.\n");
	// printf("%d\n", filearray[fd].flags.in_use);

	// test printing dirs
	// unsigned char buf[32];
	// int fd = fs_open(".");
	// int j;
	// if (fd == -1) {
	// 	printf("fs_open has failed.\n");
	// 	return -1;
	// }
	// for (i = 0; i < bootblock.n_dentries; i++) {
	// 	fs_read(fd, buf, 0);
	// 	for (j = 0; j < 32; j++) {
	// 		printf("%c", buf[j]);
	// 	}
	// 	printf("\n");
	// }
	// fs_close(fd);

    printf("~~~~~~\n");
    return ret;
}













// asdf
