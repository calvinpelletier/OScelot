// filesys.c

#include "filesys.h"
#include "lib.h"


// GLOBAL VARIABLES
static void* FILESYS_START;
static void* FILESYS_END;
static bootblock_t bootblock;
static inode_t* inodes;
static void* FS_DATA_START;
static uint32_t dirs_read;

// FUNCTION DECLARATIONS
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t file_read (file_t * file, uint8_t * buf, int32_t nbytes);
int32_t dir_read (file_t* file, uint8_t * buf, int32_t nbytes);
int32_t test_debug();


// EXTERNAL FUNCTIONS
/*
fs_init
    DESCRIPTION: initializes the file system
    INPUTS: the start and end locations of the filesystem img in memory
    OUTPUTS: none
    RETURNS: 0 for success, -1 for fail
*/
int32_t fs_init(void* start, void* end) {
    FILESYS_START = start;
    FILESYS_END = end;

    // populate bootblock
    bootblock = *((bootblock_t*)start);

    // initialize base of array of inodes (starts at absolute block number 1)
    inodes = (inode_t *)(FILESYS_START + FS_BLOCK_SIZE);

    // initialize start of data blocks (starts after boot block and inode blocks)
    FS_DATA_START = FILESYS_START + FS_BLOCK_SIZE + (bootblock.n_inodes) * FS_BLOCK_SIZE;

    if (DEBUG_ALL) {
        // test_debug();

        // int32_t result;
        // printf("~~~FILE SYSTEM DEMO~~~\n");

        // DEMO TEST 1
        // result = test_demo1("frame0.txt");
        // if (result) {
        //     printf("DEMO TEST 1 FAIL\n");
        // }

        // DEMO TEST 2
        // result = test_demo2("frame0.txt");
        // if (result == -1) {
        //     printf("DEMO TEST 2 FAIL\n");
        // } else {
        //     printf("Size: %d\n", result); // compare to actual size using 'stat --printf="%s\n" frame0.txt' command
        // }

        // DEMO TEST 3
        // result = test_demo3();
        // if (result) {
        //     printf("DEMO TEST 3 FAIL\n");
        // }

        // printf("~~~~~~\n");
    }

    return 0;
}


// HELPER FUNCTIONS
/*
read_dentry_by_name
    DESCRIPTION: populates a dentry struct given a file name
    INPUTS: file name
    OUTPUTS: dentry struct
    RETURNS: 0 for success, -1 for fail
*/
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry) {

    if (!fname || !dentry)
        return -1;
    int32_t len = 0, len2 = 0;
    while (len <= MAX_FNAME_LEN && fname[len] != '\0') {
        len++;
    }


    int32_t i = 0;
    while (i < bootblock.n_dentries && i < MAX_DENTRIES) {
        while (len2 <= MAX_FNAME_LEN && bootblock.dentries[i].name[len2] != '\0') {
            len2++;
        }
        
        if (len2 == len) {
            if (!strncmp(fname, bootblock.dentries[i].name, len)) {
                // found match
                *dentry = bootblock.dentries[i];
                return 0;
            }
        }
        i++;
        len2 = 0;
    }

    return -1;
}

/*
read_dentry_by_index
    DESCRIPTION: populates a dentry struct given an index
    INPUTS: index
    OUTPUTS: dentry struct
    RETURNS: 0 for success, -1 for fail
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {

	// Return error if invalid index
	if (index >= MAX_DENTRIES || index >= bootblock.n_dentries || index <0 || !dentry)
		return -1;

    *dentry = bootblock.dentries[index];

    return 0;
}

/*
read_data
    DESCRIPTION: reads data from the filesystem
    INPUTS: inode that points to the data, offset to start at, number of bytes to read
    OUTPUTS: bytes read
    RETURNS: number of bytes read successfully, -1 for failure
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
	// Initialize local variables
	uint32_t bytes_read = 0; 								// current number of bytes read
	uint32_t file_data_block = offset/FS_BLOCK_SIZE;		// current data block within inode (indexes into inode's array of data block numbers)
	uint32_t block_offset = offset % FS_BLOCK_SIZE;			// offset into the current data block
	uint32_t fs_data_block = inodes[inode].datablocks[file_data_block];										  	// current filesytem data block number
	uint8_t * curr_data_loc = (uint8_t *)(FS_DATA_START + fs_data_block*FS_BLOCK_SIZE + block_offset); // ptr to current byte to be read

	// Error checking
    if (!buf)
        return -1; // NULL
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
    	if (block_offset >= FS_BLOCK_SIZE) {
    		file_data_block++; // increment to next data block in inode's data block array
    		fs_data_block = inodes[inode].datablocks[file_data_block]; 	// update actual filesystem datablock number
    		block_offset = 0;  // reset offset into block to start of new data block

    		if (fs_data_block >= bootblock.n_datablocks)
    			return -1; 	 	// new data block number out of range

    		curr_data_loc = (uint8_t *)(FS_DATA_START + fs_data_block*FS_BLOCK_SIZE + block_offset); // update current byte ptr to start of new block
    	}

    	// else, read current byte into buf
    	buf[bytes_read] = *curr_data_loc;
    	bytes_read++;
        block_offset++;
    	curr_data_loc++;
    }

    return bytes_read;
}

/*
fs_copy
    DESCRIPTION: writes file into a specified location in memory
    INPUTS: file name, memory location
    OUTPUTS: file at memory location
    RETURNS: 0 for success, -1 for fail
*/
int32_t fs_copy(const int8_t* fname, uint8_t * mem_location) {
	dentry_t file_dentry;
	uint32_t inode;
	int32_t bytes_read;
	uint8_t buf[bootblock.n_datablocks*FS_BLOCK_SIZE]; //maximum file size - if 1 file used all available datablocks

	if (!fname || !mem_location)
		return -1; // invalid file name or invalid mem_location

	// get dentry for filename
	if (read_dentry_by_name(fname, &file_dentry))
		return -1; // function returned -1

	// read file data into buf
	inode = file_dentry.inode;
	bytes_read = read_data(inode, 0, buf, inodes[inode].length);
	if (bytes_read == -1)
		return -1;

	// write the file into physical memory
	int32_t i;
	for (i = 0; i < bytes_read; i++) {
		mem_location[i] = buf[i];
	}

	return 0;
}

