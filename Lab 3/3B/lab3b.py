#!/usr/bin/env python3
# NAME: Kevin Li
# EMAIL: li.kevin512@gmail.com
# ID: 123456789

import sys
import csv
from collections import defaultdict
from math import ceil


inconsistent = False

def inconsistency(*args, **kwargs):
    global inconsistent
    inconsistent = True
    print(*args, **kwargs)

class Data:
    def __init__(self, var_vals, var_names, var_types):
        for var, val, cast in zip(var_names, var_vals, var_types):
            setattr(self, var, cast(val))


class Superblock(Data):
    def __init__(self, params):
        if len(params) != 8:
            raise Exception('')
        member_vars = ('blocks', 'inodes', 'block_size',
                       'inode_size', 'bpg', 'ipg', 'first_ino')
        types = (int, int, int, int, int, int, int)
        super().__init__(params[1:], member_vars, types)


class Group(Data):
    def __init__(self, params):
        member_vars = ('group', 'blocks', 'inodes', 'free_blocks',
                       'free_inodes', 'block_bitmap', 'inode_bitmap', 'first_ino')
        types = (int, int, int, int, int, int, int, int)
        super().__init__(params[1:], member_vars, types)


class Inode(Data):
    ind_block = 12
    dind_block = 13
    tind_block = 14

    def __init__(self, params):
        member_vars = ('inode', 'type', 'mode', 'uid', 'gid', 'link_count',
                       'creation_time', 'modify_time', 'access_time', 'size')
        types = (int, str, int, int, int, int, str, str, str, int)
        super().__init__(params[1:], member_vars, types)
        self.blocks = tuple(map(int, params[12:]))


class DirEnt(Data):
    def __init__(self, params):
        member_vars = ('parent_ino', 'offset', 'file_ino',
                       'entlen', 'namelen', 'name')
        types = (int, int, int, int, int, str)
        super().__init__(params[1:], member_vars, types)


class Indirect(Data):
    def __init__(self, params):
        member_vars = ('inode', 'indirection_level', 'offset',
                       'parent_block', 'child_block')
        types = (int, int, int, int, int, int, int)
        super().__init__(params[1:], member_vars, types)


