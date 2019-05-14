/*
 * Copyright (c) 2018-2019 Nokia.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef REDISMODULE_H
#define REDISMODULE_H

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

/* ---------------- Defines common between core and modules --------------- */




/* Error status return values. */
#define REDISMODULE_OK 0
#define REDISMODULE_ERR 1

/* API versions. */
#define REDISMODULE_APIVER_1 1

/* API flags and constants */
#define REDISMODULE_READ (1<<0)
#define REDISMODULE_WRITE (1<<1)

#define REDISMODULE_LIST_HEAD 0
#define REDISMODULE_LIST_TAIL 1

/* Key types. */
#define REDISMODULE_KEYTYPE_EMPTY 0
#define REDISMODULE_KEYTYPE_STRING 1
#define REDISMODULE_KEYTYPE_LIST 2
#define REDISMODULE_KEYTYPE_HASH 3
#define REDISMODULE_KEYTYPE_SET 4
#define REDISMODULE_KEYTYPE_ZSET 5
#define REDISMODULE_KEYTYPE_MODULE 6

/* Reply types. */
#define REDISMODULE_REPLY_UNKNOWN -1
#define REDISMODULE_REPLY_STRING 0
#define REDISMODULE_REPLY_ERROR 1
#define REDISMODULE_REPLY_INTEGER 2
#define REDISMODULE_REPLY_ARRAY 3
#define REDISMODULE_REPLY_NULL 4

/* Postponed array length. */
#define REDISMODULE_POSTPONED_ARRAY_LEN -1

/* Expire */
#define REDISMODULE_NO_EXPIRE -1

/* Sorted set API flags. */
#define REDISMODULE_ZADD_XX      (1<<0)
#define REDISMODULE_ZADD_NX      (1<<1)
#define REDISMODULE_ZADD_ADDED   (1<<2)
#define REDISMODULE_ZADD_UPDATED (1<<3)
#define REDISMODULE_ZADD_NOP     (1<<4)

/* Hash API flags. */
#define REDISMODULE_HASH_NONE       0
#define REDISMODULE_HASH_NX         (1<<0)
#define REDISMODULE_HASH_XX         (1<<1)
#define REDISMODULE_HASH_CFIELDS    (1<<2)
#define REDISMODULE_HASH_EXISTS     (1<<3)

/* Context Flags: Info about the current context returned by RM_GetContextFlags */

/* The command is running in the context of a Lua script */
#define REDISMODULE_CTX_FLAGS_LUA 0x0001
/* The command is running inside a Redis transaction */
#define REDISMODULE_CTX_FLAGS_MULTI 0x0002
/* The instance is a master */
#define REDISMODULE_CTX_FLAGS_MASTER 0x0004
/* The instance is a slave */
#define REDISMODULE_CTX_FLAGS_SLAVE 0x0008
/* The instance is read-only (usually meaning it's a slave as well) */
#define REDISMODULE_CTX_FLAGS_READONLY 0x0010
/* The instance is running in cluster mode */
#define REDISMODULE_CTX_FLAGS_CLUSTER 0x0020
/* The instance has AOF enabled */
#define REDISMODULE_CTX_FLAGS_AOF 0x0040 //
/* The instance has RDB enabled */
#define REDISMODULE_CTX_FLAGS_RDB 0x0080 //
/* The instance has Maxmemory set */
#define REDISMODULE_CTX_FLAGS_MAXMEMORY 0x0100
/* Maxmemory is set and has an eviction policy that may delete keys */
#define REDISMODULE_CTX_FLAGS_EVICT 0x0200


/* A special pointer that we can use between the core and the module to signal
 * field deletion, and that is impossible to be a valid pointer. */
#define REDISMODULE_HASH_DELETE ((RedisModuleString*)(long)1)

/* Error messages. */
#define REDISMODULE_ERRORMSG_WRONGTYPE "WRONGTYPE Operation against a key holding the wrong kind of value"

#define REDISMODULE_POSITIVE_INFINITE (1.0/0.0)
#define REDISMODULE_NEGATIVE_INFINITE (-1.0/0.0)

#define REDISMODULE_NOT_USED(V) ((void) V)

