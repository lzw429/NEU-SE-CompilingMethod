//
// Created by Yiheng SHU on 12/23/18.
//

#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>


/**
 * 泛型栈
 */
typedef struct {
    size_t elem_size;// 记录所存储的类型的内存大小
    int pos; // 目前栈顶指针所处的位置
    size_t alloc_len; // 已分配空间
    void *elems; // 栈元素
    void (*freefn)(void *); // 释放空间的函数，用于动态分配
} stack;

/**
 * 创建栈
 * @param s 栈指针
 * @param elem_size 每个元素的大小
 */
void stack_new(stack *s, size_t elem_size);

/**
 * 销毁栈
 * @param s 栈指针
 */
void stack_dispose(stack *s);

/**
 * 元素进栈
 * @param s 栈指针
 * @param value 进栈元素
 */
void stack_push(stack *s, void *value);

/**
 * 获取栈顶并弹栈
 * @param s 栈指针
 * @param elem_addr 被函数赋为栈顶地址
 */
void stack_poll(stack *s, void *elem_addr);

/**
 * 获取栈顶
 * @param s 栈指针
 * @param elem_addr 被函数赋为栈顶地址
 */
void stack_peek(stack *s, void *elem_addr);

#endif // STACK_H