class FileSystemChecker:
    def __init__(self, filename):
        self.superblock = None
        self.group = None
        self.bfree = set()
        self.ifree = set()
        self.inodes = []
        self.dirents = []
        self.indirects = []
        with open(filename) as infile:
            csv_file = csv.reader(infile)
            for line in csv_file:
                name = line[0]
                if name == 'SUPERBLOCK':
                    self.superblock = Superblock(line)
                elif name == 'GROUP':
                    self.group = Group(line)
                elif name == 'BFREE':
                    self.bfree.add(int(line[1]))
                elif name == 'IFREE':
                    self.ifree.add(int(line[1]))
                elif name == 'INODE':
                    self.inodes.append(Inode(line))
                elif name == 'DIRENT':
                    self.dirents.append(DirEnt(line))
                elif name == 'INDIRECT':
                    self.indirects.append(Indirect(line))

        self.reserved_blocks = ceil(
            self.group.first_ino + self.superblock.inode_size * self.group.inodes / self.superblock.block_size)

    def _check_block(self, block_info):
        block = block_info[1]
        if block < 0 or block >= self.superblock.blocks:
            inconsistency('INVALID {}BLOCK {} IN INODE {} AT OFFSET {}'.format(*block_info))
        elif block > 0 and block < self.reserved_blocks:
            inconsistency('RESERVED {}BLOCK {} IN INODE {} AT OFFSET {}'.format(*block_info))
        else:
            return block

        return -1

    def block_consistency_audits(self):
        allocated_blocks = defaultdict(lambda: [])
        indirections = ['', 'INDIRECT ', 'DOUBLE INDIRECT ', 'TRIPLE INDIRECT ']
        max_block = self.superblock.blocks

        for inode in self.inodes:
            if inode.type == 's' and inode.size <= 60:
                continue
            for offset, block in enumerate(inode.blocks):
                indirection = indirections[0]
                if offset == Inode.ind_block:
                    indirection = indirections[1]
                elif offset == Inode.dind_block:
                    indirection = indirections[2]
                    offset = 12 + 256
                elif offset == Inode.tind_block:
                    indirection = indirections[3]
                    offset = 12 + 256 + (256 * 256)

                block_info = (indirection, block, inode.inode, offset)
                if self._check_block(block_info) > 0:
                    allocated_blocks[block].append(block_info)

        for indirect in self.indirects:
            block = indirect.child_block
            indirection = indirections[indirect.indirection_level]

            block_info = (indirection, block, indirect.inode, indirect.offset)
            if self._check_block(block_info) > 0:
                allocated_blocks[block].append(block_info)

        for block in range(self.reserved_blocks, self.superblock.blocks):
            if block not in self.bfree and len(allocated_blocks[block]) == 0:
                inconsistency('UNREFERENCED BLOCK {}'.format(block))
            if block in self.bfree and len(allocated_blocks[block]) != 0:
                inconsistency('ALLOCATED BLOCK {} ON FREELIST'.format(block))
            if len(allocated_blocks[block]) > 1:
                for duplicate_block in allocated_blocks[block]:
                    inconsistency('DUPLICATE {}BLOCK {} IN INODE {} AT OFFSET {}'.format(
                        *duplicate_block))

    def inode_allocation_audits(self):
        self.allocated_inodes = set()
        for inode in self.inodes:
            self.allocated_inodes.add(inode.inode)
            if inode.inode in self.ifree:
                inconsistency('ALLOCATED INODE {} ON FREELIST'.format(inode.inode))

        for inode in range(self.superblock.first_ino, self.superblock.inodes):
            if inode not in self.allocated_inodes and inode not in self.ifree:
                inconsistency('UNALLOCATED INODE {} NOT ON FREELIST'.format(inode))

    def _valid_dirent(self, dirent):
        if dirent.file_ino < 1 or dirent.file_ino > self.superblock.inodes:
            inconsistency('DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(
                dirent.parent_ino, dirent.name, dirent.file_ino))
        elif dirent.file_ino not in self.allocated_inodes:
            inconsistency('DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}'.format(
                dirent.parent_ino, dirent.name, dirent.file_ino))
        else:
            return True
        return False

    def directory_consistency_audits(self):
        link_counts = defaultdict(lambda: 0)
        parent = {}
        for dirent in self.dirents:
            if self._valid_dirent(dirent):
                link_counts[dirent.file_ino] += 1
                if dirent.name != "'.'" and dirent.name != "'..'" :
                    parent[dirent.file_ino] = dirent.parent_ino

        for inode in self.inodes:
            if inode.link_count != link_counts[inode.inode]:
                inconsistency('INODE {} HAS {} LINKS BUT LINKCOUNT IS {}'.format(
                    inode.inode, link_counts[inode.inode], inode.link_count))

        for dirent in self.dirents:
            if dirent.name == "'.'":
                if dirent.file_ino != dirent.parent_ino:
                    inconsistency("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(
                        dirent.parent_ino, dirent.file_ino, dirent.parent_ino))
            elif dirent.name == "'..'":
                try:
                    if dirent.file_ino != parent[dirent.parent_ino]:
                        inconsistency("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(
                            dirent.parent_ino, dirent.file_ino, parent[dirent.parent_ino]))
                except KeyError:
                    if dirent.file_ino != dirent.parent_ino:
                        inconsistency("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(
                            dirent.parent_ino, dirent.file_ino, dirent.parent_ino))


def main():
    if len(sys.argv) != 2:
        print('usage: lab3b [CSVFILE]', file=sys.stderr)
        sys.exit(1)

    global inconsistent

    fs = FileSystemChecker(sys.argv[1])
    fs.block_consistency_audits()
    fs.inode_allocation_audits()
    fs.directory_consistency_audits()

    if inconsistent:
        sys.exit(2)
    sys.exit(0)


if __name__ == '__main__':
    main()
