#include "yaccUtils.h"
#include "gen.h"
#include "expression.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

Typename_t *newEnum(char *name, EnumTable_t *p)
{
    Typename_t *t = new Typename_t;
    t->type = idt_int; // idt_enum is replaced by idt_int
    t->size = 4;
    if (name)
        t->name = strdup((std::string("enum ") + name).c_str());
    else
        t->name = NULL;
    t->structure = new IdStructure_t;
    t->structure->enumerate = p;
    StackAddTypename(t);
    return t;
}

Typename_t *newStructUnion(bool hasSTRUCT, const char *name, bool hasSymbol)
{
    Typename_t *t = new Typename_t;
    t->isConst = 0;
    if (hasSTRUCT) {
        t->type = idt_struct;
        if (!hasSymbol)
            t->size = -1;
        else {
            if (symbolStack->idList)
                t->size = symbolStack->idList->offset + symbolStack->idList->id->type->size;
            else
                t->size = 0;
        }
    }
    else {
        t->type = idt_union;
        if (!hasSymbol)
            t->size = -1;
        else {
            t->size = 0;
            for (SymbolList_t *i = symbolStack->idList; i; i = i->next)
                if (i->id->type->size > t->size)
                    t->size = i->id->type->size;
        }
    }
    if (name) {
        if (t->type == idt_union)
            t->name = strdup((std::string("union ") + name).c_str());
        else
            t->name = strdup((std::string("struct ") + name).c_str());
    }
    else
        t->name = NULL;
    if (hasSymbol) {
        t->structure = new IdStructure_t;
        t->structure->record = symbolStack->idList;
        t->serial_number = NextSerialNumber();
    }
    else {
        t->structure = NULL;
        t->serial_number = -1;
    }
    return t;
}

void genDeclare(const_Typename_ptr type, const char *TACname, bool global)
{
    auto autogen = global ? gen_gvar : gen_var;
    switch (type->type) {
    case idt_char:
        autogen("int1", TACname, -1);
        break;
    case idt_short:
        autogen("int2", TACname, -1);
        break;
    case idt_int:
        autogen("int4", TACname, -1);
        break;
    case idt_long:
        autogen("int8", TACname, -1);
        break;
    case idt_uchar:
        autogen("uint1", TACname, -1);
        break;
    case idt_ushort:
        autogen("uint2", TACname, -1);
        break;
    case idt_uint:
        autogen("uint4", TACname, -1);
        break;
    case idt_ulong:
        autogen("uint8", TACname, -1);
        break;
    case idt_float:
        autogen("float4", TACname, -1);
        break;
    case idt_double:
        autogen("float8", TACname, -1);
        break;
    case idt_pointer:
        autogen("ptr", TACname, -1);
        break;
    case idt_fpointer:
        break;
    case idt_void:
        yyerror("declaration of void type");
    case idt_array:
        autogen("ptr", TACname, type->size);
        break;
    case idt_struct:
        autogen("ptr", TACname, type->size);
        break;
    case idt_union:
        autogen("ptr", TACname, type->size);
        break;
    case idt_enum:
        autogen("uint4", TACname, -1);
        break;
    }
}

