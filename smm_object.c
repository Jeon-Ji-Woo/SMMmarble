#include "smm_common.h"
#include "smm_object.h"
#include <string.h>
#define MAX_NODETYPE    7
#define MAX_GRADE       9
#define MAX_NODE        100
static char smmNodeName[SMMNODE_TYPE_MAX][MAX_CHARNAME] = {
       "강의",
       "식당",
       "실험실",
       "집",
       "실험실로이동",
       "음식찬스",
       "축제시간"
};
char* smmObj_getTypeName(int type)
{
      return (char*)smmNodeName[type];
}
//1. 구조체 형식 정의
typedef struct smmObject {
       char name[MAX_CHARNAME];
       int type;
       int credit;
       int energy;
} smmObject_t;

//2. 구조체 배열 변수 정의 
static smmObject_t smm_node[MAX_NODE];

static int smmObj_noNode = 0;

//3. 관련 함수 변경 
//object generation
void smmObj_genNode(char* name, int type, int credit, int energy)
{    
    strcpy(smm_node[smmObj_noNode].name, name);
    smm_node[smmObj_noNode].type = type;
    smm_node[smmObj_noNode].credit = credit;
    smm_node[smmObj_noNode].energy = energy;
    
    smmObj_noNode++;
}

//node 구성 
char* smmObj_getNodeName(int node_nr)
{
    return smm_node[node_nr%16].name;
}

 
int smmObj_getNodeType(int node_nr)
{
    return smm_node[node_nr%16].type; //node type 
}
int smmObj_getNodeCredit(int node_nr)
{
    return smm_node[node_nr%16].credit;//node credit
}
int smmObj_getNodeEnergy(int node_nr)
{
    return smm_node[node_nr%16].energy;//node energy;
}

//음식 카드 
//food struct
typedef struct smmObject_food {
	char name[MAX_CHARNAME];
	int energy;
} smmObject_food;

static smmObject_food smm_food[MAX_NODE];

static int smmObj_noFood = 0;

void smmObj_genfood(char* name, int energy)
{
	strcpy(smm_food[smmObj_noFood].name, name);
	smm_food[smmObj_noFood].energy = energy;
	
	smmObj_noFood++;
}

char* smmObj_getFoodName(int food_nr)
{
	return smm_food[food_nr].name; //food name
}

int smmObj_getFoodEnergy(int food_nr)
{
	return smm_food[food_nr].energy; //food energy
}

//축제카드
//festival struct
typedef struct smmObject_festival {
	char name[MAX_CHARNAME];
} smmObject_festival;

static smmObject_festival smm_festival[MAX_NODE];

static int smmObj_nofestival = 0;

void smmObj_genfestival(char* name)
{
	strcpy(smm_festival[smmObj_nofestival].name, name);
	
	smmObj_nofestival++;
}

char* smmObj_getfestivalName(int festival_nr)
{
	return smm_festival[festival_nr].name; //festival mission get name
}



 
