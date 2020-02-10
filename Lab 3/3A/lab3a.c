// NAME: Kevin Li
// EMAIL: li.kevin512@gmail.com
// ID: 123456789
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "ext2_fs.h"

#define BOOT_SIZE 1024

int img;
int blocks_count, inodes_count, block_size, blocks_per_group, inodes_per_group;

/**
 * @brief Prints errors and exits
 *
 * @param msg Error message
 * @param code Exit code
 */
void error(char* msg, int code) {
  fprintf(stderr, "%s\n", msg);
  exit(code);
}

/**
 * @brief Looks through superblock to set up important values
 *
 * Additionally prints SUPERBLOCK csv data
 */
void setup() {
  struct ext2_super_block superblock;
  if (pread(img, &superblock, sizeof(struct ext2_super_block), BOOT_SIZE) == -1)
    error("Error preading from image file for superblock", 2);

  blocks_count = superblock.s_blocks_count;
  inodes_count = superblock.s_inodes_count;
  block_size = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
  blocks_per_group = superblock.s_blocks_per_group;
  inodes_per_group = superblock.s_inodes_per_group;

  printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", blocks_count, inodes_count,
         block_size, superblock.s_inode_size, blocks_per_group,
         inodes_per_group, superblock.s_first_ino);
}

/**
 * @brief Returns formatted time cstring
 *
 * @param time Time in integer format
 * @return char* Time in string format. Must be free()'d
 */
char* timestr(unsigned int time) {
  time_t rawtime = time;
  struct tm* time_info = gmtime(&rawtime);

  char* date = malloc(sizeof(char) * 32);
  strftime(date, 32, "%m/%d/%y %H:%M:%S", time_info);
  return date;
}

/**
 * @brief Recursively parses and prints indirect blocks
 *
 * @param block_num block number of indirect block
 * @param ino parent inode
 * @param id_level level of indirection
 * @param offset logical offset
 */
void parse_idblock(int block_num, int ino, int id_level, int offset) {
  if (id_level == 0 || block_num == 0)
    return;  // base case; null block pointer or no longer indirect

  for (int i = 0; i < block_size / 4;
       i++) {  // recursion on all block pointers in block
    uint32_t lower_block;
    pread(img, &lower_block, 4, block_size * block_num + 4 * i);
    if (lower_block != 0) {
      offset += i;
      printf("INDIRECT,%d,%d,%d,%d,%d\n", ino, id_level, offset, block_num,
             lower_block);
      parse_idblock(lower_block, ino, id_level - 1, offset);
    }
  }
}

/**
 * @brief Recursively parses and prints direct and indirect directory blocks
 *
 * @param block_num block number
 * @param ino parent inode
 * @param id_level level of indirection
 */
void parse_dirent_block(int block_num, int ino, int id_level) {
  if (block_num == 0) return;  // empty block pointer
  if (id_level == 0) {         // base case; print dirents in this block
    struct ext2_dir_entry dirent;
    for (int offset = 0; offset < block_size; offset += dirent.rec_len) {
      pread(img, &dirent, sizeof(struct ext2_dir_entry),
            (block_size * block_num) + offset);
      if (dirent.inode != 0)
        printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", ino, offset, dirent.inode,
               dirent.rec_len, dirent.name_len, dirent.name);
    }
    return;
  }

  for (int i = 0; i < block_size / 4;
       i++) {  // recursion on all block pointers in block
    uint32_t lower_block;
    pread(img, &lower_block, 4, block_size * block_num + 4 * i);
    if (lower_block != 0) parse_dirent_block(lower_block, ino, id_level - 1);
  }
}

/**
 * @brief Parses and prints data for inodes
 *
 * @param inode inode to analyze
 * @param ino inode number
 */
