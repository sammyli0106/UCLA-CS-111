#!/usr/bin/python

# Name: Sum Yi Li
# Email: sammyli0106@gmail.com
# ID: 505146702

import random, sys, argparse, csv

#define a general error message function
def error_msg_func(error_arg):
    sys.stderr.write(error_arg)
    exit(1)

#declare the needed variables
super_block_elem = None
group_desc = None

bad_param = False

#INODE
inode_list_elem = []

#BFREE
free_block_list = []

#IFREE
free_inode_list = []

#DIRENT
dirent_elem = []

#INDIRECT
indirect_elem = []

def create_duplicate_block_list():
    global duplicate_block_list
    duplicate_block_list = set()

def create_referenced_block_list():
    #this is a dictionary
    global referenced_block_list
    referenced_block_list = {}

def create_inode_allocation_list():
    global inode_allocation_list
    inode_allocation_list = []

def check_file_type(file_type):
    symbolic_link = 's'
    return file_type != symbolic_link

def create_direct_list():
    global direct_list
    direct_list = []

def create_indirect_list():
    global indirect_list
    indirect_list = []

#create a inode class with parameters
class inode:
    def extract_dirent_blocks(self, param, file_type):
        create_direct_list()
        if check_file_type(file_type):
            for b_num in xrange(12, 24):
                direct_list.append(int(param[b_num]))
        return direct_list

    def extract_indirect_blocks(self, param, file_type):
        create_indirect_list()
        if check_file_type(file_type):
            for b_num in xrange(24, 27):
                indirect_list.append(int(param[b_num]))
        return indirect_list

    def __init__(self, param):
        #argument 0 is the word INODE
        self.inode_number = int(param[1])
        self.file_type = param[2]
        self.mode = int(param[3])
        self.owner = int(param[4])
        self.group = int(param[5])
        self.link_count = int(param[6])
        self.time_last_inode_change = param[7]
        self.modify_time = param[8]
        self.access_time = param[9]
        self.file_size = int(param[10])
        self.num_of_blocks_disk = int(param[11])
        #need special handling for reading the blocks and pointers
        self.dirent_entries = self.extract_dirent_blocks(param, self.file_type)
        self.indirect_entries = self.extract_indirect_blocks(param, self.file_type)

#craete the dirent class with parameters
class dirent:
    def __init__(self, param):
        #argument 0 is the word DIRENT
        self.parent_inode_num = int(param[1])
        self.byte_offset = int(param[2])
        self.inode_num_referenced = int(param[3])
        self.entry_length = int(param[4])
        self.name_length = int(param[5])
        self.name_string = str(param[6])

#create the indirect class with parameters
class indirect:
    def __init__(self, param):
        #argument 0 is the word INDIRECT
        self.inode_num = int(param[1])
        self.level_indirection = int(param[2])
        self.block_offset = int(param[3])
        self.indirect_block_num = int(param[4])
        self.referenced_block_num = int(param[5])

#create a superblock class with parameters
class super_block:
    def __init__(self, param):
        #argument 0 is the word SUPERBLOCK
        self.total_num_of_blocks = int(param[1])
        self.total_num_of_inodes = int(param[2])
        self.block_size = int(param[3])
        self.inode_size = int(param[4])
        self.blocks_per_group = int(param[5])
        self.inodes_per_group = int(param[6])
        self.first_inode = int(param[7])

#create a group class with parameters, just need the first 5, the rest are not needed
class group:
    def __init__(self, param):
        #argument 0 is the word GROUP
        self.group_number = int(param[1])
        self.total_num_of_blocks = int(param[2])
        self.total_num_of_inodes = int(param[3])
        self.num_of_free_blocks = int(param[4])
        self.num_of_free_inodes = int(param[5])
        #might need to read in 6, 7
        self.first_block_inodes = int(param[8])

def insert_to_duplicate_block_list(b_num):
    duplicate_block_list.add(b_num)


