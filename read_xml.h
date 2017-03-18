/*=====================================================================================
 * 1.LABEL_TREE数据结构
 * 2.XML_LIST数据结构
 * 3.全局静态变态
 * 4.操作函数
 *   (1) 结点创建函数
 *   (2) LABEL_TREE操作函数
 *   (3) XML_LIST操作函数
 *   (4) 内存池操作函数
 *   (5) xml文件读写操作函数
 * 5.使用流程
 *   (1) 读取xml，形成LABEL_TREE和XML_LIST。即调用ReadXml()函数
 *   (2) 通过使用xml文件读写操作函数，完成对xml内容的获取，修改等。如GetValue()，
 *       GetValues()，SetValue()等。
 *   (3) 若对xml的值进行了修改，则需要进行提交更改到新的文件中，即调用CommitToFile()，
 *       这里的处理原则是不改动原文件而是将一系列的改动后的新版本写入新文件。
 *   (4) 释放内存，关闭xml文件。即调用CloseXml()函数
 *   注：GetValueFromFile()函数可直接使用而不用进行(1)(4)操作。
 *=====================================================================================*/

#ifndef __READ_XML__
#define __READ_XML__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//定义使用系统-多选一
#define _WINDOWS_
//#define _LINUX_

typedef int bool;
#define false 0
#define true 1

#define SAFE_FREE(x) if(x!=NULL){free(x);x=NULL;}

#define RAM_POOL_SIZE (5*1024*1024)      //内存池大小-5M
#define ROW_BUFFER_SIZE (1024+2)         //行缓冲大小-1024个字符+换行符'\n'+'\0'
#define MAX_KEY_LEN 128                  //标签名不能超过127个字符
#define MAX_PROPERTY_LEN 64              //属性名不能超过63个字符
#define MAX_PROPERTY_VALUE_LEN 64        //属性值不能超过63个字符

//换行符
#ifdef _WINDOWS_
    #define BR ("\r\n")
#endif

#ifdef _LINUX_
    #define BR ("\n")
#endif

//旋转方向
typedef enum{LEFT,RIGHT}Roll_Direction;
//元素类型，分别表示 标签名，值，属性名，属性值
typedef enum{KEY=0x1,VALUE=0x2,PROPERTY=0x4,PROPERTY_VALUE=0x8}ELEMENT_TYPE;

/*==================== LABEL_TREE数据结构 ====================*/
//值结点
typedef struct ValueNode
{
    int size;               //值大小
    char* value;            //值在内存池中的地址
    struct ValueNode* last; //链接上一个结点，下同
    struct ValueNode* next; //链接下一个结点，下同
}ValueNode,*P_ValueNode;

//值链表
typedef struct ValueList
{
    P_ValueNode head;       //链表头，下同
    P_ValueNode tail;       //链表尾，下同
    int count;              //结点数，下同
}ValueList,*P_ValueList;

//标签属性结点
typedef struct PropertyNode
{
    char property[MAX_PROPERTY_LEN];     //属性
    char value[MAX_PROPERTY_VALUE_LEN];  //属性值
    struct PropertyNode* last;
    struct PropertyNode* next;
}PropertyNode,*P_PropertyNode;

//元素标签结点
typedef struct KeyNode
{
    char key[MAX_KEY_LEN];     //标签名
    P_ValueList values;        //标签值
    P_PropertyNode properties; //标签属性
    struct KeyNode* parent;    //链接父节点，下同
    struct KeyNode* lchild;    //链接左子节点，下同
    struct KeyNode* rchild;    //链接右子节点，下同
}KeyNode,*P_KeyNode;

//标签树
typedef struct
{
    P_KeyNode root;
    int count;
}KeyTree,*P_KeyTree;

/*==================== XML_LIST数据结构 ====================*/
typedef struct XML XML;
typedef struct XML *P_XML;
typedef struct
{
    P_XML head;
    P_XML tail;
    int count;
}XML_List,*P_XML_List;

//标签结点
struct XML
{
    char* key;                    //标签名，在DealOneRow开始标签中维护
    P_ValueNode value_node;       //标签值，在LabelTree_InsertValue中维护
    P_PropertyNode property_node; //标签属性，在LabelTree_InsertProperty中维护
    int property_num;             //标签属性个数，在LabelTree_InsertProperty中维护
    P_XML parent;
    P_XML last;
    P_XML next;
    P_XML_List child_list;        //子标签链表
};

/*==================== 全局静态变量 ====================*/
//xml头信息
static char version[4];
static char encoding[12];

