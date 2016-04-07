/*
  Copyright (c) 2008-2012 Red Hat, Inc. <http://www.redhat.com>
  This file is part of GlusterFS.

  This file is licensed to you under your choice of the GNU Lesser
  General Public License, version 3 or any later version (LGPLv3 or
  later), or the GNU General Public License, version 2 (GPLv2), in all
  cases as published by the Free Software Foundation.
*/

#ifndef __IOT_H
#define __IOT_H

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif


#include "compat-errno.h"
#include "glusterfs.h"
#include "logging.h"
#include "dict.h"
#include "xlator.h"
#include "common-utils.h"
#include "list.h"
#include <stdlib.h>
#include "locking.h"
#include "iot-mem-types.h"
#include <semaphore.h>
#include "statedump.h"


struct iot_conf;

#define MAX_IDLE_SKEW                   4       /* In secs */
#define skew_sec_idle_time(sec)         ((sec) + (random () % MAX_IDLE_SKEW))
#define IOT_DEFAULT_IDLE                120     /* In secs. */

#define IOT_MIN_THREADS         1
#define IOT_DEFAULT_THREADS     16
#define IOT_MAX_THREADS         64


#define IOT_THREAD_STACK_SIZE   ((size_t)(1024*1024))


typedef enum {
        IOT_PRI_HI = 0, /* low latency */
        IOT_PRI_NORMAL, /* normal */
        IOT_PRI_LO,     /* bulk */
        IOT_PRI_LEAST,  /* least */
        IOT_PRI_MAX,
} iot_pri_t;

#define IOT_LEAST_THROTTLE_DELAY 1	/* sample interval in seconds */
struct iot_least_throttle {
	struct timeval	sample_time;	/* timestamp of current sample 当前采样时间 */
	uint32_t	sample_cnt;	/* sample count for active interval 当前采样计数 */
	uint32_t	cached_rate;	/* the most recently measured rate 最近的采样值 */
	int32_t		rate_limit;	/* user-specified rate limit 每秒最低优先级可执行的op数限制 */
	pthread_mutex_t	lock;
};

struct iot_conf {
        pthread_mutex_t      mutex;
        pthread_cond_t       cond;

        int32_t              max_count;   /* configured maximum 最大并发线程数 */
        int32_t              curr_count;  /* actual number of threads running 当前运行中的线程数 */
        int32_t              sleep_count; /* 线程sleep的次数 */

        int32_t              idle_time;   /* in seconds 线程允许的最长空闲时间 */

        struct list_head     reqs[IOT_PRI_MAX];	/* 各优先级op队列 */

        int32_t              ac_iot_limit[IOT_PRI_MAX];	/* 各队列最大并发线程数 */
        int32_t              ac_iot_count[IOT_PRI_MAX]; /* 各队列当前并发线程数 */
        int                  queue_sizes[IOT_PRI_MAX];	/* 各队列缓存的请求数 */
        int                  queue_size;				/* 全局缓存的请求数 */
        pthread_attr_t       w_attr;
        gf_boolean_t         least_priority; /*Enable/Disable least-priority 启用最低优先级控制 */

        xlator_t            *this;
        size_t              stack_size;		/* 线程栈限制 */

	struct iot_least_throttle throttle;		/* 最低优先级的控制参数 */
};

typedef struct iot_conf iot_conf_t;

#endif /* __IOT_H */