def find_non_reserved_pos():
    output = group_desc.first_block_inodes + (group_desc.total_num_of_inodes * super_block_elem.inode_size / super_block_elem.block_size)
    return output

def check_b_num_not_zero(b_num):
    return b_num != 0

def check_b_num_less_zero(b_num):
    return b_num < 0

def append_reference_block_list(refer_block_number, offset_num, number, b_num):
    referenced_block_list[b_num].append((refer_block_number, offset_num, number))

def assign_reference_block_list(refer_block_number, offset_num, number, b_num):
    referenced_block_list[b_num] = [(refer_block_number, offset_num, number)]

def check_b_num_with_block_count(b_num, total_block_count):
    return total_block_count < b_num

def check_b_num_with_first_pos(b_num, first_block_pos):
    return first_block_pos > b_num

def handle_single_indirect_block(total_block_count, first_block_pos, inode):
    #get the new block number
    zero = 0
    b_num = inode.indirect_entries[0]

    if check_b_num_not_zero(b_num):
        if check_b_num_less_zero(b_num) or check_b_num_with_block_count(b_num, total_block_count):
            invalid_block_number = inode.inode_number
            print("INVALID INDIRECT BLOCK {} IN INODE {} AT OFFSET 12".format(b_num, invalid_block_number))
            bad_param = True
        
        elif check_b_num_with_first_pos(b_num, first_block_pos):
            reserved_block_number = inode.inode_number
            print("RESERVED INDIRECT BLOCK {} IN INODE {} AT OFFSET 12".format(b_num, reserved_block_number))
            bad_param = True
        
        elif b_num in free_block_list:
            print("ALLOCATED BLOCK {} ON FREELIST".format(b_num))
            bad_param = True
        
        elif b_num in referenced_block_list and check_b_num_not_zero(b_num):
            one = 1
            offset = 12
            refer_block_number = inode.inode_number
            append_reference_block_list(refer_block_number, offset, one, b_num)
            insert_to_duplicate_block_list(b_num)
        
        elif b_num not in referenced_block_list and check_b_num_not_zero(b_num):
            one = 1
            offset = 12
            refer_block_number = inode.inode_number
            assign_reference_block_list(refer_block_number, offset, one, b_num)


def handle_double_indirect_block(total_block_count, first_block_pos, inode):
    #get the new block number
    one = 1
    b_num = inode.indirect_entries[one]
    
    if check_b_num_not_zero(b_num):
        if check_b_num_less_zero(b_num) or check_b_num_with_block_count(b_num, total_block_count):
            invalid_block_number = inode.inode_number
            print("INVALID DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET 268".format(b_num, invalid_block_number))
            bad_param = True
        
        elif check_b_num_with_first_pos(b_num, first_block_pos):
            reserved_block_number = inode.inode_number
            print("RESERVED DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET 268".format(b_num, reserved_block_number))
            bad_param = True
        
        elif b_num in free_block_list:
            print("ALLOCATED BLOCK {} ON FREELIST".format(b_num))
            bad_param = True
        
        elif b_num in referenced_block_list and check_b_num_not_zero(b_num):
            two = 2
            offset = 268
            refer_block_number = inode.inode_number
            append_reference_block_list(refer_block_number, offset, two, b_num)
            insert_to_duplicate_block_list(b_num)
        
        elif b_num not in referenced_block_list and check_b_num_not_zero(b_num):
            two = 2
            offset = 268
            refer_block_number = inode.inode_number
            assign_reference_block_list(refer_block_number, offset, two, b_num)

