/*
 Name: Sum Yi Li
 Email: sammyli0106@gmail.com
 ID: 505146702
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "ext2_fs.h"

#define super_block_size 1024
#define group_table_offset 2048
#define num_bits_in_byte 8
#define directory 0x4000
#define file 0x8000
#define symbolic_link 0xA000
#define mode_const 0x0FFF
#define symbolic_file_length 60
#define num_fields_block_address 15
#define num_directory_blocks 12
#define timestamp_buffer_size 50
#define one 1

//declare file descriptor
int image_fd;

//declare the pass in filename
char* filename_arg;

//declare variables for superblock
unsigned int block_size;
struct ext2_super_block super_block;

//create variables for directory entry
struct ext2_dir_entry directory_entry;

//declare variables for group
struct ext2_group_desc group_desc;

//set up buffer for finding the timestamp
char change_buffer[timestamp_buffer_size];
char modify_buffer[timestamp_buffer_size];
char access_buffer[timestamp_buffer_size];


void error_for_usage()
{
    fprintf(stderr, "Unrecognized argument. Proper usage : lab3a file_name\n");
    exit(1);
}

void open_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "open() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void pread_error(int check)
{
    if (check == -1)
    {
        fprintf(stderr, "pread() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void malloc_error(char* bytes_buffer)
{
    if (bytes_buffer == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        fprintf(stderr, "Reason for the failure: %s\n", strerror(errno));
        exit(1);
    }
}

void assign_block_size()
{
    //assign value to the block size variable
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_superblock_func()
{
    int ret;
    ret = pread(image_fd, &super_block, sizeof(struct ext2_super_block), super_block_size);
    pread_error(ret);
    
    //store the printed info in variables and print it
    //total number of blocks
    unsigned int num_of_blocks = super_block.s_blocks_count;
    //total number of i-nodes
    unsigned int num_of_inodes = super_block.s_inodes_count;
    //i-node size
    unsigned int inode_size = super_block.s_inode_size;
    //blocks per group
    unsigned int blocks_per_group = super_block.s_blocks_per_group;
    //i-nodes per group
    unsigned int inodes_per_group = super_block.s_inodes_per_group;
    //first non-reserved i-node
    unsigned int first_nonr_inode = super_block.s_first_ino;
    
    fprintf(stdout, "%s,%u,%u,%u,%u,%u,%u,%u\n", "SUPERBLOCK", num_of_blocks, num_of_inodes, block_size, inode_size, blocks_per_group, inodes_per_group, first_nonr_inode);
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_group_func()
{
    //assume there is only one group, no need to find the number of groups
    int ret;
    ret = pread(image_fd, &group_desc, sizeof(struct ext2_group_desc), group_table_offset);
    pread_error(ret);
    
    //stored the printed info in variables and print it
    //group number (decimal, starting from zero)
    unsigned int group_number = 0;
    //total number of blocks in this group (decimal)
    unsigned int num_of_blocks_group = super_block.s_blocks_count;
    //total number of i-nodes in this group (decimal)
    unsigned int num_of_inodes_group = super_block.s_inodes_count;
    //number of free blocks (decimal)
    unsigned int num_of_free_blocks = group_desc.bg_free_blocks_count;
    //number of free i-nodes (decimal)
    unsigned int num_of_free_inodes = group_desc.bg_free_inodes_count;
    //block number of free block bitmap for this group (decimal)
    unsigned int num_of_block_bitmap = group_desc.bg_block_bitmap;
    //block number of free i-node bitmap for this group (decimal)
    unsigned int num_of_inode_bitmap = group_desc.bg_inode_bitmap;
    //block number of first block of i-nodes in this group (decimal)
    unsigned int first_inode_block = group_desc.bg_inode_table;
    
     fprintf(stdout, "%s,%u,%u,%u,%u,%u,%u,%u,%u\n", "GROUP", group_number, num_of_blocks_group, num_of_inodes_group, num_of_free_blocks, num_of_free_inodes, num_of_block_bitmap, num_of_inode_bitmap, first_inode_block);
}

unsigned int find_block_index(int index_of_bytes, int index_of_bits)
{
    return index_of_bits + 1 + index_of_bytes * num_bits_in_byte;
}

int compare_one(int j)
{
    return (one << j);
}

int check_use_bit(char one_byte, int j)
{
    int result = (compare_one(j)) & one_byte;
    
    return (result == 0);
}

int find_free_block_offset(unsigned int calculate_block_bitmap, unsigned int block_size)
{
    return calculate_block_bitmap * block_size + super_block_size;
}

char extract_bytes_buffer(char* bytes_buffer, int i)
{
    return bytes_buffer[i];
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_free_block_entries()
{
    // 1 = 'used', 0 = 'free'
    unsigned int num_of_block_bitmap = group_desc.bg_block_bitmap;
    //assigned the block_size, in case of any modify
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    //allocate to read in number of bytes
    char* bytes_buffer = malloc(sizeof(char) * block_size);
    malloc_error(bytes_buffer);
    
    //subtract one from the block size
    unsigned int calculate_block_bitmap = num_of_block_bitmap - 1;
    
    //read in everything once first
    int ret;
    ret = pread(image_fd, bytes_buffer, block_size, find_free_block_offset(calculate_block_bitmap, block_size));
    pread_error(ret);
    
    //loop over every byte of the bitmap
    for (unsigned int i = 0; i < block_size; i++)
    {
        //extract the byte from the bitmap
        char one_byte = extract_bytes_buffer(bytes_buffer, i);
        
        //loop over every bit of the specific byte
        for (int k = 0; k < num_bits_in_byte; k++)
        {
            if (check_use_bit(one_byte, k))
            {
                unsigned int index = find_block_index(i, k);
                
                //output the information
                fprintf(stdout, "%s,%d\n", "BFREE", index);
            }
        }
    }
    
    free(bytes_buffer);
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_free_inode_entries()
{
    //repeat the same format in the free block entries
    //find the inode bitmap to get the offset
    unsigned int num_of_inode_bitmap = group_desc.bg_inode_bitmap;
    //re-assigned the block size
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    //allocate to read in number of bytes
    char* bytes_buffer = malloc(sizeof(char) * block_size);
    malloc_error(bytes_buffer);
    
    //subtract one from the block size
    unsigned int calculate_inode_bitmap = num_of_inode_bitmap - 1;
    
    //read in everything once first
    int ret;
    ret = pread(image_fd, bytes_buffer, block_size, find_free_block_offset(calculate_inode_bitmap, block_size));
    pread_error(ret);
    
    for (unsigned int i = 0; i < block_size; i++)
    {
        //extract the byte from the bitmap
        char one_byte = extract_bytes_buffer(bytes_buffer, i);
        
        for (int k = 0; k < num_bits_in_byte; k++)
        {
            //do the checking, value either 0 or 1
            if (check_use_bit(one_byte, k))
            {
                //find the index, the number of available blocks
                unsigned int index = find_block_index(i, k);
                
                //output the information
                fprintf(stdout, "%s,%d\n", "IFREE", index);
            }
        }
    }
    
    free(bytes_buffer);
}

int check_inode(int check_i_mode, int check_i_link)
{
    return (check_i_mode == 0 && check_i_link == 0);
}

//Based from resources : http://man7.org/linux/man-pages/man3/strftime.3.html
void print_time_format(struct tm* time, int flag)
{
    if (flag == 1)
    {
        strftime(change_buffer, timestamp_buffer_size, "%m/%d/%y %H:%M:%S", time);
    }
    else if (flag == 2)
    {
        strftime(modify_buffer, timestamp_buffer_size, "%m/%d/%y %H:%M:%S", time);
    }
    else if (flag == 3)
    {
        strftime(access_buffer, timestamp_buffer_size, "%m/%d/%y %H:%M:%S", time);
    }
}

//Based from resources : http://man7.org/linux/man-pages/man3/ctime.3.html
void find_last_inode_change_time(struct ext2_inode inode_buffer)
{
    //store the change time
    time_t orginal_time = inode_buffer.i_ctime;
    struct tm* change_time = gmtime(&orginal_time);
    print_time_format(change_time, 1);
}

//Based from resources : http://man7.org/linux/man-pages/man3/ctime.3.html
void find_modification_time(struct ext2_inode inode_buffer)
{
    //store the modify time
    time_t orginal_time = inode_buffer.i_mtime;
    struct tm* modify_time = gmtime(&orginal_time);
    print_time_format(modify_time, 2);
}

//Based from resources : http://man7.org/linux/man-pages/man3/ctime.3.html
void find_last_access_time(struct ext2_inode inode_buffer)
{
    //store the access time
    time_t orginal_time = inode_buffer.i_atime;
    struct tm* access_time = gmtime(&orginal_time);
    print_time_format(access_time, 3);
}

int confirm_file(struct ext2_inode inode_buffer)
{
    return inode_buffer.i_mode & file;
}

int confirm_directory(struct ext2_inode inode_buffer)
{
    return inode_buffer.i_mode & directory;
}

int confirm_symbolic(struct ext2_inode inode_buffer)
{
    return inode_buffer.i_mode & symbolic_link;
}

char find_inode_file_type(struct ext2_inode inode_buffer)
{
    if (confirm_file(inode_buffer))
    {
        return 'f';
    }
    else if (confirm_directory(inode_buffer))
    {
        return 'd';
    }
    else if (confirm_symbolic(inode_buffer))
    {
        return 's';
    }
    else{
        //anything else that is not file, directory, or symbolic link
        return '?';
    }
}

int find_inode_mode(struct ext2_inode inode_buffer)
{
    int inode_mode = inode_buffer.i_mode & mode_const;
    return inode_mode;
}

void print_newline()
{
    fprintf(stdout, "\n");
}

int check_directory_type(char file_type)
{
    return (file_type == 'd');
}

int check_directory_entry(struct ext2_dir_entry directory_entry)
{
    return (directory_entry.inode != 0);
}

unsigned int extract_i_block(struct ext2_inode inode_buffer, int i)
{
    return (inode_buffer.i_block[i]);
}

unsigned int find_directory_entry_offset(struct ext2_inode inode_buffer, int i, unsigned int number_of_bytes)
{
    return (inode_buffer.i_block[i] * block_size + number_of_bytes);
}

int check_inode_i_block(unsigned int inode_i_block)
{
    return (inode_i_block != 0);
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_directory_entries_func(struct ext2_inode inode_buffer, char file_type, int index)
{
    //check variable
    int ret;
    
    //check the file type first
    if (check_directory_type(file_type))
    {
        //need to loop through 12 directory blocks
        for (int i = 0; i < num_directory_blocks; i++)
        {
            //need to check the content inside i_block is not zero
            unsigned int inode_i_block = extract_i_block(inode_buffer, i);
            
            if (check_inode_i_block(inode_i_block))
            {
                //find the block size again
                block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
                //variable for keep track of number of bytes
                unsigned int number_of_bytes = 0;
                
                while (number_of_bytes < block_size)
                {
                    //find the offset
                    unsigned int offset_num = find_directory_entry_offset(inode_buffer, i, number_of_bytes);
                    ret = pread(image_fd, &directory_entry, sizeof(struct ext2_dir_entry), offset_num);
                    pread_error(ret);
                    
                    //check the entry is not empty
                    if (check_directory_entry(directory_entry))
                    {
                        //inode number of referenced file
                        unsigned int inode_num_reference_file = directory_entry.inode;
                        //entry length
                        unsigned int entry_length = directory_entry.rec_len;
                        //name length
                        unsigned int name_length = directory_entry.name_len;
                        //name string
                        char* name_string = directory_entry.name;
                        
                        fprintf(stdout, "%s,%d,%d,%d,%d,%d,'%s'\n", "DIRENT", index + 1, number_of_bytes, inode_num_reference_file, entry_length, name_length, name_string);
                        
                    }
                    int directory_entry_length = directory_entry.rec_len;
                    number_of_bytes = number_of_bytes + directory_entry_length;
                }
                
            }
        }
    }
}
int check_indirect_entries_file_type(char file_type)
{
    return (file_type == 'f' || file_type == 'd');
}

int check_indirect_entries_i_block(int fields, struct ext2_inode inode_buffer)
{
    return (inode_buffer.i_block[fields] > 0);
}

int find_offset_indirect_reference(int fields, struct ext2_inode inode_buffer)
{
    return (inode_buffer.i_block[fields] * block_size);
}

int find_offset_level_2(int *second_level_block_pointer, int index)
{
    return (second_level_block_pointer[index] * block_size);
}

int find_offset_level_3(int *third_level_block_pointer, int index)
{
    return (third_level_block_pointer[index] * block_size);
}


int find_logical_block_offset(int level, int power, int index)
{
    int offset = 0;
    
    if (level == 1)
    {
        offset = 12 + index;
    }
    else if (level == 2)
    {
        offset = pow(256, power) + 12 + index;
    }
    else if (level == 3)
    {
        offset = pow(256, power) + pow(256, power - 1) + 12 + index;
    }
    
    //default value (error), not the available levels
    return offset;
}

int extract_level_block_pointer(int *level_block_pointer, int index)
{
    return (level_block_pointer[index]);
}

int find_num_of_ptrs()
{
    return (block_size / sizeof(int));
}

void read_level_block_ptr(int ret, int *level_block_pointer, int offset)
{
    ret = pread(image_fd, level_block_pointer, block_size, offset);
    pread_error(ret);
}

void double_indirect_block(struct ext2_inode inode_buffer, int ret, int *second_level_block_pointer, int *first_level_block_pointer, unsigned int level_of_indirection, unsigned int logical_block_offset, unsigned int indirect_block_number, unsigned int referenced_block_number, int node_num)
{
    if (check_indirect_entries_i_block(13, inode_buffer))
    {
        //find the number of block pointers
        int number_of_pointers = find_num_of_ptrs();
        //find the offset for pread
        int offset = find_offset_indirect_reference(13, inode_buffer);
        
        //start reading using pread
        read_level_block_ptr(ret, second_level_block_pointer, offset);
        
        for (int i = 0; i < number_of_pointers; i++)
        {
            //check whether the second level slot is empty, put it as function later
            if (extract_level_block_pointer(second_level_block_pointer, i) != 0)
            {
                //level of indirection of block
                level_of_indirection = 2;
                //logical block offset
                logical_block_offset = find_logical_block_offset(2, 1, i);
                //indirect block number
                indirect_block_number = inode_buffer.i_block[13];
                //referenced block number
                referenced_block_number = second_level_block_pointer[i];
                
                fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                
                //read for first level
                //find the new offset based on second level
                int offset_2 = find_offset_level_2(second_level_block_pointer, i);
                read_level_block_ptr(ret, first_level_block_pointer, offset_2);
                
                for (int j = 0; j < number_of_pointers; j++)
                {
                    //check whether the first level slot is empty
                    if (extract_level_block_pointer(first_level_block_pointer, j) != 0)
                    {
                        //level of indirection of block
                        level_of_indirection = 1;
                        //logical block offset
                        logical_block_offset = find_logical_block_offset(2, 1, j);
                        //indirect block numbers
                        indirect_block_number = second_level_block_pointer[i];
                        //reference block number
                        referenced_block_number = first_level_block_pointer[j];
                        
                        fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                    }
                }
            }
        }
    }
}

void triple_indirect_block(struct ext2_inode inode_buffer, int ret, int *third_level_block_pointer, int *second_level_block_pointer, int *first_level_block_pointer, unsigned int level_of_indirection, unsigned int logical_block_offset, unsigned int indirect_block_number, unsigned int referenced_block_number, int node_num)
{
    if (check_indirect_entries_i_block(14, inode_buffer))
    {
        //find the number of block pointers
        int number_of_pointers = find_num_of_ptrs();
        //find the offset for pread
        int offset = find_offset_indirect_reference(14, inode_buffer);
        
        //start reading using pread
        read_level_block_ptr(ret, third_level_block_pointer, offset);
        
        for (int i = 0; i < number_of_pointers; i++)
        {
            if (extract_level_block_pointer(third_level_block_pointer, i) != 0)
            {
                //level of indirection
                level_of_indirection = 3;
                //logical block offset
                logical_block_offset = find_logical_block_offset(3, 2, i);
                //indirect block numbers
                indirect_block_number = inode_buffer.i_block[14];
                //reference block number
                referenced_block_number = third_level_block_pointer[i];
                
                fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                
                //find the new offset based on the third level
                int offset_2 = find_offset_level_3(third_level_block_pointer, i);
                
                read_level_block_ptr(ret, second_level_block_pointer, offset_2);
                
                for (int j = 0; j < number_of_pointers; j++)
                {
                    if (extract_level_block_pointer(second_level_block_pointer, j) != 0)
                    {
                        
                        level_of_indirection = 2;
                        logical_block_offset = find_logical_block_offset(3, 2, j);
                        indirect_block_number = third_level_block_pointer[i];
                        referenced_block_number = second_level_block_pointer[j];
                        
                        fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                        
                        //find the new offset based on the second level
                        int offset_3 = find_offset_level_2(second_level_block_pointer, j);
                        read_level_block_ptr(ret, first_level_block_pointer, offset_3);
                        
                        for (int k = 0; k < number_of_pointers; k++)
                        {
                            if (extract_level_block_pointer(first_level_block_pointer, k) != 0)
                            {
                                level_of_indirection = 1;
                                logical_block_offset = find_logical_block_offset(3, 2, k);
                                indirect_block_number = second_level_block_pointer[j];
                                referenced_block_number = first_level_block_pointer[k];
                                
                                fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                                
                            }
                        }
                    }
                }
            }
        }
    }
}

//Based from resources : http://www.nongnu.org/ext2-doc/ext2.html
void handle_indirect_entries_func(struct ext2_inode inode_buffer, char file_type, int index)
{
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    
    //declare the level pointers here
    int first_level_block_pointer[block_size];
    int second_level_block_pointer[block_size];
    int third_level_block_pointer[block_size];
    int ret = 0;
    unsigned int level_of_indirection;
    unsigned int logical_block_offset;
    unsigned int indirect_block_number;
    unsigned int referenced_block_number;
    int node_num = index + 1;
    
    //only apply to either directory or file
    if (check_indirect_entries_file_type(file_type))
    {
        //single indirect block references
        if (check_indirect_entries_i_block(12, inode_buffer))
        {
            //find the number of block pointers
            int number_of_pointers = find_num_of_ptrs();
            //find the offset for pread
            int offset = find_offset_indirect_reference(12, inode_buffer);
            
            //start reading using pread
            read_level_block_ptr(ret, first_level_block_pointer, offset);
            
            for (int i = 0; i < number_of_pointers; i++)
            {
                //check whether the first level slot is empty, put it as function later
                if (extract_level_block_pointer(first_level_block_pointer, i) != 0)
                {
                    //level of indirection of block
                    level_of_indirection = 1;
                    //logical block offset
                    logical_block_offset = find_logical_block_offset(1, 0, i);
                    //indirect block number
                    indirect_block_number = inode_buffer.i_block[12];
                    //referenced block number
                    referenced_block_number = first_level_block_pointer[i];
                    
                    fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", node_num, level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number);
                }
            }
        }
        
        //double indirect block references
        double_indirect_block(inode_buffer, ret, second_level_block_pointer, first_level_block_pointer,level_of_indirection, logical_block_offset, indirect_block_number, referenced_block_number, node_num);
        
        //triple indirect block references
        triple_indirect_block(inode_buffer, ret, third_level_block_pointer, second_level_block_pointer,first_level_block_pointer, level_of_indirection, logical_block_offset, indirect_block_number,referenced_block_number, node_num);
    }
}

void print_inode_i_block(struct ext2_inode inode_buffer)
{
    for (int i = 0; i < num_fields_block_address; i++)
    {
        unsigned int inode_i_block = inode_buffer.i_block[i];
        fprintf(stdout, ",%d", inode_i_block);
    }
}

void handle_inode_summary_func()
{
    //inode bitmap, inode table
    //check variable
    int ret;
    
    //find the inode table again to get the offset for pread
    unsigned int num_of_inode_table = group_desc.bg_inode_table;
    
    //find the number of inodes to loop through it
    unsigned int num_of_inodes = super_block.s_inodes_count;
    
    //recalculate the block size
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    
    //since only one group, so just one table
    struct ext2_inode inode_buffer;
    
    for (unsigned int i = 0; i < num_of_inodes; i++)
    {
        //find the offset
        int offset = num_of_inode_table * block_size + i * sizeof(struct ext2_inode);
        
        //start read into buffer by pread
        ret = pread(image_fd, &inode_buffer, sizeof(struct ext2_inode), offset);
        pread_error(ret);
        
        int check_i_mode = inode_buffer.i_mode;
        int check_i_link = inode_buffer.i_links_count;
        
        //double check whether the inodes are free or not
        if (check_inode(check_i_mode, check_i_link))
        {
            //if the inode is not free, then skip to the next one to search
            continue;
        }
            //find the time information of the inode
            //time of last inode change
            find_last_inode_change_time(inode_buffer);
            
            //modification time
            find_modification_time(inode_buffer);
            
            //time of last access
            find_last_access_time(inode_buffer);
            
            //find the type of file that inode points to
            char file_type;
            file_type = find_inode_file_type(inode_buffer);
            
            //find the mode
            int inode_mode;
            inode_mode = find_inode_mode(inode_buffer);
            
            //inode number
            int node_number = i + 1;
            
            //owner
            unsigned int owner = inode_buffer.i_uid;
            
            //group
            unsigned int group = inode_buffer.i_gid;
            
            //link count
            unsigned int link_count = inode_buffer.i_links_count;
            
            //file size
            unsigned int file_size = inode_buffer.i_size;
            
            //number of blocks of disk space of the file
            unsigned int num_blocks_diskspace = inode_buffer.i_blocks;
            
            
            fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", "INODE", node_number, file_type, inode_mode, owner, group, link_count, change_buffer, modify_buffer, access_buffer, file_size, num_blocks_diskspace);
        
        print_inode_i_block(inode_buffer);
        
            //print out a next line for formatting
            print_newline();
            
            //directory entries, for each directory I-node, scan every data block
            handle_directory_entries_func(inode_buffer, file_type, i);
            
            //indirect entries
            handle_indirect_entries_func(inode_buffer, file_type, i);
    }
    
}

int main(int argc, char **argv)
{
    //first check the pass in argument and print out error messages and exit codes
    if (argc != 2)
    {
        error_for_usage();
    }
    
    //store the pass in filename in a variable
    filename_arg = argv[1];
    
    //start reading the file into file descriptor, read only
    image_fd = open(filename_arg, O_RDONLY);
    open_error(image_fd);
    
    //find the value of the block size
    assign_block_size();
    
    //handle superblock summary
    handle_superblock_func();
    
    //handle group summary
    handle_group_func();
    
    //handle the free block entries
    handle_free_block_entries();
    
    //handle the free I-node entries
    handle_free_inode_entries();
    
    //handle the I-node summary
    handle_inode_summary_func();
    
    return 0;
}















