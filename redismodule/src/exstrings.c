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

#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../redismodule/include/redismodule.h"

#ifdef __UT__
#include "exstringsStub.h"
#endif

/* make sure the response is not NULL or an error.
sends the error to the client and exit the current function if its */
#define  ASSERT_NOERROR(r) \
    if (r == NULL) { \
        return RedisModule_ReplyWithError(ctx,"ERR reply is NULL"); \
    } else if (RedisModule_CallReplyType(r) == REDISMODULE_REPLY_ERROR) { \
        RedisModule_ReplyWithCallReply(ctx,r); \
        RedisModule_FreeCallReply(r); \
        return REDISMODULE_ERR; \
    }

#define OBJ_OP_NO 0
#define OBJ_OP_XX (1<<1)     /* OP if key exist */
#define OBJ_OP_NX (1<<2)     /* OP if key not exist */
#define OBJ_OP_IE (1<<4)     /* OP if equal old value */
#define OBJ_OP_NE (1<<5)     /* OP if not equal old value */


int setStringGenericCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                                       int argc, const int flag)
{
    RedisModuleString *oldvalstr = NULL;
    RedisModuleCallReply *reply = NULL;

    if (argc < 4)
        return RedisModule_WrongArity(ctx);
    else
        oldvalstr = argv[3];

    /*Check if key type is string*/
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
        REDISMODULE_READ);
    int type = RedisModule_KeyType(key);
    RedisModule_CloseKey(key);

    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        if (flag == OBJ_OP_IE){
            RedisModule_ReplyWithNull(ctx);
            return REDISMODULE_OK;
        }
    } else if (type != REDISMODULE_KEYTYPE_STRING) {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    /*Get the value*/
    reply = RedisModule_Call(ctx, "GET", "s", argv[1]);
    ASSERT_NOERROR(reply)
    size_t curlen=0, oldvallen=0;
    const char *oldval = RedisModule_StringPtrLen(oldvalstr, &oldvallen);
    const char *curval = RedisModule_CallReplyStringPtr(reply, &curlen);
    if (((flag == OBJ_OP_IE) &&
        (!curval || (oldvallen != curlen) || strncmp(oldval, curval, curlen)))
        ||
        ((flag == OBJ_OP_NE) && curval && (oldvallen == curlen) &&
          !strncmp(oldval, curval, curlen))) {
        RedisModule_FreeCallReply(reply);
        return RedisModule_ReplyWithNull(ctx);
    }
    RedisModule_FreeCallReply(reply);

    /* Prepare the arguments for the command. */
    int i, j=0, cmdargc=argc-2;
    RedisModuleString *cmdargv[cmdargc];
    for (i = 1; i < argc; i++) {
        if (i == 3)
            continue;
        cmdargv[j++] = argv[i];
    }

    /* Call the command and pass back the reply. */
    reply = RedisModule_Call(ctx, "SET", "v!", cmdargv, cmdargc);
    ASSERT_NOERROR(reply)
    RedisModule_ReplyWithCallReply(ctx, reply);

    RedisModule_FreeCallReply(reply);
    return REDISMODULE_OK;
}

int SetIE_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setStringGenericCommand(ctx, argv, argc, OBJ_OP_IE);
}

int SetNE_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setStringGenericCommand(ctx, argv, argc, OBJ_OP_NE);
}

int delStringGenericCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                                       int argc, const int flag)
{
    RedisModuleString *oldvalstr = NULL;
    RedisModuleCallReply *reply = NULL;

    if (argc == 3)
        oldvalstr = argv[2];
    else
        return RedisModule_WrongArity(ctx);

    /*Check if key type is string*/
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
        REDISMODULE_READ);
    int type = RedisModule_KeyType(key);
    RedisModule_CloseKey(key);

    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithLongLong(ctx, 0);
    } else if (type != REDISMODULE_KEYTYPE_STRING) {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    /*Get the value*/
    reply = RedisModule_Call(ctx, "GET", "s", argv[1]);
    ASSERT_NOERROR(reply)
    size_t curlen = 0, oldvallen = 0;
    const char *oldval = RedisModule_StringPtrLen(oldvalstr, &oldvallen);
    const char *curval = RedisModule_CallReplyStringPtr(reply, &curlen);
    if (((flag == OBJ_OP_IE) &&
        (!curval || (oldvallen != curlen) || strncmp(oldval, curval, curlen)))
        ||
        ((flag == OBJ_OP_NE) && curval && (oldvallen == curlen) &&
          !strncmp(oldval, curval, curlen))) {
        RedisModule_FreeCallReply(reply);
        return RedisModule_ReplyWithLongLong(ctx, 0);
    }
    RedisModule_FreeCallReply(reply);

    /* Prepare the arguments for the command. */
    int cmdargc=1;
    RedisModuleString *cmdargv[1];
    cmdargv[0] = argv[1];

    /* Call the command and pass back the reply. */
    reply = RedisModule_Call(ctx, "UNLINK", "v!", cmdargv, cmdargc);
    ASSERT_NOERROR(reply)
    RedisModule_ReplyWithCallReply(ctx, reply);

    RedisModule_FreeCallReply(reply);
    return REDISMODULE_OK;
}