void genInitilize(const_Typename_ptr type, const char *TACname, const initializer_s_t *init, bool outputPTR)
{
    if (init == NULL)
        return;
    if (type == NULL || TACname == NULL)
        yyerror("no id for initilization");
    if (init->data.addr || init->data.laddr) {
        if (init->data.type->type == idt_array && init->data.type->structure->pointer.base_type->type == idt_char
            && type->type == idt_array && type->structure->pointer.base_type->type == idt_char) { /* assign for string */
            int tnum1 = CreateTempVar(), tnum2 = CreateTempVar();
            char *tname1 = strdup(('t' + std::to_string(tnum1)).c_str());
            char *tname2 = strdup(('t' + std::to_string(tnum2)).c_str());
            gen_var("ptr", tname1);
            gen_var("ptr", tname2);
            gen_cpy(tname1, TACname);
            gen_cpy(tname2, init->data.addr);
            int tnum3 = CreateTempVar();
            char *tname3 = strdup(('t' + std::to_string(tnum3)).c_str());
            gen_var("int1", tname3);
            int slen = init->data.type->structure->pointer.length;
            for (int i = 0; i < slen; ++i) {
                gen_op1(tname3, tname2, "*");
                gen_pnt_cpy(tname1, tname3);
                gen_op2(tname1, tname1, "t1", "+");
                gen_op2(tname2, tname2, "t1", "+");
            }
            gen_cast(tname3, "t0", "int1");
            for (int i = slen; i < type->structure->pointer.length; ++i) {
                gen_pnt_cpy(tname1, tname3);
                gen_op2(tname1, tname1, "t1", "+");
            }
            free(tname1);
            free(tname2);
            free(tname3);
            return;
        }
        expression_s_t eA;
        eA.isConst = 0;
        eA.type = type;
        eA.lr_value = 0;
        if (outputPTR)
            eA.laddr = strdup(TACname), eA.addr = NULL;
        else
            eA.laddr = NULL, eA.addr = strdup(TACname);
        get_assign(eA, init->data, false);
        if (eA.laddr)
            free(eA.laddr);
        else
            free(eA.addr);
    }
    else {
        int tnum = CreateTempVar();
        char *tname = strdup(('t' + std::to_string(tnum)).c_str());
        gen_var("ptr", tname);
        std::vector<initializer_s_t> vinit;
        for (initializer_list_s_t *i = init->lst; i; i = i->next)
            vinit.push_back(i->data);
        auto init_it = vinit.rbegin();
        initializer_s_t noinit;
        noinit.lst = NULL;
        noinit.data.isConst = 1;
        noinit.data.type = (const_Typename_ptr)LookupSymbol("int", NULL);
        noinit.data.lr_value = 1;
        noinit.data.value.vint = 0;
        noinit.data.laddr = NULL;
        char c0[] = "t0";
        if (type->type == idt_array) {
            gen_cpy(tname, TACname);
            const_Typename_ptr btp = type->structure->pointer.base_type;
            int csize = CreateTempVar();
            std::string scsize = 't' + std::to_string(csize);
            gen_const("int4", scsize.c_str(), &btp->size);

            for (int i = 0; i < type->structure->pointer.length; ++i) {
                if (init_it == vinit.rend()) {
                    noinit.data.addr = (btp->type == idt_struct || btp->type == idt_array ? NULL : c0);
                    genInitilize(btp, tname, &noinit, true);
                }
                else
                    genInitilize(btp, tname, &*(init_it++), true);
                gen_op2(tname, tname, scsize.c_str(), "+");
            }
        }
        else if (type->type == idt_struct) {
            std::vector<SymbolList_t *> vsl;
            for (SymbolList_t *i = type->structure->record; i; i = i->next)
                vsl.push_back(i);
            for (auto vsl_it = vsl.rbegin(); vsl_it != vsl.rend(); ++vsl_it) {
                int csize = CreateTempVar();
                std::string scsize = 't' + std::to_string(csize);
                gen_const("int4", scsize.c_str(), &(*vsl_it)->offset);
                gen_op2(tname, TACname, scsize.c_str(), "+");
                const_Typename_ptr btp = (*vsl_it)->id->type;

                if (init_it == vinit.rend()) {
                    noinit.data.addr = (btp->type == idt_struct || btp->type == idt_array ? NULL : c0);
                    genInitilize(btp, tname, &noinit, true);
                }
                else
                    genInitilize(btp, tname, &*(init_it++), true);
            }
        }
        else
            yyerror("initilize error");
        free(tname);
    }
}

char *get_TAC_name(char TAC_name_prefix, int TAC_num) //注意，这玩意会内存泄漏。。。
{
    return strdup((TAC_name_prefix + std::to_string(TAC_num)).c_str());
}


const char map_name[IDTYPE_NUM][10] = {"int1", "int2", "int4", "int8", "uint1", "uint2", "uint4", "uint8", "float4", "float8",
                                       "ptr", "ptr", "(error)", "(error)", "(error)", "(error)", "(error)"
                                      };
char *get_cast_name(IdType_t goal_type, IdType_t now_type, const char *now_name) //该函数基本不判断合法性
{
    if (!(goal_type < 12 || goal_type == idt_array)) { //这个函数目前仅被用于数字和指针，数组，函数指针之间类型的类型转换
        cerr << "wrong use at get_cast_name !" << endl;
        exit(-1);
    }
    if (goal_type == idt_pointer && check_pointer(now_type))
        return strdup(now_name); //三种指针类型其实都是ptr的，不用转换
    if (now_type != goal_type) {
        char *tmp_name;
        tmp_name = get_TAC_name('t', CreateTempVar());
        gen_var(map_name[goal_type], tmp_name);
        gen_cast(tmp_name, now_name, map_name[goal_type]);
        return tmp_name;
    }
    else
        return strdup(now_name);
}

