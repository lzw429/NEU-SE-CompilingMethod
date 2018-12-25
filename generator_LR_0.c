//
// Created by syh on 12/23/18.
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include <ctype.h>

#define OK 20
struct quat {
    char op; // 运算符
    char op1[64]; // 操作数1
    char op2[64]; // 操作数2
    char res[64]; // 运算结果
};

struct item { // SYN 栈元素
    char word;
    int state;
};

struct var { // SEM 栈元素
    char str[64];
};

stack *SYN; // 算符栈
stack *SEM; // 语义栈
struct item *item;
struct var *var;
struct quat quats[20];
struct quat *q = quats;
int state = 0; // 状态
int col = 0; // 列
int action = -1; // 执行子程序：GEQ 0, PUSH 1

void print_quat(); // 打印生成的所有四元式
void print_stacks(char word); // 打印 SYN 和 SEM 栈
void print_action(); // 打印动作：GEQ 或 PUSH

void GEQ();

int SLR[][9] = {
        {8,  0,  0,  9,  0,  0,  1,  4, 7},
        {0,  2,  0,  0,  0, OK,  0,  0, 0},
        {8,  0,  0,  9,  0,  0,  0,  3, 7},
        {0,  -1, 5,  0,  -1, -1, 0,  0, 0},
        {0,  -2, 5,  0,  -2, -2, 0,  0, 0},
        {8,  0,  0,  9,  0,  0,  0,  0, 6},
        {-3, -3, -3, -3, -3, -3, 0,  0, 0},
        {-4, -4, -4, -4, -4, -4, 0,  0, 0},
        {-6, -6, -6, -6, -6, -6, 0,  0, 0},
        {8,  0,  0,  9,  0,  0,  10, 4, 7},
        {0,  2,  0,  0,  11, 0,  0,  0, 0},
        {-5, -5, -5, -5, -5, -5, 0,  0, 0}
};

int main(void) {
    // 初始化
    FILE *fp = fopen("expression.txt", "r"); // 读取文件
    if (fp == NULL) {
        printf("[Error] File cannot be open\n");
        return 0;
    }

    char *code = (char *) malloc(sizeof(char));
    char *code_start = code;
    item = (struct item *) malloc(sizeof(struct item));
    var = (struct var *) malloc(sizeof(struct var));

    fscanf(fp, "%s", code);
    int len = (int) strlen(code);
    code[len] = '#';
    code[len + 1] = '\0';

    SYN = (stack *) malloc(sizeof(stack));
    SEM = (stack *) malloc(sizeof(stack));
    stack_new(SYN, sizeof(struct item));
    stack_new(SEM, sizeof(struct var));
    state = 0;

    item->state = 0;
    item->word = '#';

    stack_push(SYN, item);
    printf("[Info] Start to generate...\n");

    printf("SYN                                               top  w       SEM\n"); // 表头
    while ((*code) != '\0') {
        print_stacks(*code);
        while ((*code) == ' ') // 跳过代码中的空格
            code++;
        // 找到对应的列
        if (isalpha(*code))
            col = 0;
        else if ((*code) == '+' || (*code) == '-')
            col = 1;
        else if ((*code) == '*' || (*code) == '/')
            col = 2;
        else if ((*code) == '(')
            col = 3;
        else if ((*code) == ')')
            col = 4;
        else if ((*code) == '#')
            col = 5;
        else {
            printf("[Error] Invalid character: %c\n", *code);
            break;
        }

        stack_peek(SYN, item);
        state = SLR[item->state][col];

        if (state < 0) { // 需要规约
            switch (state) {
                case -1:
                    // 规约：E -> E +/- T {GEQ(+/-)}
                    GEQ();
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);

                    stack_peek(SYN, item);
                    item->state = SLR[item->state][6];
                    item->word = 'E';
                    stack_push(SYN, item);
                    break;
                case -2:
                    // 规约：E -> T
                    stack_poll(SYN, item);

                    stack_peek(SYN, item);
                    item->state = SLR[item->state][6];
                    item->word = 'E';
                    stack_push(SYN, item);
                    break;
                case -3:
                    // 规约：T -> T *// F {GEQ(*//)}
                    GEQ();
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);

                    stack_peek(SYN, item);
                    item->state = SLR[item->state][7];
                    item->word = 'T';
                    stack_push(SYN, item);
                    break;
                case -4:
                    // 规约：T -> F
                    stack_poll(SYN, item);
                    stack_peek(SYN, item);
                    item->state = SLR[item->state][7];
                    item->word = 'T';
                    stack_push(SYN, item);
                    break;
                case -5:
                    // 规约：F -> (E)
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);
                    stack_poll(SYN, item);
                    stack_peek(SYN, item);
                    item->state = SLR[item->state][8];
                    item->word = 'F';
                    stack_push(SYN, item);
                    break;
                case -6:
                    // 规约：F -> i
                    stack_peek(SYN, item);
                    strcpy(var->str, &item->word);
                    stack_push(SEM, var);
                    action = 1;

                    stack_poll(SYN, item);
                    stack_peek(SYN, item);
                    item->state = SLR[item->state][8];
                    item->word = 'F';
                    stack_push(SYN, item);
                    break;
                default:
                    break;
            }
        } else if (state == 0) {
            printf("\n[Error] Invalid expression, unexpected character: %c\n", *code);
            break;
        } else if (state == OK) {
            printf("\n[Info] Generation completed\n");
            print_quat();
            break;
        } else {
            // 将当前单词放入 SYN 栈
            item->state = state;
            item->word = *code;
            stack_push(SYN, item);
            code++;
        }
        print_action();
    }

    // 退出程序前释放空间
    printf("[Info] Exit\n");
    free(code_start);
    stack_dispose(SYN);
    stack_dispose(SEM);
    free(item);
    return 0;
}