int DelIE_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return delStringGenericCommand(ctx, argv, argc, OBJ_OP_IE);
}

int DelNE_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return delStringGenericCommand(ctx, argv, argc, OBJ_OP_NE);
}

int NGet_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModuleCallReply *reply = NULL;

    if (argc != 2)
        return RedisModule_WrongArity(ctx);

    /* Call the command to get keys with pattern. */
    reply = RedisModule_Call(ctx, "KEYS", "s", argv[1]);
    ASSERT_NOERROR(reply)

    /* Prepare the arguments for the command. */
    size_t items = RedisModule_CallReplyLength(reply);
    if (items == 0) {
        //RedisModule_ReplyWithArray(ctx, items);
        RedisModule_ReplyWithCallReply(ctx, reply);
        RedisModule_FreeCallReply(reply);
    }
    else {
        RedisModuleString *cmdargv[items];
        size_t i=0, j;
        for (j = 0; j < items; j++) {
           RedisModuleString *rms = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(reply, j));
           cmdargv[i++] = rms;

           /*Assume all keys via SDL is string type for sake of saving time*/
#if 0
           /*Check if key type is string*/
           RedisModuleKey *key = RedisModule_OpenKey(ctx, rms ,REDISMODULE_READ);

           if (key) {
               int type = RedisModule_KeyType(key);
               RedisModule_CloseKey(key);
               if (type == REDISMODULE_KEYTYPE_STRING) {
                   cmdargv[i++] = rms;
               }
           } else {
               RedisModule_CloseKey(key);
           }
#endif
        }
        RedisModule_FreeCallReply(reply);

        reply = RedisModule_Call(ctx, "MGET", "v", cmdargv, i);
        ASSERT_NOERROR(reply)
        items = RedisModule_CallReplyLength(reply);
        RedisModule_ReplyWithArray(ctx, i*2);
        for (j = 0; (j<items && j<i); j++) {
           RedisModule_ReplyWithString(ctx, cmdargv[j]);
           RedisModule_ReplyWithString(ctx, RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(reply, j)));
        }

        RedisModule_FreeCallReply(reply);
    }

    return REDISMODULE_OK;
}

int NDel_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModuleCallReply *reply = NULL;

    if (argc != 2)
        return RedisModule_WrongArity(ctx);

    /* Call the command to get keys with pattern. */
    reply = RedisModule_Call(ctx, "KEYS", "s", argv[1]);
    ASSERT_NOERROR(reply)

    /* Prepare the arguments for the command. */
    size_t items = RedisModule_CallReplyLength(reply);
    if (items == 0) {
        RedisModule_ReplyWithLongLong(ctx, 0);
        RedisModule_FreeCallReply(reply);
    }
    else {
        RedisModuleString *cmdargv[items];
        size_t i=0, j;
        for (j = 0; j < items; j++) {
           RedisModuleString *rms = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(reply, j));
           cmdargv[i++] = rms;

           /*Assume all keys via SDL is string type for sake of saving time*/
#if 0
           //Check if key type is string
           RedisModuleKey *key = RedisModule_OpenKey(ctx, rms ,REDISMODULE_READ);

           if (key) {
               int type = RedisModule_KeyType(key);
               RedisModule_CloseKey(key);
               if (type == REDISMODULE_KEYTYPE_STRING) {
                   cmdargv[i++] = rms;
               }
           } else {
               RedisModule_CloseKey(key);
           }
#endif
        }
        RedisModule_FreeCallReply(reply);

        reply = RedisModule_Call(ctx, "UNLINK", "v!", cmdargv, i);
        ASSERT_NOERROR(reply)
        RedisModule_ReplyWithCallReply(ctx, reply);
        RedisModule_FreeCallReply(reply);

    }

    return REDISMODULE_OK;
}

int setPubStringGenericCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                                       int argc, const int flag)
{
    RedisModuleString *oldvalstr = NULL, *channel = NULL, *message = NULL;
    RedisModuleCallReply *reply = NULL;

    if (flag == OBJ_OP_NO) {
        if (argc < 5 || (argc % 2) == 0)
            return RedisModule_WrongArity(ctx);
        else {
            channel = argv[argc-2];
            message = argv[argc-1];
        }
    } else if (flag == OBJ_OP_XX || flag == OBJ_OP_NX) {
        if (argc != 5)
            return RedisModule_WrongArity(ctx);
        else {
            channel = argv[3];
            message = argv[4];
        }
    } else {
        if (argc != 6)
            return RedisModule_WrongArity(ctx);
        else {
            oldvalstr = argv[3];
            channel = argv[4];
            message = argv[5];
        }
    }

    /*Check if key type is string*/
    RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
        REDISMODULE_READ);
    int type = RedisModule_KeyType(key);
    RedisModule_CloseKey(key);

    if (flag != OBJ_OP_NO) {
        if (type == REDISMODULE_KEYTYPE_EMPTY) {
            if (flag == OBJ_OP_IE || flag == OBJ_OP_XX){
                return RedisModule_ReplyWithNull(ctx);
            }
        } else if (flag == OBJ_OP_NX) {
            return RedisModule_ReplyWithNull(ctx);
        } else if (type != REDISMODULE_KEYTYPE_STRING) {
            return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
        }
    }

    if (flag == OBJ_OP_IE || flag == OBJ_OP_NE) {
        /*Get the value*/
        reply = RedisModule_Call(ctx, "GET", "s", argv[1]);
        ASSERT_NOERROR(reply)
        size_t curlen = 0, oldvallen = 0;
        const char *oldval = RedisModule_StringPtrLen(oldvalstr, &oldvallen);
        const char *curval = RedisModule_CallReplyStringPtr(reply, &curlen);
        if (((flag == OBJ_OP_IE) &&
            (!curval || (oldvallen != curlen) || strncmp(oldval, curval, curlen)))
            ||
            ((flag == OBJ_OP_NE) && curval && (oldvallen == curlen) &&
              !strncmp(oldval, curval, curlen))) {
            RedisModule_FreeCallReply(reply);
            return RedisModule_ReplyWithNull(ctx);
        }
        RedisModule_FreeCallReply(reply);
    }


    /* Prepare the arguments for the command. */
    int i, j=0, cmdargc=argc-3;
    RedisModuleString *cmdargv[cmdargc];
    for (i = 1; i < argc-2; i++) {
        if ((flag == OBJ_OP_IE || flag == OBJ_OP_NE) && (i == 3))
            continue;
        cmdargv[j++] = argv[i];
    }

    /* Call the command and pass back the reply. */
    reply = RedisModule_Call(ctx, "MSET", "v!", cmdargv, j);
    ASSERT_NOERROR(reply)
    int replytype = RedisModule_CallReplyType(reply);
    if (replytype == REDISMODULE_REPLY_NULL) {
        RedisModule_ReplyWithNull(ctx);
    }
    else {
        cmdargc = 2;
        cmdargv[0] = channel;
        cmdargv[1] = message;
        RedisModuleCallReply *pubreply = RedisModule_Call(ctx, "PUBLISH", "v", cmdargv, cmdargc);
        RedisModule_FreeCallReply(pubreply);
        RedisModule_ReplyWithCallReply(ctx, reply);
    }

    RedisModule_FreeCallReply(reply);
    return REDISMODULE_OK;
}

int SetPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setPubStringGenericCommand(ctx, argv, argc, OBJ_OP_NO);
}

int SetIEPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setPubStringGenericCommand(ctx, argv, argc, OBJ_OP_IE);
}

int SetNEPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setPubStringGenericCommand(ctx, argv, argc, OBJ_OP_NE);
}

int SetNXPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setPubStringGenericCommand(ctx, argv, argc, OBJ_OP_NX);
}

int SetXXPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    return setPubStringGenericCommand(ctx, argv, argc, OBJ_OP_XX);
}

int delPubStringGenericCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                                       int argc, const int flag)
{
    RedisModuleString *oldvalstr = NULL, *channel = NULL, *message = NULL;
    RedisModuleCallReply *reply = NULL;

    if (flag == OBJ_OP_NO) {
        if (argc < 4)
            return RedisModule_WrongArity(ctx);
        else {
            channel = argv[argc-2];
            message = argv[argc-1];
        }
    } else {
        if (argc != 5)
            return RedisModule_WrongArity(ctx);
        else {
            oldvalstr = argv[2];
            channel = argv[3];
            message = argv[4];
        }
    }

    if (flag != OBJ_OP_NO) {
        /*Check if key type is string*/
        RedisModuleKey *key = RedisModule_OpenKey(ctx,argv[1],
            REDISMODULE_READ);
        int type = RedisModule_KeyType(key);
        RedisModule_CloseKey(key);

        if (type == REDISMODULE_KEYTYPE_EMPTY) {
            return RedisModule_ReplyWithLongLong(ctx, 0);
        } else if (type != REDISMODULE_KEYTYPE_STRING) {
            return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
        }
    }

    if (flag == OBJ_OP_IE || flag == OBJ_OP_NE) {
        /*Get the value*/
        reply = RedisModule_Call(ctx, "GET", "s", argv[1]);
        ASSERT_NOERROR(reply)
        size_t curlen = 0, oldvallen = 0;
        const char *oldval = RedisModule_StringPtrLen(oldvalstr, &oldvallen);
        const char *curval = RedisModule_CallReplyStringPtr(reply, &curlen);
        if (((flag == OBJ_OP_IE) &&
            (!curval || (oldvallen != curlen) || strncmp(oldval, curval, curlen)))
            ||
            ((flag == OBJ_OP_NE) && curval && (oldvallen == curlen) &&
              !strncmp(oldval, curval, curlen))) {
            RedisModule_FreeCallReply(reply);
            return RedisModule_ReplyWithLongLong(ctx, 0);
        }
        RedisModule_FreeCallReply(reply);
    }


    /* Prepare the arguments for the command. */
    int i, j=0, cmdargc=argc-3;
    RedisModuleString *cmdargv[cmdargc];
    for (i = 1; i < argc-2; i++) {
        if ((flag == OBJ_OP_IE || flag == OBJ_OP_NE) && (i == 2))
            continue;
        cmdargv[j++] = argv[i];
    }

    /* Call the command and pass back the reply. */
    reply = RedisModule_Call(ctx, "UNLINK", "v!", cmdargv, j);
    ASSERT_NOERROR(reply)
    int replytype = RedisModule_CallReplyType(reply);
    if (replytype == REDISMODULE_REPLY_NULL) {
        RedisModule_ReplyWithNull(ctx);
    }
    else if (RedisModule_CallReplyInteger(reply) == 0) {
        RedisModule_ReplyWithCallReply(ctx, reply);
    } else {
        cmdargc = 2;
        cmdargv[0] = channel;
        cmdargv[1] = message;
        RedisModuleCallReply *pubreply = RedisModule_Call(ctx, "PUBLISH", "v", cmdargv, cmdargc);
        RedisModule_FreeCallReply(pubreply);
        RedisModule_ReplyWithCallReply(ctx, reply);
    }

    RedisModule_FreeCallReply(reply);
    return REDISMODULE_OK;
}

int DelPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
   return delPubStringGenericCommand(ctx, argv, argc, OBJ_OP_NO);
}

int DelIEPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
   return delPubStringGenericCommand(ctx, argv, argc, OBJ_OP_IE);
}

int DelNEPub_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
   return delPubStringGenericCommand(ctx, argv, argc, OBJ_OP_NE);
}

/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx,"exstrings",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setie",
        SetIE_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setne",
        SetNE_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"delie",
        DelIE_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"delne",
        DelNE_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"nget",
        NGet_RedisCommand,"readonly",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"ndel",
        NDel_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"msetpub",
        SetPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setiepub",
        SetIEPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setnepub",
        SetNEPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setxxpub",
        SetXXPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"setnxpub",
        SetNXPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"delpub",
        DelPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"deliepub",
        DelIEPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"delnepub",
        DelNEPub_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