def handle_triple_indirect_block(total_block_count, first_block_pos, inode):
    two = 2
    b_num = inode.indirect_entries[two]
    
    if check_b_num_not_zero(b_num):
        if check_b_num_less_zero(b_num) or check_b_num_with_block_count(b_num, total_block_count):
            invalid_block_number = inode.inode_number
            print("INVALID TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET 65804".format(b_num, invalid_block_number))
            bad_param = True
        
        elif check_b_num_with_first_pos(b_num, first_block_pos):
            reserved_block_number = inode.inode_number
            print("RESERVED TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET 65804".format(b_num, reserved_block_number))
            bad_param = True
        
        elif b_num in free_block_list:
            print("ALLOCATED BLOCK {} ON FREELIST".format(b_num))
            bad_param = True
        
        elif b_num in referenced_block_list and check_b_num_not_zero(b_num):
            three = 3
            offset = 65804
            refer_block_number = inode.inode_number
            append_reference_block_list(refer_block_number, offset, three, b_num)
            insert_to_duplicate_block_list(b_num)
        
        elif b_num not in referenced_block_list and check_b_num_not_zero(b_num):
            three = 3
            offset = 65804
            refer_block_number = inode.inode_number
            assign_reference_block_list(refer_block_number, offset, three, b_num)

def extract_level(indirect):
    return indirect.level_indirection

def find_level(indirect):
    check = extract_level(indirect)
    result = ""
    
    if check == 1:
        result = "INDIRECT"
    elif check == 2:
        result = "DOUBLE INDIRECT"
    elif check == 3:
        result = "TRIPLE INDIRECT"

    return result

def craete_string_level():
    global string_level
    string_level = ""

def check_indirect_list(total_block_count, first_block_pos):
    for indirect in indirect_elem:
        #assigned with reference block
        b_num = indirect.referenced_block_num
        #find the level of indirection
        craete_string_level()
        string_level = find_level(indirect)

        #start checking like single, double indirect entries
        if check_b_num_not_zero(b_num):
            
            if check_b_num_less_zero(b_num) or check_b_num_with_block_count(b_num, total_block_count):
                invalid_block_number = indirect.inode_num
                offset = indirect.block_offset
                print("INVALID {} BLOCK {} IN INODE {} AT OFFSET {}".format(string_level,b_num, invalid_block_number, offset))
                bad_param = True
            
            elif check_b_num_with_first_pos(b_num, first_block_pos):
                reserved_block_number = indirect.inode_num
                offset = indirect.block_offset
                print("RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}".format(string_level,b_num, reserved_block_number, offset))
                bad_param = True
            
            elif b_num in free_block_list:
                print("ALLOCATED BLOCK {} ON FREELIST".format(b_num))
                bad_param = True
            
            elif b_num in referenced_block_list and check_b_num_not_zero(b_num):
                #insert the element in the referenced list
                refer_block_number = indirect.inode_num
                offset = indirect.block_offset
                append_reference_block_list(refer_block_number, offset, string_level, b_num)
                insert_to_duplicate_block_list(b_num)
            
            elif b_num not in referenced_block_list and check_b_num_not_zero(b_num):
                #assign the element to the referenced list
                refer_block_number = indirect.inode_num
                offset = indirect.block_offset
                assign_reference_block_list(refer_block_number, offset, string_level, b_num)

def check_block_num(block_num):
    return block_num not in referenced_block_list and block_num not in free_block_list

def check_unreferenced_block(total_block_count, first_block_pos):
    #need to add one back to total block count, since subtract previously
    total_block_count = total_block_count + 1
    #start looping to check
    for block_num in range(first_block_pos, total_block_count):
        if check_block_num(block_num):
            print("UNREFERENCED BLOCK {}".format(block_num))
            bad_param = True

def check_referenced_block_list_length(block):
    output = len(referenced_block_list[block])
    return output > 1

def check_level(ref_set):
    two = 2
    compare = ref_set[two]

    if compare == 0:
        output = ""
    elif compare == 1:
        output = "INDIRECT"
    elif compare == 2:
        output = "DOUBLE INDIRECT"
    elif compare == 3:
        output = "TRIPLE INDIRECT"

    return output

