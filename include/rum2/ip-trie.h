/* $Id: ip-trie.h,v 1.4 2004/09/14 11:20:08 jirka Exp $ */
/*
 * Trie structure for Best Matching Prefix lookup problem.
 * Copyright (C) 2004 Jiri Denemark
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
 * Trie structure for Best Matching Prefix lookup problem.
 * @author Jiri Denemark
 * @date 2004
 * @version $Revision: 1.4 $ $Date: 2004/09/14 11:20:08 $
 */

#ifndef IP_TRIE_H
#define IP_TRIE_H

#if HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "limits.h"
#include "utils.h"
#include "error.h"


/** Maximum value that fits into RUM_TRIE_BITS bits.
 * That is 2^RUM_TRIE_BITS - 1. */
#define TRIE_MAX ((1 << RUM_TRIE_BITS) - 1)

/** Bit mask for single-level part of prefix. */
#define TRIE_MASK TRIE_MAX


/** IP-trie structure. */
struct ip_trie {
    /** Pointer to the root node of IP-trie. */
    struct ip_trie_node *root;
    /** Some mutexes will follow. */
};


/** One node of IP-trie structure. */
struct ip_trie_node {
    /** Pointer to the parent node. */
    struct ip_trie_node *parent;

    /** Index to a pointer in parent->next[] which leads to this node. */
    int index;

    /** Number of significant bits of upper-level part of the prefix.
     * In fact, this is max(RUM_TRIE_BITS, prefix - level * RUM_TRIE_BITS).
     *
     * If the value is less than RUM_TRIE_BITS this node has no successors,
     * i.e. all pointer in next[] array are NULL-pointers. The only exception
     * is 0-level node (i.e. the root of a trie) which has no data pointers and
     * bits = 0 while it can have any number of successors. */
    int bits;

    /** Data pointers for any prefix length that can lead to this node.
     * This array is indexed by (bits - 1). */
    void *data[RUM_TRIE_BITS];

    /** Pointers to the nodes in next level. */
    struct ip_trie_node *next[1 << RUM_TRIE_BITS];
};


/** Create and initialize new IP trie structure.
 *
 * @return
 *      pointer to the new trie or NULL on error.
 */
extern struct ip_trie *ip_trie_init(EC);


/** Free all the memory occupied by IP trie structure.
 *
 * @param trie
 *      trie structure to be freed.
 *
 * @return
 *      nothing.
 */
extern void ip_trie_free(struct ip_trie *trie);


/** Insert new IP prefix into a trie.
 * In case the prefix which is to be inserted already exists in the trie
 * the data associated with the prefix will be replaced.
 *
 * @param trie
 *      IP trie into which new prefix is to be inserted.
 *
 * @param ip
 *      prefix to be inserted.
 *
 * @param prefix
 *      number of significant bits in ip (i.e. number of bits in the prefix).
 *
 * @param data
 *      pointer to the data associated with the prefix.
 *
 * @return
 *      zero on success, nonzero otherwise.
 */
extern int ip_trie_insert(EC, struct ip_trie *trie,
                          const IN_ADDR *ip,
                          int prefix,
                          void *data);


/** Remove IP prefix from a trie.
 *
 * @param trie
 *      IP trie from which a prefix is to be removed.
 *
 * @param ip
 *      prefix to be removed.
 *
 * @param prefix
 *      number of significant bits in ip (i.e. number of bits in the prefix).
 *
 * @return
 *      nothing.
 */
extern void ip_trie_remove(struct ip_trie *trie,
                           const IN_ADDR *ip,
                           int prefix);


/** Find the best matching prefix of IP address.
 *
 * @param trie
 *      trie to be searched.
 *
 * @param ip
 *      IP address to be found.
 *
 * @return
 *      pointer to the data associated with BMP or NULL if nothing was found.
 */
extern void *ip_trie_find(struct ip_trie *trie, const IN_ADDR *ip);


/** Function used for checking relevance of data associated with some prefix.
 *
 * @param data
 *      data to be checked for relevance.
 *
 * @param arg
 *      additional argument passed to ip_trie_find_check().
 *
 * @return
 *      zero if the prefix and its data have to be ignored, nonzero when they
 *      are relevant.
 */
typedef int (*ip_trie_check_fn)(const void *data, void *arg);


/** Find the best matching prefix of IP address.
 * Unlike ip_trie_find() this function will ignore all the prefixes (and data
 * associated with them) that the check() function marks as irrelevant.
 *
 * @param trie
 *      trie to be searched.
 *
 * @param ip
 *      IP address to be found.
 *
 * @param check
 *      function used for checking data relevance.
 *
 * @param arg
 *      additional argument to be passed through to check() function.
 *
 * @return
 *      pointer to the data associated with BMP or NULL if nothing was found.
 */
extern void *ip_trie_find_check(struct ip_trie *trie,
                                const IN_ADDR *ip,
                                ip_trie_check_fn check,
                                void *arg);


/** Find IP prefix (using exact match).
 *
 * @param trie
 *      trie to be searched.
 *
 * @param ip
 *      IP prefix to be found.
 *
 * @param prefix
 *      number of bits in the prefix.
 *
 * @return
 *      pointer to the data associated with prefix or NULL if nothing was
 *      found.
 */
extern void *ip_trie_find_prefix(struct ip_trie *trie,
                                 const IN_ADDR *ip,
                                 int prefix);


#endif