//xml标签树
static P_KeyTree LABEL_TREE;
//xml结构链表
static P_XML XML_LIST;
//用于构造xml结构链表的指针，始终指向当前标签所需挂接的父标签
static P_XML parent_pointer = NULL;
//标识当前的标签值是否记录完毕
static bool value_finished = true;
//临时标签名存储Buffer
static char KEY_BUFFER[MAX_KEY_LEN];
//key计数器
static int key_counter;
//value计数器
static int value_counter;

//内存池结点
typedef struct
{
    char *addr;       //首地址
    int size;         //总大小
    char *read_addr;  //读地址，未用
    char *write_addr; //写地址
    int data_len;     //已用大小
}RAM_POOL,*P_RAM_POOL;
//内存池
static P_RAM_POOL POOL;

//xml文件句柄
static FILE* XML_FILE;
//行缓存
char ROW[ROW_BUFFER_SIZE];

/*==================== 操作函数 ====================*/
/*==创建结点函数==*/
static P_ValueNode CreateOneValueNode(void* addr,int size);
static P_PropertyNode CreateOnePropertyNode();
static P_KeyNode CreateOneKeyNode();
static P_XML CreateOneXMLNode();
static P_KeyTree CreateOneKeyTree();

/*LABEL_TREE操作函数*/
/*LABEL_TREE旋转和伸展函数*/
static void ST_Roll(P_KeyTree tree,P_KeyNode C,Roll_Direction d);
static bool ST_Splay(P_KeyTree tree,P_KeyNode N);

/*********************************************
 * 参数
 * key:标签名
 * 返回值：若搜索到指定key结点，则返回，否则返
 * 回该key该插入的位置的父节点。
 * 说明：在伸展树LABEL_TREE中搜索指定的key值结
 * 点。
 *********************************************/
static P_KeyNode LabelTree_Search(char* key);

/*********************************************
 * 参数
 * p:属性和属性值，入month="09"
 * xml_node:相应XML_LIST结点
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：在伸展树LABEL_TREE->root中插入属性p，
 * 并在相应xml_node中维护所需引用的该属性。
 *********************************************/
static bool LabelTree_InsertProperty(char* p,P_XML xml_node);

/*********************************************
 * 参数
 * value_addr:值地址
 * size:值大小
 * xml_node:相应XML_LIST结点
 * finished:值是否完成标识
 * 返回值：若成功，返回true，若失败，返回false。
 * 说明：在伸展树LABEL_TREE->root中插入值，并在
 * 相应xml_node中维护所需引用的该值。若finished
 * 为true，则表示上一个值已完成，当前插入的值为
 * 新值，若为false，则表示上一个值未记录完成，
 * 当前插入的值为上一个值的追加值。
 *********************************************/
static bool LabelTree_InsertValue(char* value_addr,int size,P_XML xml_node,bool finished);

/*********************************************
 * 参数
 * key:标签名
 * 返回值：若成功，返回true，若失败，返回false。
 * 说明:在伸展树LABEL_TREE->root中插入标签名key。
 *********************************************/
static bool LabelTree_InsertKey(char* key);

/*********************************************
 * 参数
 * root:LABEL_TREE根节点
 * 返回值：无
 * 说明:显示所有LABEL_TREE结点。
 *********************************************/
static void LabelTree_Show(P_KeyNode root);

/*********************************************
 * 参数
 * root:LABEL_TREE根节点
 * 返回值：无
 * 说明:释放所有LABEL_TREE结点。
 *********************************************/
static void LabelTree_Free(P_KeyNode root);

/*==XML_LIST操作函数==*/
/*********************************************
 * 参数
 * xml_node: XML_LIST结点
 * key:标签名
 * property:标签属性
 * property_value:标签属性值
 * 返回值：若成功，返回指定的标签结点，若失败，
 * 返回NULL
 * 说明：搜索指定条件的XML_LIST标签结点。搜索
 * 策略，key不能为NULL，当属性和属性值为NULL时
 * 为模糊搜索，即只返回第一个匹配key的结点；当
 * 指定属性和属性值时，为精确搜索，返回同时匹
 * 配key，属性，属性值的结点。模糊搜索或精确搜
 * 索不成功时，返回NULL。
 *********************************************/
static P_XML XML_Search(P_XML xml_node,char* key,char* property,char* property_value);

/*********************************************
 * 参数
 * head: XML_LIST头结点
 * s:流
 * 返回值：无
 * 说明：打印XML_LIST中所有结点到指定的流。
 *********************************************/
static void XML_Print(P_XML head,FILE* s);

