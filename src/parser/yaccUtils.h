#ifndef YACCUTILS_H
#define YACCUTILS_H

#include "MiniC.tab.hpp"
#include "symbol.h"
#include "yaccType.h"
#include "expression.h"
#include <iostream>
#define debug(x) std::cerr<<#x<<"="<<x<<std::endl

Typename_t *newEnum(char *, EnumTable_t *);
Typename_t *newStructUnion(bool, const char *, bool);

template<class T> T *memDup(const T *src)
{
    return (T *)memcpy(new T, src, sizeof(T));
}

void arrayLengthSum(const_Typename_ptr);

void genDeclare(const_Typename_ptr type, const char *TACname, bool global);
void genInitilize(const_Typename_ptr, const char *TACname, const initializer_s_t *, bool outputPTR = false);


char *get_TAC_name(char TAC_name_prefix, int TAC_num);
char *get_cast_name(IdType_t goal_type, IdType_t now_type, const char *now_name);
//判断类型是否一样，一样返回now_name,否则自动产生新临时变量，并返回新变量的名字
extern const char map_name[IDTYPE_NUM][10];
bool check_float(IdType_t now_type);//判断是否为浮点数
bool check_pointer(IdType_t now_type);//判断是否为指针
bool check_int(IdType_t now_type);//判断是否为整数
bool check_number(IdType_t now_type);//判断是否为数字
bool check_str_un(IdType_t now_type);//判断是否为struct/union


bool check_float(expression_s_t now);//判断是否为浮点数
bool check_pointer(expression_s_t now);//判断是否为指针
bool check_int(expression_s_t now);//判断是否为整数
bool check_number(expression_s_t now);//判断是否为数字
bool check_str_un(expression_s_t now);//判断是否为struct/union

void declareParameter(const SymbolList_t *);
expression_s_t get_c0_c1_exp(const char *name); //用来生成一个addr为c0或者c1的expression
void genIfGoto(expression_s_t, const char *name2, const char *op, int num);

const_Typename_ptr addConst(const_Typename_ptr type);

#endif