/**
 * 打印四元式
 */
void print_quat() {
    struct quat *p = quats;
    while (p < q) {
        printf("(%c, %s, %s, %s)\n", p->op, p->op1, p->op2, p->res);
        p++;
    }
}

/**
 * 生成四元式
 */
void GEQ() {
    char op; // 运算符
    /**
     * SYN 取运算符
     */
    for (int i = SYN->pos - 1; i >= 0; i--) {
        struct item *t = SYN->elems;
        op = (t + i)->word;
        if (op == '+' || op == '-' || op == '*' || op == '/') {
            q->op = op;
            break;
        }
    }
    /**
     * SEM 取操作数
     */
    stack_poll(SEM, var);
    strcpy(q->op2, var->str); // 栈顶是操作数2
    stack_poll(SEM, var);
    strcpy(q->op1, var->str); // 次栈顶是操作数1
    int num = (int) (q - quats + 1);
    sprintf(q->res, "t%d", num);
    /**
     * SEM 存运算结果
     */
    stack_push(SEM, q->res);
    q++;
    action = 0;
}


void print_stacks(char word) {
    struct item *p_item = (struct item *) SYN->elems;
    for (int i = 0; i < SYN->pos; i++) {
        printf("%c%d ", (p_item + i)->word, (p_item + i)->state);
    }
    for (int i = 0; i < 50 - 3 * (SYN->pos); i++)
        printf(" ");
    struct item *top = (struct item *) malloc(sizeof(item));
    stack_peek(SYN, top);
    printf("%d", top->state);
    printf("   ");
    printf("%c", word);
    for (int i = 0; i < 8; i++)
        printf(" ");

    struct var *p_var = (struct var *) SEM->elems;
    for (int i = 0; i < SEM->pos; i++) {
        printf("%s ", (p_var + i)->str);
    }
    free(top);
}

void print_action() {
    if (action == 0) {
        struct quat *tmp = q - 1;
        printf("\tGEQ (%c, %s, %s, %s)", tmp->op, tmp->op1, tmp->op2, tmp->res);
    } else if (action == 1) {
        struct item *top = (struct item *) malloc(sizeof(item));
        stack_peek(SYN, top);
        printf("\tPUSH");
        free(top);
    }
    action = -1;
    printf("\n");
}
