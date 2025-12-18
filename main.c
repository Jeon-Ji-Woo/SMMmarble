#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define MAX_NODE        100
#define BOARDFILEPATH   "marbleBoardConfig.txt"
#define FOODFILEPATH    "marbleFoodConfig.txt"
#define FESTFILEPATH    "marbleFestivalConfig.txt"

#define MAX_GRADE       9

//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;
static int player_nr;

//player struct 정의
typedef struct player {
    int energy;
    int position;
    char name[MAX_CHARNAME];
    int accumCredit;

    // 수강 과목(lecture_id) + 해당 과목 성적(0~8)
    int flag_graduate[8];
    int grade_history[8];
} player_t;

static player_t cur_player[MAX_PLAYER];

/* -------------------- helpers -------------------- */
static void clear_stdin_line(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

static const char* GRADE_NAME[MAX_GRADE] = {
    "A+","A0","A-",
    "B+","B0","B-",
    "C+","C0","C-"
};

static int random_grade(void)
{
    return rand() % MAX_GRADE; // 0~8
}

/* -------------------- grade printing -------------------- */
void printGrades(int player)
{
    int i;

    printf("======== GRADE HISTORY (%s) ========\n", cur_player[player].name);

    for (i = 0; i < 8; i++) {
        if (cur_player[player].flag_graduate[i] == 0)
            break;

        printf("%d) %s : %s\n",
               i + 1,
               smmObj_getNodeName(cur_player[player].flag_graduate[i]),
               (cur_player[player].grade_history[i] >= 0 && cur_player[player].grade_history[i] < MAX_GRADE)
                   ? GRADE_NAME[cur_player[player].grade_history[i]]
                   : "N/A");
    }

    printf("====================================\n");
}

/* -------------------- banner -------------------- */
static void print_game_banner(void)
{
    printf("\n");
    printf("=======================================================================\n");
    printf("-----------------------------------------------------------------------\n");
    printf("                Sookmyung Marble !! Let's Graduate (total credit : %d)!!\n", GRADUATE_CREDIT);
    printf("-----------------------------------------------------------------------\n");
    printf("=======================================================================\n");
    printf("\n");
}

/* -------------------- status printing -------------------- */
void printPlayerStatus(void)
{
    int i;

    printf("\n");
    printf("========================== PLAYER STATUS ==========================\n");

    for (i = 0; i < player_nr; i++)
    {
        printf("%s at %d.%s, credit: %d, energy: %d\n",
            cur_player[i].name,
            cur_player[i].position,
            smmObj_getNodeName(cur_player[i].position),
            cur_player[i].accumCredit,
            cur_player[i].energy);
    }

    printf("========================== PLAYER STATUS ==========================\n");
    printf("\n");
}

/* -------------------- init players -------------------- */
void generatePlayers(int n, int initEnergy)
{
    int i, j;

    for (i = 0; i < n; i++)
    {
        printf("Input player %d's name: ", i);
        scanf("%s", cur_player[i].name);
        clear_stdin_line();

        cur_player[i].position = 0;
        cur_player[i].energy = initEnergy;
        cur_player[i].accumCredit = 0;

        for (j = 0; j < 8; j++) {
            cur_player[i].flag_graduate[j] = 0;
            cur_player[i].grade_history[j] = -1;
        }
    }
}

/* -------------------- dice -------------------- */
int rolldie(int player)
{
    int c;

    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    clear_stdin_line();

    if (c == 'g' || c == 'G')
        printGrades(player);

    return (rand() % MAX_DIE + 1);
}

/* -------------------- node action -------------------- */
void actionNode(int player, int die_result)
{
    int type = smmObj_getNodeType(cur_player[player].position);

    char choice_lecture;
    int restaurant_node;
    int random_food;
    int random_mission;

    int i;
    int lab_dice;
    int standard_lab;

    switch (type)
    {
    case SMMNODE_TYPE_LECTURE:
    {
        int lecture_id = cur_player[player].position % 16; // 너 코드 규칙 유지
        int already_taken = 0;

        // 1) 이미 수강했는지 검사
        for (i = 0; i < 8; i++) {
            if (cur_player[player].flag_graduate[i] == lecture_id) {
                already_taken = 1;
                break;
            }
            if (cur_player[player].flag_graduate[i] == 0) {
                break;
            }
        }

        if (already_taken) {
            printf("이미 수강한 과목입니다!\n");
            break;
        }

        // 2) 에너지 조건: 현재 에너지 >= 소요 에너지(노드 energy)
        if (cur_player[player].energy < smmObj_getNodeEnergy(cur_player[player].position)) {
            printf("Not enough Energy!\n");
            break;
        }

        // 3) 수강/드랍 선택
        printf("수강하기-> y , 드랍하기-> n : ");
        scanf(" %c", &choice_lecture);   // 앞 공백 중요
        clear_stdin_line();

        if (choice_lecture == 'y' || choice_lecture == 'Y') {
            int g = random_grade();

            printf("수강합니다.\n");
            printf("성적: %s\n", GRADE_NAME[g]);

            // credit/energy 반영
            cur_player[player].accumCredit += smmObj_getNodeCredit(cur_player[player].position);
            cur_player[player].energy      -= smmObj_getNodeEnergy(cur_player[player].position);

            // 수강/성적 기록 저장
            for (i = 0; i < 8; i++) {
                if (cur_player[player].flag_graduate[i] == 0) {
                    cur_player[player].flag_graduate[i] = lecture_id;
                    cur_player[player].grade_history[i] = g;
                    break;
                }
            }
        } else {
            printf("드랍합니다.\n");
        }

        break;
    }

    case SMMNODE_TYPE_RESTAURANT:
        restaurant_node = cur_player[player].position;
        cur_player[player].energy += smmObj_getNodeEnergy(restaurant_node);
        break;

    case SMMNODE_TYPE_HOME:
        if ((cur_player[player].position % 16 == 0) && (cur_player[player].position != 0))
        {
            cur_player[player].energy += smmObj_getNodeEnergy(0);
            printf("집에 도착하여 에너지를 얻습니다.\n");
        }
        break;

    case SMMNODE_TYPE_FOODCHANCE:
        random_food = rand() % 14;
        cur_player[player].energy += smmObj_getFoodEnergy(random_food);
        printf("%s를(을) 먹었습니다.\n", smmObj_getFoodName(random_food));
        break;

    case SMMNODE_TYPE_GOTOLAB:
        if (cur_player[player].position % 16 == 8 || cur_player[player].position % 16 == 12)
        {
            if (cur_player[player].position % 16 == 12)
            {
                printf("실험중 상태로 전환하며 실험실로 이동합니다.\n");
                cur_player[player].position -= 4;
            }

            printf("실험 성공 기준값을 입력하시오 1-6까지 중 하나: ");
            scanf("%d", &standard_lab);
            clear_stdin_line();

            for (i = 0; i < MAX_NODE; i++)
            {
                cur_player[player].energy -= smmObj_getNodeEnergy(cur_player[player].position);
                lab_dice = (rand() % MAX_DIE + 1);

                printf("%d만큼 주사위를 굴렸습니다.\n", lab_dice);

                if (lab_dice >= standard_lab)
                {
                    printf("finish experiment!!!\n");
                    break;
                }
                else
                {
                    printf("fail experiment again....\n");
                }
            }
        }
        break;

    case SMMNODE_TYPE_FESTIVAL:
        random_mission = rand() % 5;
        if (cur_player[player].position % 16 == 10)
        {
            printf("====================mission start====================\n");
            printf("'%s'\n", smmObj_getfestivalName(random_mission));
            printf("=====================================================\n");
        }
        break;

    default:
        break;
    }

    // 집을 지나쳐서 에너지를 얻는 규칙 유지
    if (cur_player[player].position % 16 != 0 && cur_player[player].position % 16 - die_result < 0)
    {
        cur_player[player].energy += smmObj_getNodeEnergy(0);
        printf("집을 지나쳐서 에너지를 얻습니다.\n");
    }
}

/* -------------------- move -------------------- */
void goForward(int player, int step)
{
    cur_player[player].position += step;

    printf("%s go to node %d (name: %s)\n",
        cur_player[player].name,
        cur_player[player].position,
        smmObj_getNodeName(cur_player[player].position));
}

/* -------------------- main -------------------- */
int main(int argc, const char* argv[])
{
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy = 0;
    int turn = 0;

    (void)argc;
    (void)argv;

    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;

    srand((unsigned)time(NULL));

    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig
    if ((fp = fopen(BOARDFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        return -1;
    }

    printf("Reading board component......\n");
    while (fscanf(fp, "%s %d %d %d", name, &type, &credit, &energy) == 4)
    {
        smmObj_genNode(name, type, credit, energy);
        if (type == SMMNODE_TYPE_HOME)
            initEnergy = energy;
        board_nr++;
    }
    fclose(fp);

    printf("Total number of board nodes : %d\n", board_nr);

    for (i = 0; i < board_nr; i++)
        printf("node %2d : %15s, %d(%s), credit %d, energy %d\n",
            i, smmObj_getNodeName(i),
            smmObj_getNodeType(i), smmObj_getTypeName(smmObj_getNodeType(i)),
            smmObj_getNodeCredit(i), smmObj_getNodeEnergy(i));

    //2. food card config -----------------------------------------------------------------------------------
    if ((fp = fopen(FOODFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }

    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %d", name, &energy) == 2)
    {
        smmObj_genfood(name, energy);
        food_nr++;
    }
    fclose(fp);

    printf("Total number of food cards : %d\n", food_nr);
    for (i = 0; i < food_nr; i++)
        printf("food %2d : %8s, energy %d\n", i, smmObj_getFoodName(i), smmObj_getFoodEnergy(i));

    //3. festival card config --------------------------------------------------------------------------------
    if ((fp = fopen(FESTFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }

    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", name) == 1)
    {
        smmObj_genfestival(name);
        festival_nr++;
    }
    fclose(fp);

    printf("Total number of festival cards : %d\n", festival_nr);
    for (i = 0; i < festival_nr; i++)
        printf("%s\n", smmObj_getfestivalName(i));

    printf("\n\n");

    //2. Player configuration ---------------------------------------------------------------------------------
    print_game_banner();

    do
    {
        printf("input player no.: ");
        scanf("%d", &player_nr);
        clear_stdin_line();
    } while (player_nr <= 0 || player_nr > MAX_PLAYER);

    generatePlayers(player_nr, initEnergy);

    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1)
    {
        int die_result;
        int flag = 0;
        int winner = -1;

        //4-1. initial printing
        printPlayerStatus();

        //4-2. die rolling
        die_result = rolldie(turn);

        //4-3. go forward
        goForward(turn, die_result);

        //4-4. take action
        actionNode(turn, die_result);

        //게임 종료 조건
        for (i = 0; i < player_nr; i++) {
            if (cur_player[i].position >= MAX_NODE) {
                flag = 1;
                winner = i;
            } else if (cur_player[i].accumCredit >= GRADUATE_CREDIT) {
                flag = 2;
                winner = i;
            }
        }

        if (flag == 1)
        {
            printf("player가 node를 벗어났습니다. 게임종료\n");
            break;
        }
        else if (flag == 2)
        {
            printf("졸업학점을 이수하였습니다. 게임종료 집으로 가시오\n");
            printf("%s이(가) 수강한 과목/성적\n", cur_player[winner].name);
            printGrades(winner);
            printf("total credit : %d\n", cur_player[winner].accumCredit);
            break;
        }

        //4-5. next turn
        turn = (turn + 1) % player_nr;
    }

    system("PAUSE");
    return 0;
}