def print_duplicate_block(string_block, string_inode_number, string_offset, level):
    if level != "":
        print("DUPLICATE " + level + " BLOCK " + string_block + " IN INODE " + string_inode_number + " AT OFFSET " + string_offset)
        bad_param = True
    else:
        print("DUPLICATE" + level + " BLOCK " + string_block + " IN INODE " + string_inode_number + " AT OFFSET " + string_offset)
        bad_param = True

def check_duplicate_block(total_block_count, first_block_pos):
    #loop through the duplicate list and print out the info
    for block in duplicate_block_list:
        for ref_set in referenced_block_list[block]:
            level = check_level(ref_set)
            #maybe put this into a function later on
            inode_number = ref_set[0]
            offset = ref_set[1]
                
            string_block = str(block)
            string_inode_number = str(inode_number)
            string_offset = str(offset)
                
            print_duplicate_block(string_block, string_inode_number, string_offset, level)

def set_up_matrix(zero, matrix_size):
    return [zero] * matrix_size

def set_up_parent_matrix(two, parent_matrix):
    parent_matrix[two] = two

def set_link_matrix(super_block_inodes_count):
    zero = 0
    #size is super block inodes count
    matrix_size = super_block_inodes_count
    global link_matrix
    link_matrix = set_up_matrix(zero, matrix_size)

def set_parent_matrix(super_block_inodes_count):
    zero = 0
    two = 2
    #size is super block inodes count
    matrix_size = super_block_inodes_count
    global parent_matrix
    parent_matrix = set_up_matrix(zero, matrix_size)
    set_up_parent_matrix(two, parent_matrix)

def insert_inode_allocate_list(inode_num):
    inode_allocation_list.append(inode_num)

def check_inode_num_not_zero(inode_num):
    return inode_num != 0

def check_inode_num_zero(inode_num):
    return inode_num == 0

def check_in_free_node_list(inode_num):
    return inode_num in free_inode_list

def check_in_inode_allocation_list(i_num):
    return i_num not in inode_allocation_list


def check_allocate_inode_freelist(super_block_first_inode, super_block_inodes_count):
    #allocated inode case
    for inode_list_number in inode_list_elem:
        
        #be aware of this, use a lot at below
        inode_num = inode_list_number.inode_number

        if check_inode_num_not_zero(inode_num):
            #append into the inode allocation list
            insert_inode_allocate_list(inode_num)
            #check if it is in the freelist
            if check_in_free_node_list(inode_num):
                print("ALLOCATED INODE {} ON FREELIST".format(inode_num))
                bad_param = True
    
        #when inode_num is zero and need to be on the freelist
        elif check_inode_num_zero(inode_num) and not check_in_free_node_list(inode_num):
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(inode_num))
            bad_param = True

    #unalloctaed inode case, put in a function maybe, this is the problem
    for i_num in range(super_block_first_inode, super_block_inodes_count):
        if not check_in_free_node_list(i_num) and check_in_inode_allocation_list(i_num):
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(i_num))
            bad_param = True


def inode_allocation_audits(total_block_count, first_block_pos):
    #need the super block element first inode, first param
    super_block_first_inode = super_block_elem.first_inode
    #need the super block eleemtn inodes count, second param
    super_block_inodes_count = super_block_elem.total_num_of_inodes

    #create an allocated inode list from above
    create_inode_allocation_list()

    #set up matrix for links and parent
    set_link_matrix(super_block_inodes_count)
    set_parent_matrix(super_block_inodes_count)

    #checking valid inode number and on freelist or not on freelist
    check_allocate_inode_freelist(super_block_first_inode, super_block_inodes_count)

def check_directory_inode(directory_inode, block_inode_count):
    return block_inode_count < directory_inode

def check_directory_inode_allocation_list(directory_inode):
    return directory_inode in inode_allocation_list

def check_link_matrix(inode_num, inode_link_num):
    return link_matrix[inode_num] == inode_link_num

def handle_link_count():
    #need to handle the link count, loop through the inode list elem
    for inode_elem in inode_list_elem:
        inode_link_num = inode_elem.link_count
        inode_num = inode_elem.inode_number
        if not check_link_matrix(inode_num, inode_link_num):
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode_num, link_matrix[inode_num], inode_link_num))
            bad_param = True