/* ------------------------- End of common defines ------------------------ */




#ifndef REDISMODULE_CORE



typedef long long mstime_t;

/* Incomplete structures for compiler checks but opaque access. */
typedef struct RedisModuleCtx RedisModuleCtx;
typedef struct RedisModuleKey RedisModuleKey;
typedef struct RedisModuleString RedisModuleString;
typedef struct RedisModuleCallReply RedisModuleCallReply;
typedef struct RedisModuleIO RedisModuleIO;
typedef struct RedisModuleType RedisModuleType;
typedef struct RedisModuleDigest RedisModuleDigest;
typedef struct RedisModuleBlockedClient RedisModuleBlockedClient;

//typedef int (*RedisModuleCmdFunc) (RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

typedef void *(*RedisModuleTypeLoadFunc)(RedisModuleIO *rdb, int encver);
typedef void (*RedisModuleTypeSaveFunc)(RedisModuleIO *rdb, void *value);
typedef void (*RedisModuleTypeRewriteFunc)(RedisModuleIO *aof, RedisModuleString *key, void *value);
typedef size_t (*RedisModuleTypeMemUsageFunc)(const void *value);
typedef void (*RedisModuleTypeDigestFunc)(RedisModuleDigest *digest, void *value);
typedef void (*RedisModuleTypeFreeFunc)(void *value);

#define REDISMODULE_TYPE_METHOD_VERSION 1
typedef struct RedisModuleTypeMethods {
    uint64_t version;
    RedisModuleTypeLoadFunc rdb_load;
    RedisModuleTypeSaveFunc rdb_save;
    RedisModuleTypeRewriteFunc aof_rewrite;
    RedisModuleTypeMemUsageFunc mem_usage;
    RedisModuleTypeDigestFunc digest;
    RedisModuleTypeFreeFunc free;
} RedisModuleTypeMethods;

