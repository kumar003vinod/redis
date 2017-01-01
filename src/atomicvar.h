/* This file implements atomic counters using __atomic or __sync macros if
 * available, otherwise synchronizing different threads using a mutex.
 *
 * The exported interaface is composed of three macros:
 *
 * atomicIncr(var,count,mutex) -- Increment the atomic counter
 * atomicDecr(var,count,mutex) -- Decrement the atomic counter
 * atomicGet(var,dstvar,mutex) -- Fetch the atomic counter value
 *
 * If atomic primitives are availble (tested in config.h) the mutex
 * is not used.
 *
 * Never use return value from the macros. To update and get use instead:
 *
 *  atomicIncr(mycounter,...);
 *  atomicGet(mycounter,newvalue);
 *  doSomethingWith(newvalue);
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// I think this code is responsible for INCR and DECR commands in redis,
// which does atomic increment or decrement of variable
// I think the use of atomicIncr, atomicDecr, atomicGet depends on pthread
// implementation, which can be different for different operating systems

// why __atomic and __sync macros are prefered over mutex??
// what are __atomic and __sync
// http://stackoverflow.com/questions/13941385/using-gcc-atomic-builtins
// these are gcc atomic builtins, more on above link
//
// in case of __sync why redis uses __ATOMIC_RELAXED memory model, isn't is week?
// http://stackoverflow.com/questions/32377201/what-does-atomic-relaxed-mean
// http://en.cppreference.com/w/cpp/atomic/memory_order
// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/_005f_005fatomic-Builtins.html




#include <pthread.h>

#ifndef __ATOMIC_VAR_H
#define __ATOMIC_VAR_H

#if defined(__ATOMIC_RELAXED) && (!defined(__clang__) || !defined(__APPLE__) || __apple_build_version__ > 4210057)
/* Implementation using __atomic macros. */

#define atomicIncr(var,count,mutex) __atomic_add_fetch(&var,(count),__ATOMIC_RELAXED)
#define atomicDecr(var,count,mutex) __atomic_sub_fetch(&var,(count),__ATOMIC_RELAXED)
#define atomicGet(var,dstvar,mutex) do { \
    dstvar = __atomic_load_n(&var,__ATOMIC_RELAXED); \
} while(0)

#elif defined(HAVE_ATOMIC)
/* Implementation using __sync macros. */

#define atomicIncr(var,count,mutex) __sync_add_and_fetch(&var,(count))
#define atomicDecr(var,count,mutex) __sync_sub_and_fetch(&var,(count))
#define atomicGet(var,dstvar,mutex) do { \
    dstvar = __sync_sub_and_fetch(&var,0); \
} while(0)

#else
/* Implementation using pthread mutex. */

#define atomicIncr(var,count,mutex) do { \
    pthread_mutex_lock(&mutex); \
    var += (count); \
    pthread_mutex_unlock(&mutex); \
} while(0)

#define atomicDecr(var,count,mutex) do { \
    pthread_mutex_lock(&mutex); \
    var -= (count); \
    pthread_mutex_unlock(&mutex); \
} while(0)

#define atomicGet(var,dstvar,mutex) do { \
    pthread_mutex_lock(&mutex); \
    dstvar = var; \
    pthread_mutex_unlock(&mutex); \
} while(0)
#endif

#endif /* __ATOMIC_VAR_H */
