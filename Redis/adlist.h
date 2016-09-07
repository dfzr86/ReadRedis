/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

//每一个节点
typedef struct listNode {
    //前置节点
    struct listNode *prev;
    //后置节点
    struct listNode *next;
    //节点的值
    void *value;
} listNode;

//这个是迭代器, 应该是用来做数组的循环的吧.
typedef struct listIter {
    //下一个节点(前一个或后一个)
    listNode *next;
    //链表的指向? 顺序还是逆序?
    int direction;
} listIter;

//数组的数据结构
typedef struct list {
    //头节点
    listNode *head;
    //尾节点
    listNode *tail;
    //TODO... 复制某个节点的指针
    void *(*dup)(void *ptr);
    //释放某个节点的指针
    //返回值是一个 指针
    void (*free)(void *ptr);
    //匹配某个节点的指针
    int (*match)(void *ptr, void *key);
    //数组的长度
    unsigned long len;
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)
//数组的头元素
#define listFirst(l) ((l)->head)
//数组的尾部元素
#define listLast(l) ((l)->tail)
//第 n 个元素的前一个元素
#define listPrevNode(n) ((n)->prev)
//第 n 个元素的后一个元素
#define listNextNode(n) ((n)->next)
//第 n 个元素的值
#define listNodeValue(n) ((n)->value)
//
#define listSetDupMethod(l,m) ((l)->dup = (m))
//
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
//创建一个数组
list *listCreate(void);
//干掉一个数组
void listRelease(list *list);
//添加数组头部节点? 添加元素进第一个位置
list *listAddNodeHead(list *list, void *value);
//在数组的尾部添加元素
list *listAddNodeTail(list *list, void *value);
//插入元素吧?
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
//列举被删除的元素? 这是什么鬼...
void listDelNode(list *list, listNode *node);
//获取迭代器, 其实就是遍历数组吧
listIter *listGetIterator(list *list, int direction);
//遍历数组, 取下一个元素
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
//通过角标来搜索元素节点
listNode *listIndex(list *list, long index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);
//数组翻转?
void listRotate(list *list);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