#define REDISMODULE_GET_API(name) \
    RedisModule_GetApi("RedisModule_" #name, ((void **)&RedisModule_ ## name))

#define REDISMODULE_API_FUNC(x) (*x)

#if 1


typedef struct redisObject {
    unsigned type:4;
    unsigned encoding:4;
    int refcount;
    void *ptr;
} robj;
/* This structure represents a module inside the system. */
struct RedisModule {
    void *handle;   /* Module dlopen() handle. */
    char *name;     /* Module name. */
    int ver;        /* Module version. We use just progressive integers. */
    int apiver;     /* Module API version as requested during initialization.*/
    //list *types;    /* Module data types. */
};
typedef struct RedisModule RedisModule;

//static dict *modules; /* Hash table of modules. SDS -> RedisModule ptr.*/

/* Entries in the context->amqueue array, representing objects to free
 * when the callback returns. */
struct AutoMemEntry {
    void *ptr;
    int type;
};

/* AutMemEntry type field values. */
#define REDISMODULE_AM_KEY 0
#define REDISMODULE_AM_STRING 1
#define REDISMODULE_AM_REPLY 2
#define REDISMODULE_AM_FREED 3 /* Explicitly freed by user already. */

/* The pool allocator block. Redis Modules can allocate memory via this special
 * allocator that will automatically release it all once the callback returns.
 * This means that it can only be used for ephemeral allocations. However
 * there are two advantages for modules to use this API:
 *
 * 1) The memory is automatically released when the callback returns.
 * 2) This allocator is faster for many small allocations since whole blocks
 *    are allocated, and small pieces returned to the caller just advancing
 *    the index of the allocation.
 *
 * Allocations are always rounded to the size of the void pointer in order
 * to always return aligned memory chunks. */

#define REDISMODULE_POOL_ALLOC_MIN_SIZE (1024*8)
#define REDISMODULE_POOL_ALLOC_ALIGN (sizeof(void*))

typedef struct RedisModulePoolAllocBlock {
    uint32_t size;
    uint32_t used;
    struct RedisModulePoolAllocBlock *next;
    char memory[];
} RedisModulePoolAllocBlock;

/* This structure represents the context in which Redis modules operate.
 * Most APIs module can access, get a pointer to the context, so that the API
 * implementation can hold state across calls, or remember what to free after
 * the call and so forth.
 *
 * Note that not all the context structure is always filled with actual values
 * but only the fields needed in a given context. */

struct RedisModuleBlockedClient;

struct RedisModuleCtx {
    void *getapifuncptr;            /* NOTE: Must be the first field. */
    struct RedisModule *module;     /* Module reference. */
    //client *client;                 /* Client calling a command. */
    struct RedisModuleBlockedClient *blocked_client; /* Blocked client for
                                                        thread safe context. */
    struct AutoMemEntry *amqueue;   /* Auto memory queue of objects to free. */
    int amqueue_len;                /* Number of slots in amqueue. */
    int amqueue_used;               /* Number of used slots in amqueue. */
    int flags;                      /* REDISMODULE_CTX_... flags. */
    void **postponed_arrays;        /* To set with RM_ReplySetArrayLength(). */
    int postponed_arrays_count;     /* Number of entries in postponed_arrays. */
    void *blocked_privdata;         /* Privdata set when unblocking a client. */

    /* Used if there is the REDISMODULE_CTX_KEYS_POS_REQUEST flag set. */
    int *keys_pos;
    int keys_count;

    struct RedisModulePoolAllocBlock *pa_head;
};
typedef struct RedisModuleCtx RedisModuleCtx;

#define REDISMODULE_CTX_INIT {(void*)(unsigned long)&RM_GetApi, NULL, NULL, NULL, NULL, 0, 0, 0, NULL, 0, NULL, NULL, 0, NULL}
#define REDISMODULE_CTX_MULTI_EMITTED (1<<0)
#define REDISMODULE_CTX_AUTO_MEMORY (1<<1)
#define REDISMODULE_CTX_KEYS_POS_REQUEST (1<<2)
#define REDISMODULE_CTX_BLOCKED_REPLY (1<<3)
#define REDISMODULE_CTX_BLOCKED_TIMEOUT (1<<4)
#define REDISMODULE_CTX_THREAD_SAFE (1<<5)

/* This represents a Redis key opened with RM_OpenKey(). */
struct RedisModuleKey {
    RedisModuleCtx *ctx;
    //redisDb *db;
    robj *key;      /* Key name object. */
    robj *value;    /* Value object, or NULL if the key was not found. */
    void *iter;     /* Iterator. */
    int mode;       /* Opening mode. */

    /* Zset iterator. */
    uint32_t ztype;         /* REDISMODULE_ZSET_RANGE_* */
    //zrangespec zrs;         /* Score range. */
    //zlexrangespec zlrs;     /* Lex range. */
    uint32_t zstart;        /* Start pos for positional ranges. */
    uint32_t zend;          /* End pos for positional ranges. */
    void *zcurrent;         /* Zset iterator current node. */
    int zer;                /* Zset iterator end reached flag
                               (true if end was reached). */
};
typedef struct RedisModuleKey RedisModuleKey;

/* RedisModuleKey 'ztype' values. */
#define REDISMODULE_ZSET_RANGE_NONE 0       /* This must always be 0. */
#define REDISMODULE_ZSET_RANGE_LEX 1
#define REDISMODULE_ZSET_RANGE_SCORE 2
#define REDISMODULE_ZSET_RANGE_POS 3

/* Function pointer type of a function representing a command inside
 * a Redis module. */

typedef int (*RedisModuleCmdFunc) (RedisModuleCtx *ctx, RedisModuleString **argv, int argc);


/* This struct holds the information about a command registered by a module.*/
struct RedisModuleCommandProxy {
    struct RedisModule *module;
    RedisModuleCmdFunc func;
    struct redisCommand *rediscmd;
};
typedef struct RedisModuleCommandProxy RedisModuleCommandProxy;

#define REDISMODULE_REPLYFLAG_NONE 0
#define REDISMODULE_REPLYFLAG_TOPARSE (1<<0) /* Protocol must be parsed. */
#define REDISMODULE_REPLYFLAG_NESTED (1<<1)  /* Nested reply object. No proto
                                                or struct free. */

/* Reply of RM_Call() function. The function is filled in a lazy
 * way depending on the function called on the reply structure. By default
 * only the type, proto and protolen are filled. */
typedef struct RedisModuleCallReply {
    RedisModuleCtx *ctx;
    int type;       /* REDISMODULE_REPLY_... */
    int flags;      /* REDISMODULE_REPLYFLAG_...  */
    size_t len;     /* Len of strings or num of elements of arrays. */
    char *proto;    /* Raw reply protocol. An SDS string at top-level object. */
    size_t protolen;/* Length of protocol. */
    union {
        const char *str; /* String pointer for string and error replies. This
                            does not need to be freed, always points inside
                            a reply->proto buffer of the reply object or, in
                            case of array elements, of parent reply objects. */
        long long ll;    /* Reply value for integer reply. */
        struct RedisModuleCallReply *array; /* Array of sub-reply elements. */
    } val;
} RedisModuleCallReply;

/* Structure representing a blocked client. We get a pointer to such
 * an object when blocking from modules. */
typedef struct RedisModuleBlockedClient {
    //client *client;  /* Pointer to the blocked client. or NULL if the client
     //                   was destroyed during the life of this object. */
    RedisModule *module;    /* Module blocking the client. */
    RedisModuleCmdFunc reply_callback; /* Reply callback on normal completion.*/
    RedisModuleCmdFunc timeout_callback; /* Reply callback on timeout. */
    void (*free_privdata)(void *);       /* privdata cleanup callback. */
    void *privdata;     /* Module private data that may be used by the reply
                           or timeout callback. It is set via the
                           RedisModule_UnblockClient() API. */
    //client *reply_client;           /* Fake client used to accumulate replies
   //                                    in thread safe contexts. */
    int dbid;           /* Database number selected by the original client. */
} RedisModuleBlockedClient;

#define RedisModuleString robj

#endif




int RedisModule_CreateCommand(RedisModuleCtx *ctx, const char *name, RedisModuleCmdFunc cmdfunc, const char *strflags, int firstkey, int lastkey, int keystep);
int RedisModule_WrongArity(RedisModuleCtx *ctx);
int RedisModule_ReplyWithLongLong(RedisModuleCtx *ctx, long long ll);
void *RedisModule_OpenKey(RedisModuleCtx *ctx, RedisModuleString *keyname, int mode);
RedisModuleCallReply *RedisModule_Call(RedisModuleCtx *ctx, const char *cmdname, const char *fmt, ...);
void RedisModule_FreeCallReply(RedisModuleCallReply *reply);
int RedisModule_CallReplyType(RedisModuleCallReply *reply);
long long RedisModule_CallReplyInteger(RedisModuleCallReply *reply);
const char *RedisModule_StringPtrLen(const RedisModuleString *str, size_t *len);
int RedisModule_ReplyWithError(RedisModuleCtx *ctx, const char *err);
int RedisModule_ReplyWithString(RedisModuleCtx *ctx, RedisModuleString *str);
int RedisModule_ReplyWithNull(RedisModuleCtx *ctx);
int RedisModule_ReplyWithCallReply(RedisModuleCtx *ctx, RedisModuleCallReply *reply);
const char *RedisModule_CallReplyStringPtr(RedisModuleCallReply *reply, size_t *len);
RedisModuleString *RedisModule_CreateStringFromCallReply(RedisModuleCallReply *reply);
int RedisModule_StringToLongLong(const RedisModuleString *str, long long *ll);

int RedisModule_KeyType(RedisModuleKey *kp);
void RedisModule_CloseKey(RedisModuleKey *kp);

/* This is included inline inside each Redis module. */
int RedisModule_Init(RedisModuleCtx *ctx, const char *name, int ver, int apiver);

size_t RedisModule_CallReplyLength(RedisModuleCallReply *reply);
RedisModuleCallReply *RedisModule_CallReplyArrayElement(RedisModuleCallReply *reply, size_t idx);
int RedisModule_ReplyWithArray(RedisModuleCtx *ctx, long len);


/* Things only defined for the modules core, not exported to modules
 * including this file. */

#else

/* Things only defined for the modules core, not exported to modules
 * including this file. */
#define RedisModuleString robj

#endif /* REDISMODULE_CORE */
#endif /* REDISMOUDLE_H */
