//
// Created by syh on 12/23/18.
//

#include "stack.h"

void stack_new(stack *s, size_t elem_size) {
    assert(elem_size > 0);
    s->elems = malloc(10 * elem_size);
    assert(s->elems != NULL);
    s->elem_size = elem_size;
    s->pos = 0;
    s->alloc_len = 10;
}

void stack_dispose(stack *s) {
    free(s->elems);
    s->alloc_len = 0;
    s->pos = 0;
}

static void stack_grow(stack *s) { // 空间不足时重新分配更大的空间
    assert(s != NULL);
    s->alloc_len *= 2;
    s->elems = realloc(s->elems, s->alloc_len);
}

void stack_push(stack *s, void *value) {
    if (s->pos >= s->alloc_len)
        stack_grow(s);
    void *top = s->elems + s->pos * s->elem_size; // 计算栈顶指针的位置
    memcpy(top, value, s->elem_size);
    s->pos++;
}

void stack_poll(stack *s, void *elem_addr) {
    assert(elem_addr != NULL && s != NULL);
    s->pos--;
    void *top = (char *) s->elems + s->pos * s->elem_size; // 栈顶元素地址
    memcpy(elem_addr, top, s->elem_size);
}

void stack_peek(stack *s, void *elem_addr) {
    assert(elem_addr != NULL && s != NULL);
    void *top = (char *) s->elems + (s->pos - 1) * s->elem_size; // 栈顶元素地址
    memcpy(elem_addr, top, s->elem_size);
}
