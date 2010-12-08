/* $Id: mem.h,v 1.23 2006/03/12 20:30:38 jirka Exp $ */
/*
 * Memory management.
 * Copyright (C) 2003 Jiri Denemark
 *
 * This file is part of RUM2.
 *
 * RUM2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** @file
 * Memory management.
 * @author Jiri Denemark
 * @date 2003
 * @version $Revision: 1.23 $ $Date: 2006/03/12 20:30:38 $
 */

#ifndef MEM_H
#define MEM_H

#include "error.h"
#include "module.h"


/** Initialize memory management.
 * This function must be called before any of the other functions concerning
 * memory management.
 *
 * @return
 *      nothing.
 */
extern void memory_management_init(void);


/** Initialize memory management module.
 * @sa module_initialize
 */
extern int mem_initialize(struct module *module);


/** Count real size (which is the power of 2) of a memory block.
 * Get the smallest size such that the allocated memory block will be at
 * least specified amount of bytes large.
 *
 * @param bytes
 *      the smallest size of the buffer.
 *
 * @return
 *      size of the memory block in the form of base-2 logarithm. Zero is
 *      returned when the requested size is greater than maximum size of
 *      memory block.
 */
extern int mem_size(int bytes);


/** Get base-2 logarithm of actual size of memory block.
 *
 * @param block
 *      pointer to the memory block.
 *
 * @return
 *      the actual size of block. Use <tt>1 @<@< mem_getsize()</tt> to get
 *      block size in bytes.
 */
extern int mem_getsize(const void *block);


/** Get real size (from the memory managemnt's point of view) of type.
 *
 * @param type
 *      type or variable of which the size has to be computed.
 *
 * @return
 *      size (in fact base-2 logarithm of the size) of a memory block that
 *      will be used for storing that type or variable.
 */
#define mem_sizeof(type)        mem_size(sizeof(type))


/** Get the size of memory block for storing @c n times the data of given type.
 *
 * @param n
 *      number of structures.
 *
 * @param type
 *      type or variable of which has to be stored in a memory block.
 *
 * @return
 *      size (in fact base-2 logarithm of the size) of a memory block that
 *      will be used for storing @c n times the data of given type.
 */
#define mem_nsizeof(n, type)    mem_size((n) * sizeof(type))


/** Request some free memory block(s).
 *
 * @param size
 *      base-2 logarihtm of size of the memory block to be allocated.
 *      The result of mem_size(), mem_sizeof() or mem_nsizeof() should be
 *      passed on as this parameter.
 *
 * @param count
 *      number of requested blocks.
 *
 * @param storage
 *      where to place pointers to the blocks.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int mem_new(EC, int size, int count, void **storage);


/** Change the size of memory block.
 * If the requested block size is the same as the actual block size, nothing
 * happens. Otherwise a new block of requested size is used and the original
 * data (or part of them when the requested size is smaller then the actual)
 * are copied into it. All bytes in the rest of the block are set to zero.
 *
 * @param ptr
 *      pointer to a place where a pointer to a memory block is stored. On
 *      success a pointer to a new block will be stored in that place. When
 *      the original pointer (i.e. *ptr) is NULL, this function just calls
 *      mem_new() and sets all bytes to zero.
 *
 * @param size
 *      base-2 logarihtm of new size of the block. The result of mem_size(),
 *      mem_sizeof() or mem_nsizeof() should be passed on as this parameter.
 *
 * @return
 *      zero on success, nonzero otherwise. In case of failure the old block
 *      and a pointer to it are left untouched.
 */
extern int mem_realloc(EC, void **ptr, int size);


/** Increment reference counter to memory block.
 * Caller MUST be absolutely sure that the memory block supplied was obtained
 * by calling mem_new(), otherwise really BAD things will happen.
 *
 * @param mem
 *      memory block.
 *
 * @return
 *      nothing.
 */
extern void mem_ref(void *mem);


/** Function for copying memory block with pointers.
 * This function is used by mem_copy_struct() function for deep-copy of
 * complex data structures stored in memory blocks allocated by mem_new().
 *
 * @param orig
 *      pointer to the original memory block (content of which must not be
 *      touched in any way).
 *
 * @param copy
 *      pointer to the memory block where a copy of the original block should
 *      be stored.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
typedef int (* mem_copy_function)(EC, const void *orig, void *copy);


/** Copy memory block obtained by mem_new() used for structs with pointers.
 * The memory is copied iff the number references to the original memory block
 * is greater than one. In this case, the original reference counter is
 * decreased by one. If no copy is performed (because it is not needed), the
 * pointer to the original block is returned.
 *
 * @param mem
 *      pointer to the memory block to be copied.
 *
 * @param copy_struct
 *      pointer to a function for deep-copying the memory block. If this
 *      function is NULL, mem_copy_struct() is equivalent to mem_copy().
 *
 * @return
 *      pointer to a copy of the original memory block or NULL on error.
 */
extern void *mem_copy_struct(EC, void *mem, mem_copy_function copy_struct);


/** Copy memory block obtained by mem_new().
 * The memory is copied iff the number references to the original memory block
 * is greater than one. In this case, the original reference counter is
 * decreased by one. If no copy is performed (because it is not needed), the
 * pointer to the original block is returned.
 *
 * @param errctx
 *      pointer to an error context.
 *
 * @param mem
 *      pointer to the memory block to be copied.
 *
 * @return
 *      pointer to a copy of the original memory block or NULL on error.
 */
#define mem_copy(errctx, mem) \
    mem_copy_struct((errctx), (mem), NULL)


/** Callback for freeing memory pointed to from memory block to be freed.
 * This callback is used by mem_free_struct() function for deep-free of
 * complex data structures stored in memory blocks allocated by mem_new().
 *
 * @param data
 *      memory block to be freed.
 *
 * @return
 *      nothing.
 */
typedef void (* mem_free_function)(void *data);


/** Free memory blocks obtained by mem_new() used for structs with pointers.
 *
 * @param count
 *      number of blocks to be freed.
 *
 * @param storage
 *      pointers to memory blocks. All of these pointers are set to NULL.
 *
 * @param free_struct
 *      callback for freeing memory pointed to from each memory block which
 *      is to be freed. If this function is NULL, mem_free_struct() is
 *      equivalent to calling mem_free().
 *
 * @return
 *      nothing.
 */
extern void mem_free_struct(int count,
                            void **storage,
                            mem_free_function free_struct);


/** Free memory blocks obtained by mem_new().
 *
 * @param count
 *      number of blocks to be freed.
 *
 * @param storage
 *      pointers to memory blocks. All of these pointers are set to NULL.
 *
 * @return
 *      nothing.
 */
#define mem_free(count, storage) \
    mem_free_struct((count), (storage), NULL)

#endif