bool check_float(IdType_t now_type) //判断是否为浮点数
{
    if (now_type == idt_float || now_type == idt_double)
        return 1;
    else
        return 0;
}
bool check_pointer(IdType_t now_type) //判断是否为指针
{
    if (now_type == idt_pointer || now_type == idt_fpointer || now_type == idt_array)
        return 1;
    else
        return 0;
}
bool check_int(IdType_t now_type) //判断是否为整数
{
    if (now_type < 8)
        return 1;
    else
        return 0;
}
bool check_number(IdType_t now_type) //判断是否为数字
{
    if (now_type < 10)
        return 1;
    else
        return 0;
}
bool check_str_un(IdType_t now_type) //判断是否为struct/union
{
    if (now_type == idt_struct || now_type == idt_union)
        return 1;
    else
        return 0;
}

bool check_float(expression_s_t now)
{
    return check_float(now.type->type);
}
bool check_pointer(expression_s_t now)
{
    return check_pointer(now.type->type);
}
bool check_int(expression_s_t now)
{
    return check_int(now.type->type);
}
bool check_number(expression_s_t now)
{
    return check_number(now.type->type);
}
bool check_str_un(expression_s_t now)
{
    return check_str_un(now.type->type);
}

void declareParameter(const SymbolList_t *lst)
{
    std::vector<Identifier_t *> vid;
    for (auto i = lst; i; i = i->next)
        vid.push_back(i->id);
    std::reverse(vid.begin(), vid.end());
    for (size_t i = 0; i < vid.size(); ++i) {
        Identifier_t *id = vid[i];
        CreateParam(id);
        genDeclare(id->type, id->TACname, false);
        if (id->name == NULL)
            yyerror("parameter name omitted");
    }
}

expression_s_t get_c0_c1_exp(const char *name) //用来生成一个addr为c0或者c1的expression
{
    expression_s_t This;
    This.type = get_Typename_t(idt_int);
    This.lr_value = 0;
    This.isConst = 1;
    This.value.vint = 1;
    This.addr = strdup(name);
    This.laddr = NULL;
    return This;
}

void genIfGoto(expression_s_t expr, const char *name2, const char *op, int num) //name2必须为c0/c1
{
    expression_s_t cint = get_c0_c1_exp(name2), rel;
    get_relational_equality(rel, expr, cint, op); //写的略有些麻烦，先得到该逻辑表达式的值，然后判断是否为真
    gen_if_goto(rel.addr, "t1", "==", num); //???是否可以改进一下三地址码，允许直接对一个0或者1的变量 goto，不用非得表达式？
    /*
    #warning "need be changed to call expression functions"
    printf("(!!) if %s %s %s goto l%d\n", expr.get_addr(), op, name2, num);
    /*switch (expr.type->type) {
    case idt_struct:
    case idt_union:
        yyerror("struct/union condition error");
    }
    if (expr.type->type == idt_int)
        gen_if_goto(expr.addr, name2, op, num);
    else {
        int tnu = CreateTempVar();
        std::string stmp = 't' + std::to_string(tnu);
        gen_cast(stmp.c_str(), name2, "int4");
        gen_if_goto(stmp.c_str(), name2, op, num);
    }*/
}

const_Typename_ptr addConst(const_Typename_ptr type)
{
    if (type->type != idt_union && type->type != idt_struct && type->isConst)
        return type;
    Typename_t *ans = new Typename_t;
    *ans = *type;
    ans->isConst = 1;
    if (type->type == idt_union || type->type == idt_struct) {
        SymbolList_t *nlst = NULL, *tail = NULL;
        for (SymbolList_t *i = type->structure->record; i; i = i->next) {
            SymbolList_t *tmp = new SymbolList_t;
            tmp->offset = i->offset;
            tmp->id = memDup(i->id);
            tmp->id->type = addConst(i->id->type);
            tmp->next = NULL;
            if (nlst == NULL)
                nlst = tail = tmp;
            else {
                tail->next = tmp;
                tail = tmp;
            }
        }
        ans->structure->record = nlst;
    }
    return ans;
}
