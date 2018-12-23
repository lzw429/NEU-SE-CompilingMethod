//
// Created by Yiheng SHU on 2018/12/9.
//

#include <string.h>
#include <stdio.h>
#include <math.h>

// 代码关键字
char keywords[30][12] = {"program", "begin", "end", "var", "while", "do", "repeat",
                         "until", "for", "to", "if", "then", "else", "Double", ";", ":", "(", ")", ",",
                         ":=", "+", "-", "*", "/", ">", ">=", "==", "<", "<="};

// 关键字数量
int num_key = 28;

// 状态转换矩阵
int aut[11][8] = {0, 0, 0, 0, 0, 0, 0, 0,
                  0, 2, 0, 0, 0, 8, 9, 15,
                  0, 2, 3, 5, 11, 0, 0, 11,
                  0, 4, 0, 0, 0, 0, 0, 0,
                  0, 4, 0, 5, 11, 0, 0, 11,
                  0, 7, 0, 0, 6, 0, 0, 0,
                  0, 7, 0, 0, 0, 0, 0, 0,
                  0, 7, 0, 0, 11, 0, 0, 11,
                  0, 8, 0, 0, 0, 8, 0, 12,
                  0, 0, 0, 0, 0, 0, 10, 14,
                  0, 0, 0, 0, 0, 0, 0, 13};

char ID[50][12]; // 存储源程序中的标识符，如变量、常量、数组名
double C[20]; // 存储源程序中的常数值
int num_ID = 0, num_C = 0; // 标识符和常数的个数

/* 其他变量 */

// Token 结构
struct token {
    int code; // 1 标识符，2 常数，3 及以后关键字
    int value;
};
struct token tok[100]; // Token 数组
int i_token = 0, num_token = 0; // Token 计数器和 Token 个数
char strTOKEN[15]; // 当前单词
int i_str; // 当前单词指针
int n, p, m, e, t; // 尾数值，指数值，小数位数，指数符号，类型
double num; // 常数值
char buf[50]; // 源程序缓冲区
int i_buf; // 源程序缓冲区指针，当前字符为 buf[i_buf]

struct map // 当前字符到状态转换矩阵列标记的映射
{
    char str[50];
    int col;
};
struct map col1[4] = {{"0123456789", 1},
                      {".",          2},
                      {"Ee",         3},
                      {"+-",         4}}; // 数字
struct map col2[2] = {{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 5},
                      {"0123456789",                                           1}}; // 关键字或标识符
struct map col3[1] = {{";:(),+-*/=><", 6}}; // 界符
struct map *ptr_map;
int num_map; // map 数量


/* 函数声明 */

/**
 * 执行语义动作
 * @param state 当前状态
 */
void act(int state);


/**
 * 查询状态转换矩阵，获取新状态
 * @param state 当前状态
 * @param ch 当前字符
 * @return 转换后状态
 */
int find(int state, char ch);


/**
 * 将 num 插入到常数表中
 * @param num 常数
 * @return 常数表索引
 */
int InsertConst(double num);


/**
 *
 * @param str 潜在关键字
 * @return token code，从 3 开始；1 是标识符，2 是常数
 */
int Reserve(char *str);


/**
 * 将 str 插入到标识符表
 * @param str 标识符
 * @return 标识符表索引
 */
int InsertID(char *str);

int main(int argc, char *argv[]) {
    int state; // 当前状态
    FILE *fp = fopen("exa.txt", "r"); // 读取文件
    if (fp == NULL) {
        printf("[Error] File cannot be open\n");
        return 0;
    }
    printf("[Info] Scan start...\n");
    int Line = 0;
    while (!feof(fp)) {
        fgets(buf, 50, fp);
        i_buf = 0;
        ++Line;
        do {
            while (buf[i_buf] == ' ') // 跳过源程序空格
                i_buf++;

            if ((buf[i_buf] >= 'a' && buf[i_buf] <= 'z')
                || (buf[i_buf] >= 'A' && buf[i_buf] <= 'Z')) {
                // 读取到字母
                ptr_map = col2;
                num_map = 2;
            } else if (buf[i_buf] >= '0' && buf[i_buf] <= '9') {
                // 读取到数字
                ptr_map = col1;
                num_map = 4;
            } else if (strchr(col3[0].str, buf[i_buf]) == NULL) {
                // 读取到关键字或标识符
                if (buf[i_buf] == '\n')
                    continue;
                printf("[Error] Invalid character: Line %d, column %d\n", Line, i_buf + 1); // 非法字符
                printf("%s", buf);
                for (int s = 0; s < i_buf; s++)
                    printf(" ");
                printf("^\n"); // 显示非法字符位置
                i_buf++;
                continue;
            } else {
                ptr_map = col3;
                num_map = 1;
            }

            i_buf--;
            state = 1; // 开始处理一个单词
            while (state != 0) {
                act(state);
                if (state >= 11 && state <= 14)
                    break;
                i_buf++;
                state = find(state, buf[i_buf]); // 状态转换
            }
            if (state == 0) {
                printf("[Error] Lexical error: Line %d, column %d\n%s", Line, i_buf, buf); // 词法错误
                for (int s = 0; s < i_buf - 1; s++)
                    printf(" ");
                printf("^\n"); // 显示词法错误位置
            }
        } while ((buf[i_buf] != 10) && !feof(fp)); // 读到换行为止
    }

    /* 输出结果 */
    printf("[Info] Keyword table: "); // 关键字表
    for (i_buf = 0; i_buf < 30; i_buf++) {
        printf("%s ", keywords[i_buf]);
    }
    printf("\n");
    printf("[Info] Token sequence: "); // Token 序列
    for (i_buf = 0; i_buf < num_token; i_buf++)
        printf("(%d, %d)", tok[i_buf].code, tok[i_buf].value);
    printf("\n");
    printf("[Info] Symbol table: "); // 符号表
    for (i_buf = 0; i_buf < num_ID; i_buf++)
        printf("%s ", ID[i_buf]);
    printf("\n");
    printf("[Info] Constant table: ");  // 常数表
    for (i_buf = 0; i_buf < num_C; i_buf++)
        printf("%lf ", C[i_buf]);
    printf("\n");

    fclose(fp); // 关闭文件
    printf("[Info] Scan completed\n"); // 扫描完成
    return 0;
}

