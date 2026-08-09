#ifndef CACHE_H
#define CACHE_H

#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
typedef struct cache_entry{
    char key[MAXLINE]; // the url:port
    char* data; // the response
    size_t size;
    struct cache_entry *next, *prev;
} cache_entry;

typedef struct {
    cache_entry* head; // the most recently accessed entry
    cache_entry* tail; // the earliest accessed entry
    size_t total_size;
    sem_t mutex, w;
    int reader_cnt;
} cache_t;

/* 初始化缓存 */
void cache_init();

/*
 * 查找缓存。key = "host:port/uri"
 * 命中：返回 0，*data 指向新分配的拷贝（调用者负责 Free），*size 是数据大小
 * 未命中：返回 1
 */
int cache_lookup(const char *key, char **data, size_t *size);

/*
 * 插入缓存。对象超过 MAX_OBJECT_SIZE 则不缓存。
 * data 由调用者所有，本函数内部做拷贝。
 */
void cache_insert(const char *key, const char *data, size_t size);

/* 释放所有缓存资源 */
void cache_free();

#endif
