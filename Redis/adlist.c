/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * 创建一个数组, 数组可以通过调用 AlFreeList() 函数来销毁
 * AlFreeList(), but private value of every node need to be freed
 * 但是数组中的节点的值, 需要在调用 AlFreeList() 函数之前手动销毁
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. 
 * 创建失败, 则返回 NULL, 成功则返回创建成功的数组
 */
list *listCreate(void)
{
    struct list *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 * 清空数组
 * This function can't fail. 
 * 这个方法不会失败, 必然会成功
 */

void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    //标准的链表释放节点过程, 把节点指向第二个元素, 然后释放第一个. 指向第三个, 释放第二个...
    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        //这句话的意思是, 如果上一个节点没有被释放的话, 就手动释放一下?
        //TODO...可是这个free的实现在哪?
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * 在数组的头部添加一个新的节点, 节点中的值的指针 会被保存在数组中的值域
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * 如果返回错误, 则表示数组中没有变化, 例如 添加的节点跟原始节点相同
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. 
 * 修改成功之后会返回数组的指针
 */
list *listAddNodeHead(list *list, void *value)
{
    listNode *node;
    //创建一个节点, 赋值
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    //如果数组是空数组, 则头节点 = 尾节点 = 要添加的节点
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        //数组不空, 则直接把头结点的前驱指向要添加的节点, 节点的后继指向原来的头结点
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    //数组长度 +1
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * 在数组的尾部添加一个结点, 并且把值的指针赋值给数组
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * 如果返回错误, 则表示数组中没有变化, 例如 添加的节点跟原始节点相同
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. 
 * 修改成功之后会返回数组的指针
 */
list *listAddNodeTail(list *list, void *value)
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        //要添加的结点的前驱指向数组的尾部
        node->prev = list->tail;
        //要添加结点的后继为空
        node->next = NULL;
        //数组的尾部指向目标结点
        list->tail->next = node;
        //尾结点赋值
        list->tail = node;
    }
    //数组长度 +1
    list->len++;
    return list;
}

//插入数据, after: 非0即真, 传正数表示向后插入
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * 在指定的数组中删除指定的节点
 * It's up to the caller to free the private value of the node.
 * 节点的私有值, 通过调用者来释放?
 * This function can't fail.
 * 方法调用不会失败
 */
void listDelNode(list *list, listNode *node)
{
    //判断前驱有没有值,即是不是数组头部
    if (node->prev)
        node->prev->next = node->next;
    else
        //数组头部
        list->head = node->next;
    //判断有没有后继, 即是不是数组尾
    if (node->next)
        node->next->prev = node->prev;
    else
        //数组尾
        list->tail = node->prev;
    //如果当前正在释放? 还是说原来有释放过的值?
    if (list->free) list->free(node->value);
    zfree(node);
    //数组长度-1
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * 返回一个迭代元素, 内部会调用 listNext(),
 * call to listNext() will return the next element of the list.
 * 这个方法就是用来做数组循环的, 内部会不断的获取下一个元素
 * This function can't fail. 
 * 该方法调用, 不会失败
 */
listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    //顺序遍历, 还是逆序遍历
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
//将迭代器的指针指向表头
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}
//将迭代器的指针指向表尾
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * 返回迭代器中的下一个元素(即数组中的下一个元素)
 * It's valid to remove the currently returned element using
 * 调用 listDelNode() 方法来删除`当前`元素的操作是 可执行的
 * listDelNode(), but not to remove other elements.
 * 但是不能去移除其他的元素
 * The function returns a pointer to the next element of the list,
 * 这个方法将会返回当前数组中的下一个元素
 * or NULL if there are no more elements, so the classical usage patter
 * 如果没有元素(表尾?)的话, 就返回NULL, 标准的调用方法如下:
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
listNode *listNext(listIter *iter)
{
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list *listDup(list *orig)
{
    list *copy;
    listIter iter;
    listNode *node;

    if ((copy = listCreate()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    listRewind(orig, &iter);
    while((node = listNext(&iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                return NULL;
            }
        } else
            value = node->value;
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            return NULL;
        }
    }
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
listNode *listSearchKey(list *list, void *key)
{
    listIter iter;
    listNode *node;

    listRewind(list, &iter);
    while((node = listNext(&iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                return node;
            }
        } else {
            if (key == node->value) {
                return node;
            }
        }
    }
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
listNode *listIndex(list *list, long index) {
    listNode *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
void listRotate(list *list) {
    listNode *tail = list->tail;

    if (listLength(list) <= 1) return;

    /* Detach current tail */
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