def check_dir_inode(block_inode_count, dir_inode):
    return block_inode_count >= dir_inode

def check_dir_inode_allocation_list(dir_inode):
    return dir_inode in inode_allocation_list

def check_dir_name(dir_name, check):
    return dir_name != check

def handle_parent_assign(block_inode_count):
    #take out the parent first
    for directory in dirent_elem:
        dir_name = directory.name_string
        #find directory inode
        dir_inode = directory.inode_num_referenced
        #find directory inode number
        dir_inode_num = directory.parent_inode_num
        
        if check_dir_inode(block_inode_count, dir_inode) and not check_dir_inode_allocation_list(dir_inode):
            if check_dir_name(dir_name, "'..'") and check_dir_name(dir_name, "'.'"):
                parent_matrix[dir_inode] = dir_inode_num

def check_dir_name_equal(dir_name, string):
    return dir_name == string

def check_dir_inode_with_inode_num(dir_inode, dir_inode_num):
    return dir_inode == dir_inode_num

def check_dir_inode_with_parent_matrix(dir_inode, dir_inode_num):
    return dir_inode == parent_matrix[dir_inode_num]

def handle_parent_inconsist():
    for directory in dirent_elem:
        dir_name = directory.name_string
        #find directory inode
        dir_inode = directory.inode_num_referenced
        #find directory inode number
        dir_inode_num = directory.parent_inode_num
        if check_dir_name_equal(dir_name, "'.'") and not check_dir_inode_with_inode_num(dir_inode, dir_inode_num):
            print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(dir_inode_num, dir_inode, dir_inode_num))
            bad_param = True
        
        #need another filter here
        elif dir_inode_num == 2:
            if not check_dir_inode_with_parent_matrix(dir_inode, dir_inode_num) and check_dir_name_equal(dir_name, "'..'"):
                parent_num = parent_matrix[dir_inode_num]
                print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(dir_inode_num, dir_inode, parent_num))
                bad_param = True

def directory_consistency_audits():
    
    block_inode_count = super_block_elem.total_num_of_inodes
    
    #need to handle the dirent element (put into function later)
    for directory in dirent_elem:
        directory_inode = directory.inode_num_referenced
        #maybe need to flip the two cases later
        
        if check_directory_inode(directory_inode, block_inode_count):
            #find the directory inode number
            dir_inode_num = directory.parent_inode_num
            #find the directory name
            dir_name = directory.name_string
            #find the directory inode
            dir_inode = directory.inode_num_referenced
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(dir_inode_num, dir_name, dir_inode))
            bad_param = True
        
        elif not check_directory_inode_allocation_list(directory_inode):
            #find the directory inode number
            dir_inode_num = directory.parent_inode_num
            #find the directory name
            dir_name = directory.name_string
            #find the directory inode
            dir_inode = directory.inode_num_referenced
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(dir_inode_num, dir_name, dir_inode))
        else:
            link_matrix[directory_inode] = link_matrix[directory_inode] + 1

    handle_link_count()

    handle_parent_assign(block_inode_count)

    handle_parent_inconsist()

def check_symbolic_link(inode):
    symbolic_link = 's'
    file_type = inode.file_type
    return file_type == symbolic_link

def check_file_size(inode, total_block_count):
    file_size = inode.file_size
    return total_block_count > file_size

def find_block_count():
    output = super_block_elem.total_num_of_blocks - 1
    return output

