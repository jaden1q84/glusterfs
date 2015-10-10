/*
  Copyright (c) 2008-2012 Red Hat, Inc. <http://www.redhat.com>
  This file is part of GlusterFS.

  This file is licensed to you under your choice of the GNU Lesser
  General Public License, version 3 or any later version (LGPLv3 or
  later), or the GNU General Public License, version 2 (GPLv2), in all
  cases as published by the Free Software Foundation.
*/

#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include "list.h"
#include "locking.h"
#include "logging.h"
#include "mem-types.h"
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>


struct mem_acct {
        uint32_t            num_types;
        struct mem_acct_rec     *rec;
};

struct mem_acct_rec {
        size_t          size;
        size_t          max_size;
        uint32_t        num_allocs;
        uint32_t        total_allocs;
        uint32_t        max_num_allocs;
        gf_lock_t       lock;
};


void *
__gf_calloc (size_t cnt, size_t size, uint32_t type);

void *
__gf_malloc (size_t size, uint32_t type);

void *
__gf_realloc (void *ptr, size_t size);

int
gf_vasprintf (char **string_ptr, const char *format, va_list arg);

int
gf_asprintf (char **string_ptr, const char *format, ...);

void
__gf_free (void *ptr);


static inline
void* __gf_default_malloc (size_t size)
{
        void *ptr = NULL;

        ptr = malloc (size);
        if (!ptr)
                gf_log_nomem ("", GF_LOG_ALERT, size);

        return ptr;
}

static inline
void* __gf_default_calloc (int cnt, size_t size)
{
        void *ptr = NULL;

        ptr = calloc (cnt, size);
        if (!ptr)
                gf_log_nomem ("", GF_LOG_ALERT, (cnt * size));

        return ptr;
}

static inline
void* __gf_default_realloc (void *oldptr, size_t size)
{
        void *ptr = NULL;

        ptr = realloc (oldptr, size);
        if (!ptr)
                gf_log_nomem ("", GF_LOG_ALERT, size);

        return ptr;
}

#define MALLOC(size)       __gf_default_malloc(size)
#define CALLOC(cnt,size)   __gf_default_calloc(cnt,size)
#define REALLOC(ptr,size)  __gf_default_realloc(ptr,size)

#define FREE(ptr)                               \
        if (ptr != NULL) {                      \
                free ((void *)ptr);             \
                ptr = (void *)0xeeeeeeee;       \
        }

#define GF_CALLOC(nmemb, size, type) __gf_calloc (nmemb, size, type)

#define GF_MALLOC(size, type)  __gf_malloc (size, type)

#define GF_REALLOC(ptr, size)  __gf_realloc (ptr, size)

#define GF_FREE(free_ptr) __gf_free (free_ptr)

static inline
char *gf_strndup (const char *src, size_t len)
{
        char *dup_str = NULL;

        if (!src) {
                goto out;
        }

        dup_str = GF_CALLOC (1, len + 1, gf_common_mt_strdup);
        if (!dup_str) {
                goto out;
        }

        memcpy (dup_str, src, len);
out:
        return dup_str;
}

static inline
char * gf_strdup (const char *src)
{

        char    *dup_str = NULL;
        size_t  len = 0;

        len = strlen (src) + 1;

        dup_str = GF_CALLOC(1, len, gf_common_mt_strdup);

        if (!dup_str)
                return NULL;

        memcpy (dup_str, src, len);

        return dup_str;
}

static inline void *
gf_memdup (const void *src, size_t size)
{
        void *dup_mem = NULL;

        dup_mem = GF_CALLOC(1, size, gf_common_mt_strdup);
        if (!dup_mem)
                goto out;

        memcpy (dup_mem, src, size);

out:
        return dup_mem;
}

struct mem_pool {
        struct list_head  list;				/* 内存池空闲chunk列表，和cold_count对应 */
        int               hot_count;		/* 从内存池分配出去的个数 */
        int               cold_count;		/* 内存池未分配出去的个数 */
        gf_lock_t         lock;				/* 内存池的锁，get、put都要加 */
        unsigned long     padded_sizeof_type;/* 补充mem_pool边界位置后的块大小，即使走stdalloc也是按这个大小分配，因此整个池在用空间应该是 (hot_count + cold_count + curr_stdalloc) * padded_sizeof_type */
        void             *pool;				/* 内存池起始位置，可用来判断一块内存是否从池分配 */
        void             *pool_end;			/* 内存池结束位置 */
        int               real_sizeof_type; /* 请求的块大小，会被pad成padded_sizeof_type */
        uint64_t          alloc_count;		/* 总共分配过的次数，包括池内分配和走标准分配，不会减少 */
        uint64_t          pool_misses;		/* 内存池用满，无法分配的次数统计 */
        int               max_alloc;		/* 从池中分配出去的在用chunk数峰值 */
        int               curr_stdalloc;	/* 无法从池中分配时，会走标准分配，当前走标准分配的个数，put回来时减少 */
        int               max_stdalloc;		/* 无法从池中分配的最大峰值次数 */
        char             *name;				/* 池名字 */
        struct list_head  global_list;		/* 链到指定ctx的 mempool_list 里面 */
};

struct mem_pool *
mem_pool_new_fn (unsigned long sizeof_type, unsigned long count, char *name);

#define mem_pool_new(type,count) mem_pool_new_fn (sizeof(type), count, #type)

void mem_put (void *ptr);
void *mem_get (struct mem_pool *pool);
void *mem_get0 (struct mem_pool *pool);

void mem_pool_destroy (struct mem_pool *pool);

void gf_mem_acct_enable_set (void *ctx);

#endif /* _MEM_POOL_H */
