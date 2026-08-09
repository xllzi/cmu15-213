#include "cache.h"
#include "csapp.h"
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

cache_t* cache;

void cache_init() {
    cache = malloc(sizeof(cache_t));
    cache->head = NULL;
    cache->tail = NULL;
    cache->total_size = 0;
    Sem_init(&(cache->mutex), 0, 1);
    Sem_init(&(cache->w), 0, 1);
    cache->reader_cnt = 0;
}

void cache_free() {
    cache_entry* cur = cache->head;
    while (cur) {
        cache_entry* nxt = cur->next;
        free(cur->data);
        free(cur);
        cur = nxt;
    }
    sem_destroy(&(cache->mutex));
    sem_destroy(&(cache->w));
    free(cache);
}


int cache_lookup(const char *key, char **data, size_t *size) {
    P(&(cache->mutex));
    cache->reader_cnt++;
    if (cache->reader_cnt == 1) {
        P(&(cache->w));
    }
    V(&(cache->mutex));
    cache_entry* cur = cache->head;
    while (cur != NULL) {
        if (strcmp(key, cur->key) == 0) {
            *data = malloc(cur->size);
            memcpy(*data, cur->data, cur->size);
            *size = cur->size;
            P(&(cache->mutex));
            cache->reader_cnt--;
            if (cache->reader_cnt == 0) {
                V(&(cache->w));
            }
            V(&(cache->mutex));
            // move current node to the head
            if (cur != cache->head) {
                P(&(cache->w));
                cache_entry* found = cache->head;
                while (found != NULL) {
                    if (strcmp(key, found->key) == 0)
                        break;
                    found = found->next;
                }
                if (found && found != cache->head) {
                    // 从原位摘除
                    found->prev->next = found->next;
                    if (found == cache->tail)
                        cache->tail = found->prev;
                    else
                        found->next->prev = found->prev;
                    // 插到头部
                    found->next = cache->head;
                    found->prev = NULL;
                    cache->head->prev = found;
                    cache->head = found;
                }
                V(&(cache->w));
            }
            return 0;
        } else {
            cur = cur->next;
        }
    }
    P(&(cache->mutex));
    cache->reader_cnt--;
    if (cache->reader_cnt == 0) {
        V(&(cache->w));
    }
    V(&(cache->mutex));
    return 1;
}

void cache_insert(const char *key, const char *data, size_t size) {
    if (size <= MAX_OBJECT_SIZE) {
        P(&(cache->w));
        cache_entry *insert_entry = malloc(sizeof(cache_entry));
        insert_entry->data = malloc(size);
        strcpy(insert_entry->key, key);
        memcpy(insert_entry->data, data, size);
        insert_entry->size = size;
        insert_entry->next = cache->head;
        insert_entry->prev = NULL;
        if (cache->head) {
            cache->head->prev = insert_entry;
        } else {
            cache->tail = insert_entry;
        }
        cache->head = insert_entry;
        cache->total_size += size;
        while (cache->total_size > MAX_CACHE_SIZE) {
            cache_entry* old_tail = cache->tail;
            cache->tail = cache->tail->prev;
            cache->total_size -= old_tail->size;
            free(old_tail->data);
            free(old_tail);
        }
        if (cache->tail) {
            cache->tail->next = NULL;
        } else {
            cache->head = NULL;
        }
        V(&(cache->w));
    }
}
