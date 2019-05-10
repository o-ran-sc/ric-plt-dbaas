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


#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <unistd.h>
#include <string.h>




extern "C" {
#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
}


int RedisModule_CreateCommand(RedisModuleCtx *ctx, const char *name, RedisModuleCmdFunc cmdfunc, const char *strflags, int firstkey, int lastkey, int keystep)
{
    (void)ctx;
    (void)name;
    (void)cmdfunc;
    (void)strflags;
    (void)firstkey;
    (void)lastkey;
    (void)keystep;
    return REDISMODULE_OK;

}

int RedisModule_WrongArity(RedisModuleCtx *ctx)
{
    (void)ctx;
    return REDISMODULE_ERR;
}

int RedisModule_ReplyWithLongLong(RedisModuleCtx *ctx, long long ll)
{

    (void)ctx;
    mock().setData("RedisModule_ReplyWithLongLong", (int)ll);
    return REDISMODULE_OK;
}

void *RedisModule_OpenKey(RedisModuleCtx *ctx, RedisModuleString *keyname, int mode)
{
    (void)ctx;
    (void)keyname;
    (void)mode;

    if (mock().hasData("RedisModule_OpenKey_no"))
    {
        return (void*)(0);
    }

    if (mock().hasData("RedisModule_OpenKey_have"))
    {
        return (void*)(111111);
    }


    return (void*)(0);
}

RedisModuleCallReply *RedisModule_Call(RedisModuleCtx *ctx, const char *cmdname, const char *fmt, ...)
{
    (void)ctx;
    (void)cmdname;
    (void)fmt;

    if (!strcmp(cmdname, "GET"))
        mock().setData("GET", 1);
    else if (!strcmp(cmdname, "SET"))
        mock().setData("SET", 1);
    else if (!strcmp(cmdname, "MSET"))
        mock().setData("MSET", 1);
    else if (!strcmp(cmdname, "DEL"))
        mock().setData("DEL", 1);
    else if (!strcmp(cmdname, "UNLINK"))
        mock().setData("UNLINK", 1);
    else if (!strcmp(cmdname, "PUBLISH"))
        mock().setData("PUBLISH", 1);
    else if (!strcmp(cmdname, "KEYS"))
        mock().setData("KEYS", 1);
    else if (!strcmp(cmdname, "MGET"))
        mock().setData("MGET", 1);

    if (mock().hasData("RedisModule_Call_Return_Null"))
        return NULL;
    else
        return (RedisModuleCallReply *)1;
}

void RedisModule_FreeCallReply(RedisModuleCallReply *reply)
{
    (void)reply;
    mock().setData("RedisModule_FreeCallReply", mock().getData("RedisModule_FreeCallReply").getIntValue()+1);
}

int RedisModule_CallReplyType(RedisModuleCallReply *reply)
{

    (void)reply;
    if (mock().hasData("RedisModule_CallReplyType_null"))
    {
        return REDISMODULE_REPLY_NULL;
    }

    if (mock().hasData("RedisModule_CallReplyType_inter"))
    {
        return REDISMODULE_REPLY_INTEGER;
    }

    if (mock().hasData("RedisModule_CallReplyType_str"))
    {
        return REDISMODULE_REPLY_STRING;
    }

    if (mock().hasData("RedisModule_CallReplyType_err"))
    {
        return REDISMODULE_REPLY_ERROR;
    }

    return REDISMODULE_REPLY_NULL;;

}

long long RedisModule_CallReplyInteger(RedisModuleCallReply *reply)
{

    (void)reply;
    return mock().getData("RedisModule_CallReplyInteger").getIntValue();
}

const char *RedisModule_StringPtrLen(const RedisModuleString *str, size_t *len)
{

    (void)str;
    *len = 5;
    if (mock().hasData("RedisModule_String_same"))
    {
        return "11111";
    }

    if (mock().hasData("RedisModule_String_nosame"))
    {
        return "22222";
    }

    return "11111";
}

int RedisModule_ReplyWithError(RedisModuleCtx *ctx, const char *err)
{
    (void)ctx;
    (void)err;
    mock().setData("RedisModule_ReplyWithError", 1);
    return REDISMODULE_OK;
}

int RedisModule_ReplyWithString(RedisModuleCtx *ctx, RedisModuleString *str)
{
    (void)ctx;
    (void)str;
    mock().setData("RedisModule_ReplyWithString", mock().getData("RedisModule_ReplyWithString").getIntValue()+1);
    return REDISMODULE_OK;
}

int RedisModule_ReplyWithNull(RedisModuleCtx *ctx)
{

    (void)ctx;
    mock().setData("RedisModule_ReplyWithNull", 1);
    return REDISMODULE_OK;
}

int RedisModule_ReplyWithCallReply(RedisModuleCtx *ctx, RedisModuleCallReply *reply)
{
    (void)ctx;
    (void)reply;
    mock().setData("RedisModule_ReplyWithCallReply", 1);
    return REDISMODULE_OK;
}

const char *RedisModule_CallReplyStringPtr(RedisModuleCallReply *reply, size_t *len)
{
    (void)reply;

    *len = 5;

    if (mock().hasData("RedisModule_String_same"))
    {
        return "11111";
    }


    if (mock().hasData("RedisModule_String_nosame"))
    {
        *len = 6;
        return "333333";
    }

    return "11111";
}

RedisModuleString *RedisModule_CreateStringFromCallReply(RedisModuleCallReply *reply)
{
    (void)reply;
    return (RedisModuleString *)1;
}


int RedisModule_KeyType(RedisModuleKey *kp)
{


    (void)kp;
    if (mock().hasData("RedisModule_KeyType_empty"))
    {
        return REDISMODULE_KEYTYPE_EMPTY;
    }

    if (mock().hasData("RedisModule_KeyType_str"))
    {
        return REDISMODULE_KEYTYPE_STRING;
    }

    if (mock().hasData("RedisModule_KeyType_set"))
    {

        return REDISMODULE_KEYTYPE_SET;
    }

    return REDISMODULE_KEYTYPE_EMPTY;


}

void RedisModule_CloseKey(RedisModuleKey *kp)
{
    (void)kp;
    mock().actualCall("RedisModule_CloseKey");
}

/* This is included inline inside each Redis module. */
int RedisModule_Init(RedisModuleCtx *ctx, const char *name, int ver, int apiver)
{
    (void)ctx;
    (void)name;
    (void)ver;
    (void)apiver;
    return REDISMODULE_OK;
}

size_t RedisModule_CallReplyLength(RedisModuleCallReply *reply)
{
    (void)reply;
    return mock().getData("RedisModule_CallReplyLength").getIntValue();
}


RedisModuleCallReply *RedisModule_CallReplyArrayElement(RedisModuleCallReply *reply, size_t idx)
{
    (void)reply;
    (void)idx;
    return (RedisModuleCallReply *)1;
}

int RedisModule_ReplyWithArray(RedisModuleCtx *ctx, long len)
{
    (void)ctx;
    mock().setData("RedisModule_ReplyWithArray", (int)len);
    return REDISMODULE_OK;
}

