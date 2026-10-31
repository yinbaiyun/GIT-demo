//用到的知识: 数组 / 循环 / 函数 / 指针 / 结构体
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS   9    
#define COLS   9
#define MINES  10

//结构体 1：一个格子。
typedef struct
{
    int mine;     // 1 = 有雷，0 = 没雷       
    int state;    // 0 = 未翻开，1 = 已翻开，2 = 插旗
    int around;   // 周围 8 格里有几颗雷
} Cell;

//结构体 2：一整局游戏
typedef struct
{
    Cell cell[ROWS][COLS];   
    int  opened;             //已翻开格子数
    int  flags;              //已插旗数 
    int  started;            //是否开始
    int  result;             //游戏结果
} Game;

//返回某一格的指针；越界就返回 NULL。
   
Cell *getCell(Game *g, int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
    {
        return NULL;             /* 指针可以是 NULL，用之前必须判空 */
    }
    return &g->cell[r][c];       /* & 取地址：把这一格的地址交出去 */
}

//数 (r, c) 周围有几颗雷  形参 board[][COLS]会被编译器当成指针    "数组传参退化成指针"。
int countAround(Cell board[][COLS], int r, int c)
{
    int dr, dc, nr, nc;
    int n = 0;

    for (dr = -1; dr <= 1; dr++)
    {
        for (dc = -1; dc <= 1; dc++)
        {
            if (dr == 0 && dc == 0) continue;      
            nr = r + dr;
            nc = c + dc;
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (board[nr][nc].mine == 1) n++;
        }
    }
    return n;
}

// 开局清零。要修改调用者的结构体，必须传地址（Game *）。
   
void initGame(Game *g)
{
    int r, c;

    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            g->cell[r][c].mine   = 0;    /* -> 是"指针取成员" */
            g->cell[r][c].state  = 0;
            g->cell[r][c].around = 0;
        }
    }

    g->opened  = 0;
    g->flags   = 0;
    g->started = 0;
    g->result  = 0;
}

//随机放 MINES 颗，且第一下点的周围 3x3 不放雷。

void placeMines(Game *g, int sr, int sc)
{
    int k = 0, r, c;

    while (k < MINES)
    {
        r = rand() % ROWS;
        c = rand() % COLS;

        if (abs(r - sr) <= 1 && abs(c - sc) <= 1) continue;  
        if (g->cell[r][c].mine == 1) continue;                

        g->cell[r][c].mine = 1;
        k++;
    }

    //存雷
    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            g->cell[r][c].around = countAround(g->cell, r, c);
        }
    }
}

//翻开一格；如果周围 0 颗雷，就向外连锁展开     栈
  
void openCell(Game *g, int r, int c)
{
    int stackR[ROWS * COLS * 8];
    int stackC[ROWS * COLS * 8];
    int top = 0;
    int dr, dc, nr, nc;
    Cell *p;

    stackR[top] = r;
    stackC[top] = c;
    top++;

    while (top > 0)
    {
        top--;
        r = stackR[top];
        c = stackC[top];

        p = getCell(g, r, c);        
        if (p == NULL) continue;     // 越界 
        if (p->state != 0) continue; // 已翻开或已插旗
        if (p->mine == 1) continue;  // 是雷就不自动翻

        p->state = 1;                // 通过指针修改格子
        g->opened++;

        if (p->around != 0) continue;   // 周围有雷就停在这里

        for (dr = -1; dr <= 1; dr++)
        {
            for (dc = -1; dc <= 1; dc++)
            {
                if (dr == 0 && dc == 0) continue;
                nr = r + dr;
                nc = c + dc;
                stackR[top] = nr;
                stackC[top] = nc;
                top++;
            }
        }
    }
}

//打印棋盘  const Game *g
   
void printBoard(const Game *g)
{
    int r, c;
    const Cell *p;

    printf("\n    ");
    for (c = 0; c < COLS; c++) printf("%d ", c + 1);
    printf("\n   ");
    for (c = 0; c < COLS; c++) printf("--");
    printf("\n");

    for (r = 0; r < ROWS; r++)
    {
        printf("%2d |", r + 1);
        for (c = 0; c < COLS; c++)
        {
            p = &g->cell[r][c];     

            if (p->state == 1)                  /* 已翻开 */
            {
                if (p->mine == 1)            printf(" *");
                else if (p->around == 0)     printf(" .");
                else                         printf(" %d", p->around);
            }
            else if (p->state == 2)              // 插旗 
            {
                printf(" F");
            }
            else if (g->result == 1 && p->mine == 1)   // 输了亮出所有雷 
            {
                printf(" *");
            }
            else                                  // 没翻开 
            {
                printf(" #");
            }
        }
        printf("\n");
    }
}

//主函数 
int main(void)
{
    Game  game;             
    Game *pg = &game;        
    char  line[64];
    char  op;
    int   r, c;

    srand((unsigned)time(NULL));
    initGame(pg);

    printf("===== 扫雷  9x9  10 颗雷 =====\n");
    printf("翻开： 行 列          例如  5 5\n");
    printf("插旗： 行 列 f        例如  5 5 f\n");

    while (pg->result == 0)
    {
        printBoard(pg);
        printf("剩余雷数 %d    已翻开 %d\n", MINES - pg->flags, pg->opened);
        printf("你的操作: ");

        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break;                   
        }

        op = 0;
        if (sscanf(line, "%d %d %c", &r, &c, &op) < 2)
        {
            printf("格式不对，请输入两个数字。\n");
            continue;
        }

        r--;                           
        c--;
        if (r < 0 || r >= ROWS || c < 0 || c >= COLS)
        {
            printf("坐标超出范围（1~9）。\n");
            continue;
        }

        if (op == 'f' || op == 'F')    //插旗
        {
            Cell *p = getCell(pg, r, c);

            if (p->state == 1)
            {
                printf("这一格已经翻开了，不能插旗。\n");
            }
            else if (p->state == 2)
            {
                p->state = 0;
                pg->flags--;
            }
            else
            {
                p->state = 2;
                pg->flags++;
            }
        }
        else                           //翻开格子
        {
            Cell *p = getCell(pg, r, c);

            if (p->state != 0)
            {
                printf("这一格翻不开（已翻开或已插旗）。\n");
                continue;
            }

            if (pg->started == 0)      //第一次点击后布雷。
            {
                placeMines(pg, r, c);
                pg->started = 1;
            }

            if (p->mine == 1)
            {
                p->state = 1;
                pg->result = 1;
            }
            else
            {
                openCell(pg, r, c);
                if (pg->opened == ROWS * COLS - MINES)
                {
                    pg->result = 2;
                }
            }
        }
    }

    printBoard(pg);

    if (pg->result == 2)
    {
        printf("\n恭喜，雷区全部扫清！\n");
    }
    else if (pg->result == 1)
    {
        printf("\n踩到雷了，游戏结束。\n");
    }
    else
    {
        printf("\n已退出。\n");
    }

    return 0;
}