/*
fs_open
    DESCRIPTION: opens a file
    INPUTS: file name
    OUTPUTS: none
    RETURNS: 0 on success, -1 on fail
*/
int32_t fs_open () {
	return 0;
}

/*
fs_close
    DESCRIPTION: closes a file
    INPUTS: file descriptor
    OUTPUTS: none
    RETURNS: 0 for success, -1 for fail
*/

int32_t fs_close(file_t * file) {
    if (!file)
        return -1; // NULL
	if (file->filetype == 1)
		dirs_read = 0;
	return 0;
}


/*
fs_read
    DESCRIPTION: reads from a file starting at the last read location and ending at the target number of bytes
    INPUTS: file descriptor, number of bytes
    OUTPUTS: bytes read
    RETURNS: number of bytes read on success, -1 for fail
*/
int32_t fs_read (file_t* file, uint8_t * buf, int32_t nbytes) {
    if (!file || !buf)
        return -1; // NULL
	if (file->filetype == 2) // regular file
		return file_read (file, buf, nbytes);
	else if (file->filetype == 1) // dir
		return dir_read(file, buf, nbytes);
	else // RTC
		return -1;
}


/*
fs_write
    DESCRIPTION: does nothing
    INPUTS: file descriptor, data, number of bytes
    OUTPUTS: none
    RETURNS: -1
*/
int32_t fs_write (file_t * file, uint8_t * buf, int32_t nbytes) {
	return -1; // file system is read only
}

/*
file_read
    DESCRIPTION: helper function for reading files
    INPUTS: file descriptor, number of bytes
    OUTPUTS: read bytes
    RETURNS: number of bytes read on success, -1 for fail
*/
int32_t file_read (file_t * file, uint8_t * buf, int32_t nbytes) {
	int32_t bytes_read = read_data(file->inode, file->position, buf, nbytes);
	if (bytes_read != -1)
		file->position += bytes_read;
	return bytes_read;
}

/*
dir_read
    DESCRIPTION: helper function for reading directories
    INPUTS: file descriptor
    OUTPUTS: directory name
    RETURNS: directory name length on success, 0 for fail
*/
int32_t dir_read (file_t * file, uint8_t * buf, int32_t nbytes) {
	int len = 0;
    if (dirs_read >= bootblock.n_dentries)
		return 0;

    while (len <= MAX_FNAME_LEN && bootblock.dentries[dirs_read].name[len] != '\0') {
            len++;
    }
	strncpy( (int8_t *) buf, bootblock.dentries[dirs_read].name, len); // copy full 32 bytes of filename into buf
    if (len < MAX_FNAME_LEN){       // make it so if filename is less than max, we terminate it just past its length and return len+1
        buf[len] = '\0';            // makes cat . look a lot better
        len++;
    }
	dirs_read++;

	return len;
}