/*********************************************
 * 参数
 * head: XML_LIST头结点
 * 返回值：无
 * 说明：释放所有XML_LIST结点
 *********************************************/
static void XML_Free(P_XML head);

/*==内存池操作函数==*/
/*********************************************
 * 参数
 * size: 内存池大小
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：创建指定大小的内存池。
 *********************************************/
static bool CreateRAM_Pool(unsigned int size);

/*********************************************
 * 参数：无
 * 返回值：内存池写地址
 * 说明：获取内存池的写地址。
 *********************************************/
static char* Pool_GetWritePosition();

/*********************************************
 * 参数
 * v: 值
 * is_add: 追加标识
 * 返回值：写入字节数
 * 说明：向内存池最新可写地址处写入值v，若is_add
 * 为true，则标识当前插入的值为追加的，若为false
 * 则标识当前插入的值为新值。
 *********************************************/
static int Pool_WriteData(char* v,bool is_add);

/*********************************************
 * 参数：无
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：释放内存池
 *********************************************/
static void FreeRAM_Pool();

/*==xml文件读写操作函数==*/
/*********************************************
 * 参数
 * row：读取xml文件一行内容的行缓存
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：处理读取的xml文件的一行内容。不做xml
 * 内容合法性的验证，按字符读取并处理。辨别标
 * 签与值的根据为'<'和非'<'。辨别不同类型标签
 * 的根据为'<'字符后的第一个可见字符。
 *********************************************/
static bool DealOneRow(char* row);

/*********************************************
 * 参数
 * file：xml文件路径
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：负责创建LABEL_TREE,内存池和xml文件句柄，
 * 逐行读取并处理xml文件。文件的打开模式为r+。
 *********************************************/
bool ReadXml(char* file);

/*********************************************
 * 参数
 * key：标签名
 * value：值buffer
 * property：标签属性
 * property_value：标签属性值
 * value_type：指定要获取的元素类型，VALUE或PR-
 * OPERTY_VALUE。
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：从LABEL_TREE或XML_LIST中获取一个指定条
 * 件的值或属性值，相应存入value或property_value。
 * (1)当value_type=VALUE，有两种模式：①若指定key
 * 而未指定property和property_value，则为模糊搜
 * 索，将从LABEL_TREE中搜索并将匹配key的结点的第
 * 一个值写入value，速度较快，适合搜索标签名具有
 * 唯一性的值。②同时指定key，property和propert-
 * y_value，为精确搜索，将从XML_LIST中搜索并将匹
 * 配key，peroperty和property_value的结点的值写
 * 入value。速度较慢，适合搜索标签名不具有唯一性
 * 但需要获取其中指定属性和属性值的标签的值。
 * (2)value_type=PROPERTY_VALUE，直接搜索LABEL_TR-
 * EE并将匹配key的结点的第一个匹配property的属性
 * 值写入property_value。
 *********************************************/
bool GetValue(char* key,char* value,char* property,char* property_value,ELEMENT_TYPE value_type);

/*********************************************
 * 参数
 * key：标签名
 * 返回值：若成功，返回系列值的头结点，若失败，
 * 返回NULL。
 * 说明：从LABEL_TREE中获取指定key的结点下的所有
 * 值。适合要获取xml中存在一系列同名同类型同用途
 * 标签的值的情况。
 *********************************************/
P_ValueNode GetValues(char* key);

/*********************************************
 * 参数
 * key：标签名
 * property：标签属性
 * property_value：标签属性值
 * 返回值：若成功，返回指定结点，若失败，返回
 * NULL。
 * 说明：从XML_LIST中获取指定条件的结点。当指定
 * key而未指定property和property_value时为模糊
 * 搜索，默认返回第一个匹配key的结点，若同时指
 * 定三个参数，则为精确搜索，返回同时匹配三个
 * 参数的结点。
 *********************************************/
P_XML GetXmlNode(char* key,char* property,char* property_value);

/*********************************************
 * 参数
 * pk：父标签名
 * pp：父标签属性
 * ppv：父标签属性值
 * 返回值：若成功，返回指定父结点的子结点链表
 * 的头结点，若失败或无子结点，返回NULL。
 * 说明：从XML_LIST中获取指定条件的结点。当指定
 * pk而未指定pp和ppv时为模糊搜索，默认返回第一
 * 个匹配pk的结点的子结点链表的头结点，若同时指
 * 定三个参数，则为精确搜索，返回同时匹配三个参
 * 数的结点的子结点链表的头结点。
 *********************************************/