/**
 * 执行语义动作
 * @param state 当前状态
 */
void act(int state) {
    int code;
    switch (state) {
        case 1:
            n = 0; // 尾数值
            m = 0; // 指数值
            p = 0; // 小数位数
            t = 0; // 指数符号
            e = 1; // 类型
            num = 0; // 常数值
            i_str = 0; // 当前单词指针
            strTOKEN[i_str] = '\0'; // 当前单词末尾添加 '\0'
            break;
        case 2:
            n = 10 * n + buf[i_buf] - '0';
            break;
        case 3:
            t = 1;
            break;
        case 4:
            n = 10 * n + buf[i_buf] - '0';
            m++;
            break;
        case 5:
            t = 1;
            break;
        case 6:
            if (buf[i_buf] == '-') e = -1;
            break;
        case 7:
            p = 10 * p + buf[i_buf] - '0';
            break;
        case 8:
            strTOKEN[i_str++] = buf[i_buf]; // 将ch中的符号拼接到strTOKEN的尾部；
            break;
        case 9:
            strTOKEN[i_str++] = buf[i_buf]; // 将ch中的符号拼接到strTOKEN的尾部；
            break;
        case 10:
            strTOKEN[i_str++] = buf[i_buf]; // 将ch中的符号拼接到strTOKEN的尾部；
            break;
        case 11:
            num = n * pow(10, e * p - m); // 计算常数值
            tok[i_token].code = 2; // 常数
            tok[i_token++].value = InsertConst(num);  // 生成常数Token
            num_token++;
            break;
        case 12:
            strTOKEN[i_str] = '\0';
            code = Reserve(strTOKEN); // 查关键字表
            if (code) {
                tok[i_token].code = code;
                tok[i_token++].value = 0;
            }   // 生成关键字Token
            else {
                tok[i_token].code = 1; // 标识符
                tok[i_token++].value = InsertID(strTOKEN);
            }    // 生成标识符Token
            num_token++;
            break;
        case 13:
            strTOKEN[i_str] = '\0';
            code = Reserve(strTOKEN); // 查界符表
            if (code) {
                // 生成界符Token
                tok[i_token].code = code;
                tok[i_token++].value = 0;
            } else {
                strTOKEN[strlen(strTOKEN) - 1] = '\0'; // 单界符
                i_buf--;
                code = Reserve(strTOKEN); // 查界符表
                tok[i_token].code = code;
                tok[i_token++].value = 0; // 生成界符 Token
            }
            num_token++;
            break;
        case 14:
            strTOKEN[i_str] = '\0';
            code = Reserve(strTOKEN); // 查界符表
            tok[i_token].code = code;
            tok[i_token++].value = 0; // 生成界符Token
            num_token++;
            break;
        default:
            printf("[Error] State error\n");
            break;
    }
}


/**
 * 查询状态转换矩阵，获取新状态
 * @param state 当前状态
 * @param ch 当前字符
 * @return 转换后状态
 */
int find(int state, char ch) {
    int i, col = 7;
    struct map *p;
    p = ptr_map;
    for (i = 0; i < num_map; i++) {
        if (strchr((p + i)->str, ch)) {
            col = (p + i)->col;
            break;
        }
    }
    return aut[state][col];
}


/**
 * 将 str 插入到标识符表
 * @param str 标识符
 * @return 标识符表索引
 */
int InsertID(char *str) {
    int i;
    for (i = 0; i < num_ID; i++)
        if (!strcmp(ID[i], str)) // 匹配已有标识符
            return i;
    strcpy(ID[i], str);
    num_ID++;
    return i;
}


/**
 * 将 num 插入到常数表中
 * @param num 常数
 * @return 常数表索引
 */
int InsertConst(double num) {
    int i;
    for (i = 0; i < num_C; i++)
        if (num == C[i])
            return i;
    C[i] = num;
    num_C++; // 常数个数
    return i;
}

/**
 *
 * @param str 潜在关键字
 * @return token code，从 3 开始；1 是标识符，2 是常数
 */
int Reserve(char *str) {
    int i;
    for (i = 0; i < num_key; i++)
        if (!strcmp(keywords[i], str))
            return (i + 3);
    return 0;
}