// TESTING FUNCTIONS
// int32_t test_debug(void) {
//     int32_t ret = 0;
//     printf("~~~FILE SYSTEM TEST~~~\n");
//     printf("n_dentries: %d\n", bootblock.n_dentries);
//     printf("n_inodes: %d\n", bootblock.n_inodes);
//     printf("n_datablocks: %d\n", bootblock.n_datablocks);
//
//     // test read_dentry_by_index, read_dentry_by_name too (if it isn't commented out)
//     dentry_t temp;
//     int32_t result;
//     int32_t i;
//     for (i = 0; i < bootblock.n_dentries; i++) {
// 	    result = read_dentry_by_index(i, &temp);
// 	    if (result) {
// 	        printf("FAIL: did not find the %d directory entry\n", i);
// 	        ret = -1;
// 	    } else {
// 	        printf("dentry name: %s, type: %d, inode: %d\n", temp.name, temp.type, temp.inode);
//             char* tmp = temp.name;
//     	    result = read_dentry_by_name(tmp, &temp);
//     	    if (result) {
//                 printf("FAIL: did not find %s directory entry\n", temp.name);
//           	    ret = -1;
//             } else {
//            	    printf("dentry name: %s, type: %d, inode: %d\n", temp.name, temp.type, temp.inode);
//        		}
// 	    }
// 	}
//
// 	// test read_data
// 	// tested with inode 16, 13
// 	int32_t bytes_read;
// 	uint8_t buf[bootblock.n_datablocks*FS_BLOCK_SIZE];
// 	bytes_read = read_data(13, 0, buf, bootblock.n_datablocks*FS_BLOCK_SIZE);
// 	if (bytes_read == -1) {
// 		printf("Read_data returned an error\n");
// 		ret = -1;
// 	}
// 	else {
// 		for (i = 0; i < bytes_read; i++)
// 			printf("%c", buf[i]);
// 		printf("\n");
// 	}
//
// 	// test fs_copy by writing frame1.txt to memory and printing from this location
// 	bytes_read = read_data(13, 0, buf, bootblock.n_datablocks*FS_BLOCK_SIZE);
// 	uint8_t *mem_location = (uint8_t *)0x8000;
// 	if (fs_copy("frame1.txt", mem_location)) {
// 		printf("fs_copy returned an error\n");
// 		ret = -1;
// 	}
// 	else {
// 		for (i = 0; i < bytes_read; i++)
// 		printf("%c", mem_location[i]);
// 		printf("\n");
// 	}
//
// 	// test OCRW
// 	int32_t fd = fs_open("frame1.txt");
// 	if (fd == -1) {
// 		printf("fs_open has failed.\n");
// 		return -1;
// 	}
// 	bytes_read = fs_read(fd, buf, inodes[filearray[fd].inode].length);
// 	if (bytes_read == -1) {
// 		printf("fs_read has failed.\n");
// 		return -1;
// 	}
// 	for (i = 0; i < bytes_read; i++)
// 		printf("%c", buf[i]);
// 	printf("\n");
// 	if(fs_close(fd))
// 		printf("fs_close has failed.\n");
// 	printf("%d\n", filearray[fd].flags.in_use);
//
// 	// test printing dirs
// 	uint8_t buf32[32];
// 	fd = fs_open(".");
// 	int32_t j;
// 	if (fd == -1) {
// 		printf("fs_open has failed.\n");
// 		return -1;
// 	}
// 	for (i = 0; i < bootblock.n_dentries; i++) {
// 		fs_read(fd, buf32, 0);
// 		for (j = 0; j < 32; j++) {
// 			printf("%c", buf32[j]);
// 		}
// 		printf("\n");
// 	}
// 	fs_close(fd);
//
//     printf("~~~~~~\n");
//     return ret;
// }
//
// int32_t test_demo1(char* filename) {
//     int32_t fd, bytes_read;
//     uint8_t buf[bootblock.n_datablocks * FS_BLOCK_SIZE];
//
//     if ((fd = fs_open(filename)) == -1) {
//         return -1;
//     }
//     if (filearray[fd].inode == -1) {
//         return -1;
//     }
//
//     bytes_read = fs_read(fd, buf, inodes[filearray[fd].inode].length);
//     if (bytes_read == -1) {
//         return -1;
//     }
//
//     buf[inodes[filearray[fd].inode].length] = '\0';
//     printf("%s\n", buf);
//     return 0;
// }
//
// int32_t test_demo2(char* filename) {
//     int32_t fd;
//     if ((fd = fs_open(filename)) == -1) {
//         return -1;
//     }
//     if (filearray[fd].inode == -1) {
//         return -1;
//     }
//     return inodes[filearray[fd].inode].length;
// }
//
// int32_t test_demo3(void) {
//     uint8_t buf[MAX_FNAME_LEN];
//     int32_t fd, count;
//
//     if ((fd = fs_open((char*)".")) == -1) {
//         return -1;
//     }
//
//     while ((count = fs_read(fd, buf, MAX_FNAME_LEN))) {
//         if (count == -1) {
//             return -1;
//         }
//         printf("%s\n", buf);
//     }
//
//     return 0;
// }