P_XML GetChilds(char* pk,char* pp,char* ppv);

/*********************************************
 * 参数
 * xf：xml文件路径
 * k：标签名
 * v：值Buffer
 * p：标签属性
 * pv：标签属性值
 * vt：要获取元素值类型，VALUE或PROPERTY_VALUE。
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：直接从指定的xf文件中获取指定条件下的
 * 值或属性值并对应写入v或pv中。处理模式与Get-
 * Value()函数一致。该函数适合非频繁获取，只是
 * 偶尔从xml中获取少数值的情况，不需创建LABEL-
 * _TREE和XML_LIST，可单独使用。该获取的值不处
 * 理xml预设的五种引用
 *********************************************/
bool GetValueFromFile(char* xf,char* k,char* v,char* p,char* pv,ELEMENT_TYPE vt);

/*********************************************
 * 参数
 * v：xml头信息中的版本值
 * e：xml头信息中的字符集值
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：设置xml文件的头信息中的版本为v，字符集
 * 为e。
 *********************************************/
bool SetXmlHead(char* v,char* e);

/*********************************************
 * 参数
 * k：标签名
 * p：标签属性
 * pv：标签属性值
 * nv：要设置的新的值、属性或属性值
 * vt：要设置的元素类型，VALUE，ROPERTY或PROPE-
 * RTY_VALUE。
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：在LABEL_TREE和XML_LIST中设置指定条件下
 * 的结点的值、属性或属性值为nv。指定条件为k，
 * p，pv，搜索策略同上，分为模糊搜索和精确搜索。
 * 其中模糊搜索只能修改值。
 *********************************************/
bool SetValue(char* k,char* p,char* pv,char* nv,ELEMENT_TYPE vt);

/*********************************************
 * 参数
 * pk：父标签名
 * pp：父标签属性
 * ppv：父标签属性值
 * nk：要添加新的标签的标签名
 * nv：要添加新的标签的值
 * ...:要添加新的标签的一系列属性，最后一个参数
 * 必须为元素类型。
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：在LABEL_TREE和XML_LIST中pk，pp，ppv下
 * 增加一个标签名为nk，标签值为nv，属性值为...的
 * 结点。其中变长参数中的一系列属性、属性值，应
 * 成对依次出现，重复的属性将被省略。
 * 最后一个元素类型可为KEY，KEY|PROPERTY|PROPE-
 * RTY_VALUE，KEY|PROPERTY|PROPERTY_VALUE|VALUE，
 * PROPERTY|PROPERTY_VALUE，VALUE。表示增加相应
 * 的元素。其中为后两者时表示在指定的pk，pp，ppv
 * 条件下增加属性或值。对pk，pp，ppv的条件搜索
 * 策略同上。
 *********************************************/
bool AddOne(char* pk,char* pp,char* ppv,char* nk,char* nv,...);

/*********************************************
 * 参数
 * k：标签名
 * p：标签属性
 * pv：标签属性值
 * vt：要删除的元素类型，KEY，PROPERTY或VALUE
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：在LABEL_TREE和XML_LIST中删除指定条件下
 * 的元素。指定条件为k，p，pv，搜索策略同上。
 * 若删除的标签A及A标签的所有子标签A_childs与
 * xml中其他标签B有重名情况，需保证A、A_childs
 * 与B能够通过第一个属性和属性值保持各自唯一，
 * 否则将误删B中第一个匹配A或A_childs标签名的
 * 标签。
 *********************************************/
bool DelOne(char* k,char* p,char* pv,ELEMENT_TYPE vt);

/*********************************************
 * 参数
 * pk：父标签名
 * pp：父标签属性
 * ppv：父标签属性值
 * label：要添加新的标签字符串
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：在LABEL_TREE和XML_LIST中pk，pp，ppv下
 * 增加一个标签名label，label为一个可带值，任意
 * 个属性的完整的标签字符串。pk，pp，ppv的搜索
 * 策略同上。
 *********************************************/
bool AddOneLabel(char* pk,char* pp,char* ppv,char* label);

/*********************************************
 * 参数
 * file：文件路径
 * 返回值：若成功，返回true，若失败，返回false
 * 说明：提交对LABEL_TREE和XML_LIST中的信息的改
 * 变，将修改后的xml内容打印到file文件中。
 *********************************************/
bool CommitToFile(char* file);

/*********************************************
 * 参数：无
 * 返回值：无
 * 说明：负责释放所有手工分配的有关LABEL_TREE，
 * XML_LIST的内存，关闭xml文件。
 *********************************************/
void CloseXml();


#endif



