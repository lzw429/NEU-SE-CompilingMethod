//
// Created by Yiheng SHU on 12/23/18.
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include <ctype.h>

#define OK 20 // 分析结束状态
#define PRODUCTION_SIZE 6
struct quat { // 四元式
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

struct production { // 产生式
    int state_case; // 产生式编号
    int act; // 动作：GEQ 0, PUSH 1，无 -1
    int poll_times; // 弹栈次数
    int SLR_col; // 更新 state 需要查询的 SLR col
    char word; // 产生式左侧
} productions[PRODUCTION_SIZE] = {
        {-1, 0,  3, 6, 'E'},
        {-2, -1, 1, 6, 'E'},
        {-3, 0,  3, 7, 'T'},
        {-4, -1, 1, 7, 'T'},
        {-5, -1, 3, 8, 'F'},
        {-6, 1,  1, 8, 'F'}
};

stack *SYN; // 算符栈
stack *SEM; // 语义栈
struct item *item; // 用于 SYN 栈的临时结构体变量
struct var *var; // 用于 SEM 栈的临时结构体变量
struct quat quats[20]; // 生成的四元式数组
struct quat *q = quats; // 四元式数组指针
int state = 0; // 状态
int col = 0; // 列
int action = -1; // 执行子程序：GEQ 0, PUSH 1

void print_quat(); // 打印生成的所有四元式
void print_stacks(char word); // 打印 SYN 和 SEM 栈
void print_action(); // 打印动作：GEQ 或 PUSH
void GEQ(); // 生成四元式
void statute(); // 执行规约

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
}; // SLR 分析表

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
    if (len == 0) {
        printf("[Info] The expression is empty");
        return 0;
    }
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

    // 表头
    printf("SYN                                               top  w       SEM\n");
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
            statute();
        } else if (state == 0) {
            // 未期望的符号
            printf("\n[Error] Invalid expression, unexpected character: %c\n", *code);
            break;
        } else if (state == OK) {
            // 分析结束
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
        print_action(); // 打印动作信息
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
 * 打印四元式数组中的所有四元式
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

/**
 * 打印 SYN 和 SEM 栈
 * @param word 当前单词
 */
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

/**
 * 打印动作：GEQ 或 PUSH
 */
void print_action() {
    if (action == 0) { // 执行了 GEQ
        struct quat *tmp = q - 1;
        printf("\tGEQ (%c, %s, %s, %s)", tmp->op, tmp->op1, tmp->op2, tmp->res);
    } else if (action == 1) { // 执行了 PUSH
        struct item *top = (struct item *) malloc(sizeof(item));
        stack_peek(SYN, top);
        printf("\tPUSH");
        free(top);
    }
    action = -1; // 重置
    printf("\n");
}

void statute() {
    struct production *p = productions;
    int i;
    for (i = 0; i <= PRODUCTION_SIZE; i++) {
        if ((p + i)->state_case == state) { break; }
    }
    if (i == PRODUCTION_SIZE)
        return;
    p = p + i;
    switch (p->act) {
        case 0: // GEQ
            GEQ();
            break;
        case 1: // PUSH
            stack_peek(SYN, item);
            strcpy(var->str, &item->word);
            stack_push(SEM, var);
            action = 1;
            break;
        default:
            break;
    }
    for (i = 0; i < p->poll_times; i++) {
        stack_poll(SYN, item);
    }
    stack_peek(SYN, item);
    item->state = SLR[item->state][p->SLR_col];
    item->word = p->word;
    stack_push(SYN, item);
}