void parse_inode(struct ext2_inode* inode, int ino) {
  if (inode->i_mode != 0 && inode->i_links_count != 0) {
    char filetype = '?';
    if (inode->i_mode & 0x8000)
      filetype = 'f';
    else if (inode->i_mode & 0x4000)
      filetype = 'd';
    else if (inode->i_mode & 0xA000)
      filetype = 's';

    char* creat_time = timestr(inode->i_ctime);
    char* mod_time = timestr(inode->i_mtime);
    char* access_time = timestr(inode->i_atime);
    int fsize = inode->i_size;

    printf("INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", ino, filetype,
           inode->i_mode & 0x0FFF, inode->i_uid, inode->i_gid,
           inode->i_links_count, creat_time, mod_time, access_time, fsize,
           inode->i_blocks);

    // have to free the cstrings made by timestr()
    free(creat_time);
    free(mod_time);
    free(access_time);

    if (filetype == 's' && fsize <= 60)
      return;  // cant have indirection or directory entries on small softlink
    else
      for (int b = 0; b < EXT2_N_BLOCKS; b++) printf(",%d", inode->i_block[b]);
    printf("\n");

    int block_num;
    if (filetype == 'd') {
      for (int i = 0; i < EXT2_NDIR_BLOCKS; i++) {
        block_num = inode->i_block[i];
        if (block_num != 0) parse_dirent_block(block_num, ino, 0);
      }
      parse_dirent_block(inode->i_block[EXT2_IND_BLOCK], ino, 1);
      parse_dirent_block(inode->i_block[EXT2_DIND_BLOCK], ino, 2);
      parse_dirent_block(inode->i_block[EXT2_TIND_BLOCK], ino, 3);
    }

    int base_offset = 12;
    parse_idblock(inode->i_block[EXT2_IND_BLOCK], ino, 1, base_offset);
    base_offset += block_size / 4;
    parse_idblock(inode->i_block[EXT2_DIND_BLOCK], ino, 2, base_offset);
    base_offset += (block_size / 4) * (block_size / 4);
    parse_idblock(inode->i_block[EXT2_TIND_BLOCK], ino, 3, base_offset);
  }
}

/**
 * @brief Parses a group for information
 *
 * Because end groups can be partial groups, block and inode
 * count are asked for. Additionally prints almost all the required
 * csv data
 *
 * @param group group type from ext2_fs.h
 * @param i group number
 * @param blocks blocks in group
 * @param inodes inodes in group
 */
void parse_group(struct ext2_group_desc* group, int i, int blocks, int inodes) {
  int block_bitmap = group->bg_block_bitmap;
  int inode_bitmap = group->bg_inode_bitmap;
  printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", i, blocks, inodes,
         group->bg_free_blocks_count, group->bg_free_inodes_count, block_bitmap,
         inode_bitmap, group->bg_inode_table);

  uint8_t* blk_block = malloc(block_size);
  uint8_t* ino_block = malloc(block_size);
  pread(img, blk_block, block_size, block_bitmap * block_size);
  pread(img, ino_block, block_size, inode_bitmap * block_size);

  for (int byte = 0; byte < block_size; byte++)  // iterates through bitmaps
    for (int bit = 0; bit < 8; bit++) {
      if (((blk_block[byte] >> bit) & 1) == 0)
        printf("BFREE,%d\n", (i * blocks_per_group) + (byte * 8) + bit + 1);
      if (((ino_block[byte] >> bit) & 1) == 0)
        printf("IFREE,%d\n", (i * inodes_per_group) + (byte * 8) + bit + 1);
    }

  struct ext2_inode inode;
  for (int ino = 0; ino < inodes; ino++) {  // parses each inode
    pread(img, &inode, sizeof(struct ext2_inode),
          BOOT_SIZE + ((blocks_per_group * i + 4) * block_size) +
              (ino * sizeof(struct ext2_inode)));
    parse_inode(&inode, ino + 1);
  }
}

/**
 * @brief Breaks the filesystem into groups, and calls parse_group() on them
 *
 */
void parse_main() {
  int groups = blocks_count / blocks_per_group + 1;
  struct ext2_group_desc group;
  int blocks = blocks_per_group;
  int inodes = inodes_per_group;
  for (int i = 0; i < groups; i++) {
    if (pread(img, &group, sizeof(struct ext2_group_desc),
              BOOT_SIZE + sizeof(struct ext2_super_block) +
                  i * sizeof(struct ext2_group_desc)) == -1)
      error("Error preading group descriptor", 2);

    if (i == groups - 1) {  // accounting for partial groups at the end
      if (blocks_count % blocks_per_group != 0)
        blocks = blocks_count % blocks_per_group;
      if (inodes_count % inodes_per_group != 0)
        inodes = inodes_count % inodes_per_group;
    }
    parse_group(&group, i, blocks, inodes);
  }
}

/**
 * @brief Parses a file system image as specified
 *
 * @return int Exits with 0 if successful, 1 if invalid args, 2 if corruption or
 * other exceptions
 */
int main(int argc, char** argv) {
  if (argc != 2) error("Invalid number of arguments. Usage: lab3a IMGFILE", 1);
  if ((img = open(argv[1], O_RDONLY)) < 0) error("Error opening image file", 2);

  setup();
  parse_main();
  exit(0);
}