def block_data_process():
    #block consistency audits
    #find the maximum block count
    total_block_count = find_block_count()
    
    #find the position of first non-reserved block
    first_block_pos = find_non_reserved_pos()
    
    #create a referenced block list for objects
    create_referenced_block_list()
    
    #create a duplicate block list for objects
    create_duplicate_block_list()
    
    #loop through the inode table by the inode list element
    for inode in inode_list_elem:

        offset_num = 0
        if check_symbolic_link(inode) and check_file_size(inode, total_block_count):
            continue

        for b_num in inode.dirent_entries:
            #check for invalid blocks
            if check_b_num_not_zero(b_num):
                
                if check_b_num_less_zero(b_num) or b_num > total_block_count:
                    invalid_block_number = inode.inode_number
                    print("INVALID BLOCK {} IN INODE {} AT OFFSET {}".format(b_num, invalid_block_number, offset_num))
                    bad_param = True
                
                elif b_num < first_block_pos:
                    reserved_block_number = inode.inode_number
                    print("RESERVED BLOCK {} IN INODE {} AT OFFSET {}".format(b_num, reserved_block_number, offset_num))
                    bad_param = True
                
                elif b_num in free_block_list:
                    print("ALLOCATED BLOCK {} ON FREELIST".format(b_num))
                    bad_param = True
                
                elif b_num in referenced_block_list and check_b_num_not_zero(b_num):
                    #insert the element in the referenced list
                    zero = 0
                    refer_block_number = inode.inode_number
                    
                    #create the function call here
                    append_reference_block_list(refer_block_number, offset_num, zero, b_num)
                    insert_to_duplicate_block_list(b_num)
                
                elif b_num not in referenced_block_list and check_b_num_not_zero(b_num):
                    #assign the element to the referenced list
                    zero = 0
                    refer_block_number = inode.inode_number
                    assign_reference_block_list(refer_block_number, offset_num, zero, b_num)

            offset_num = offset_num + 1

        #check the single indirect block references
        handle_single_indirect_block(total_block_count, first_block_pos, inode)

        #check the double indirect block references
        handle_double_indirect_block(total_block_count, first_block_pos, inode)

        #check the triple indirect block references
        handle_triple_indirect_block(total_block_count, first_block_pos, inode)

    #checking the indirect element list
    check_indirect_list(total_block_count, first_block_pos)

    #checking for unreferenced block
    check_unreferenced_block(total_block_count, first_block_pos)

    #checking for duplicate block
    check_duplicate_block(total_block_count, first_block_pos)

    #inode allocation audits
    inode_allocation_audits(total_block_count, first_block_pos)

    #directory consistency audits
    directory_consistency_audits()

def handle_exit():
    if bad_param == True:
        exit(2)
    else:
        exit(0)

if __name__ == '__main__':
    #check length of argument and print out error messages and exit codes
    if (len(sys.argv)) != 2:
        error_msg_func("Incorrect arguments number. Proper usage : ./lab3b file_name\n")

    #make the csv file global for convenient access
    global csv_file

    #try open the pass in csv file, read only
    #Based from resources : https://realpython.com/python-csv/
    try:
        csv_file = open(sys.argv[1], "r");
    except:
        error_msg_func("Passed in csv file cannot be opened\n")

    #start reading the csv file line by line
    #Based from resources : https://realpython.com/python-csv/
    csv_reader = csv.reader(csv_file)

    #process each row from the file
    for row in csv_reader:
        #first slot is the type of the group
        info_type = row[0]
        if row[0] == "SUPERBLOCK":
            #need the total blocks and total inodes in the superblock elem
            super_block_elem = super_block(row)
        elif row[0] == "GROUP":
            group_desc = group(row)
        elif row[0] == "IFREE":
            #append to the free inodes list
            free_inode_list.append(int(row[1]))
        elif row[0] == "INODE":
            #append to the inode elem list
            inode_list_elem.append(inode(row))
        elif row[0] == "BFREE":
            free_block_list.append(int(row[1]))
        #append to the free blocks list
        elif row[0] == "INDIRECT":
            #append to the indirect element list
            indirect_elem.append(indirect(row))
        elif row[0] == "DIRENT":
            #append to the dirent element list
            dirent_elem.append(dirent(row))

    #function that handle the data in blocks and printing
    block_data_process()

    #handle exit properly
    handle_exit()

