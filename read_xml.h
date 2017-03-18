/*=====================================================================================
 * 1.LABEL_TREE���ݽṹ
 * 2.XML_LIST���ݽṹ
 * 3.ȫ�־�̬��̬
 * 4.��������
 *   (1) ��㴴������
 *   (2) LABEL_TREE��������
 *   (3) XML_LIST��������
 *   (4) �ڴ�ز�������
 *   (5) xml�ļ���д��������
 * 5.ʹ������
 *   (1) ��ȡxml���γ�LABEL_TREE��XML_LIST��������ReadXml()����
 *   (2) ͨ��ʹ��xml�ļ���д������������ɶ�xml���ݵĻ�ȡ���޸ĵȡ���GetValue()��
 *       GetValues()��SetValue()�ȡ�
 *   (3) ����xml��ֵ�������޸ģ�����Ҫ�����ύ���ĵ��µ��ļ��У�������CommitToFile()��
 *       ����Ĵ���ԭ���ǲ��Ķ�ԭ�ļ����ǽ�һϵ�еĸĶ�����°汾д�����ļ���
 *   (4) �ͷ��ڴ棬�ر�xml�ļ���������CloseXml()����
 *   ע��GetValueFromFile()������ֱ��ʹ�ö����ý���(1)(4)������
 *=====================================================================================*/

#ifndef __READ_XML__
#define __READ_XML__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

//����ʹ��ϵͳ-��ѡһ
#define _WINDOWS_
//#define _LINUX_

typedef int bool;
#define false 0
#define true 1

#define SAFE_FREE(x) if(x!=NULL){free(x);x=NULL;}

#define RAM_POOL_SIZE (5*1024*1024)      //�ڴ�ش�С-5M
#define ROW_BUFFER_SIZE (1024+2)         //�л����С-1024���ַ�+���з�'\n'+'\0'
#define MAX_KEY_LEN 128                  //��ǩ�����ܳ���127���ַ�
#define MAX_PROPERTY_LEN 64              //���������ܳ���63���ַ�
#define MAX_PROPERTY_VALUE_LEN 64        //����ֵ���ܳ���63���ַ�

//���з�
#ifdef _WINDOWS_
    #define BR ("\r\n")
#endif

#ifdef _LINUX_
    #define BR ("\n")
#endif

//��ת����
typedef enum{LEFT,RIGHT}Roll_Direction;
//Ԫ�����ͣ��ֱ��ʾ ��ǩ����ֵ��������������ֵ
typedef enum{KEY=0x1,VALUE=0x2,PROPERTY=0x4,PROPERTY_VALUE=0x8}ELEMENT_TYPE;

/*==================== LABEL_TREE���ݽṹ ====================*/
//ֵ���
typedef struct ValueNode
{
    int size;               //ֵ��С
    char* value;            //ֵ���ڴ���еĵ�ַ
    struct ValueNode* last; //������һ����㣬��ͬ
    struct ValueNode* next; //������һ����㣬��ͬ
}ValueNode,*P_ValueNode;

//ֵ����
typedef struct ValueList
{
    P_ValueNode head;       //����ͷ����ͬ
    P_ValueNode tail;       //����β����ͬ
    int count;              //���������ͬ
}ValueList,*P_ValueList;

//��ǩ���Խ��
typedef struct PropertyNode
{
    char property[MAX_PROPERTY_LEN];     //����
    char value[MAX_PROPERTY_VALUE_LEN];  //����ֵ
    struct PropertyNode* last;
    struct PropertyNode* next;
}PropertyNode,*P_PropertyNode;

//Ԫ�ر�ǩ���
typedef struct KeyNode
{
    char key[MAX_KEY_LEN];     //��ǩ��
    P_ValueList values;        //��ǩֵ
    P_PropertyNode properties; //��ǩ����
    struct KeyNode* parent;    //���Ӹ��ڵ㣬��ͬ
    struct KeyNode* lchild;    //�������ӽڵ㣬��ͬ
    struct KeyNode* rchild;    //�������ӽڵ㣬��ͬ
}KeyNode,*P_KeyNode;

//��ǩ��
typedef struct
{
    P_KeyNode root;
    int count;
}KeyTree,*P_KeyTree;

/*==================== XML_LIST���ݽṹ ====================*/
typedef struct XML XML;
typedef struct XML *P_XML;
typedef struct
{
    P_XML head;
    P_XML tail;
    int count;
}XML_List,*P_XML_List;

//��ǩ���
struct XML
{
    char* key;                    //��ǩ������DealOneRow��ʼ��ǩ��ά��
    P_ValueNode value_node;       //��ǩֵ����LabelTree_InsertValue��ά��
    P_PropertyNode property_node; //��ǩ���ԣ���LabelTree_InsertProperty��ά��
    int property_num;             //��ǩ���Ը�������LabelTree_InsertProperty��ά��
    P_XML parent;
    P_XML last;
    P_XML next;
    P_XML_List child_list;        //�ӱ�ǩ����
};

/*==================== ȫ�־�̬���� ====================*/
//xmlͷ��Ϣ
static char version[4];
static char encoding[12];

//xml��ǩ��
static P_KeyTree LABEL_TREE;
//xml�ṹ����
static P_XML XML_LIST;
//���ڹ���xml�ṹ�����ָ�룬ʼ��ָ��ǰ��ǩ����ҽӵĸ���ǩ
static P_XML parent_pointer = NULL;
//��ʶ��ǰ�ı�ǩֵ�Ƿ��¼���
static bool value_finished = true;
//��ʱ��ǩ���洢Buffer
static char KEY_BUFFER[MAX_KEY_LEN];
//key������
static int key_counter;
//value������
static int value_counter;

//�ڴ�ؽ��
typedef struct
{
    char *addr;       //�׵�ַ
    int size;         //�ܴ�С
    char *read_addr;  //����ַ��δ��
    char *write_addr; //д��ַ
    int data_len;     //���ô�С
}RAM_POOL,*P_RAM_POOL;
//�ڴ��
static P_RAM_POOL POOL;

//xml�ļ����
static FILE* XML_FILE;
//�л���
char ROW[ROW_BUFFER_SIZE];

/*==================== �������� ====================*/
/*==������㺯��==*/
static P_ValueNode CreateOneValueNode(void* addr,int size);
static P_PropertyNode CreateOnePropertyNode();
static P_KeyNode CreateOneKeyNode();
static P_XML CreateOneXMLNode();
static P_KeyTree CreateOneKeyTree();

/*LABEL_TREE��������*/
/*LABEL_TREE��ת����չ����*/
static void ST_Roll(P_KeyTree tree,P_KeyNode C,Roll_Direction d);
static bool ST_Splay(P_KeyTree tree,P_KeyNode N);

/*********************************************
 * ����
 * key:��ǩ��
 * ����ֵ����������ָ��key��㣬�򷵻أ�����
 * �ظ�key�ò����λ�õĸ��ڵ㡣
 * ˵��������չ��LABEL_TREE������ָ����keyֵ��
 * �㡣
 *********************************************/
static P_KeyNode LabelTree_Search(char* key);

/*********************************************
 * ����
 * p:���Ժ�����ֵ����month="09"
 * xml_node:��ӦXML_LIST���
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵��������չ��LABEL_TREE->root�в�������p��
 * ������Ӧxml_node��ά���������õĸ����ԡ�
 *********************************************/
static bool LabelTree_InsertProperty(char* p,P_XML xml_node);

/*********************************************
 * ����
 * value_addr:ֵ��ַ
 * size:ֵ��С
 * xml_node:��ӦXML_LIST���
 * finished:ֵ�Ƿ���ɱ�ʶ
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false��
 * ˵��������չ��LABEL_TREE->root�в���ֵ������
 * ��Ӧxml_node��ά���������õĸ�ֵ����finished
 * Ϊtrue�����ʾ��һ��ֵ����ɣ���ǰ�����ֵΪ
 * ��ֵ����Ϊfalse�����ʾ��һ��ֵδ��¼��ɣ�
 * ��ǰ�����ֵΪ��һ��ֵ��׷��ֵ��
 *********************************************/
static bool LabelTree_InsertValue(char* value_addr,int size,P_XML xml_node,bool finished);

/*********************************************
 * ����
 * key:��ǩ��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false��
 * ˵��:����չ��LABEL_TREE->root�в����ǩ��key��
 *********************************************/
static bool LabelTree_InsertKey(char* key);

/*********************************************
 * ����
 * root:LABEL_TREE���ڵ�
 * ����ֵ����
 * ˵��:��ʾ����LABEL_TREE��㡣
 *********************************************/
static void LabelTree_Show(P_KeyNode root);

/*********************************************
 * ����
 * root:LABEL_TREE���ڵ�
 * ����ֵ����
 * ˵��:�ͷ�����LABEL_TREE��㡣
 *********************************************/
static void LabelTree_Free(P_KeyNode root);

/*==XML_LIST��������==*/
/*********************************************
 * ����
 * xml_node: XML_LIST���
 * key:��ǩ��
 * property:��ǩ����
 * property_value:��ǩ����ֵ
 * ����ֵ�����ɹ�������ָ���ı�ǩ��㣬��ʧ�ܣ�
 * ����NULL
 * ˵��������ָ��������XML_LIST��ǩ��㡣����
 * ���ԣ�key����ΪNULL�������Ժ�����ֵΪNULLʱ
 * Ϊģ����������ֻ���ص�һ��ƥ��key�Ľ�㣻��
 * ָ�����Ժ�����ֵʱ��Ϊ��ȷ����������ͬʱƥ
 * ��key�����ԣ�����ֵ�Ľ�㡣ģ��������ȷ��
 * �����ɹ�ʱ������NULL��
 *********************************************/
static P_XML XML_Search(P_XML xml_node,char* key,char* property,char* property_value);

/*********************************************
 * ����
 * head: XML_LISTͷ���
 * s:��
 * ����ֵ����
 * ˵������ӡXML_LIST�����н�㵽ָ��������
 *********************************************/
static void XML_Print(P_XML head,FILE* s);

/*********************************************
 * ����
 * head: XML_LISTͷ���
 * ����ֵ����
 * ˵�����ͷ�����XML_LIST���
 *********************************************/
static void XML_Free(P_XML head);

/*==�ڴ�ز�������==*/
/*********************************************
 * ����
 * size: �ڴ�ش�С
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵��������ָ����С���ڴ�ء�
 *********************************************/
static bool CreateRAM_Pool(unsigned int size);

/*********************************************
 * ��������
 * ����ֵ���ڴ��д��ַ
 * ˵������ȡ�ڴ�ص�д��ַ��
 *********************************************/
static char* Pool_GetWritePosition();

/*********************************************
 * ����
 * v: ֵ
 * is_add: ׷�ӱ�ʶ
 * ����ֵ��д���ֽ���
 * ˵�������ڴ�����¿�д��ַ��д��ֵv����is_add
 * Ϊtrue�����ʶ��ǰ�����ֵΪ׷�ӵģ���Ϊfalse
 * ���ʶ��ǰ�����ֵΪ��ֵ��
 *********************************************/
static int Pool_WriteData(char* v,bool is_add);

/*********************************************
 * ��������
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵�����ͷ��ڴ��
 *********************************************/
static void FreeRAM_Pool();

/*==xml�ļ���д��������==*/
/*********************************************
 * ����
 * row����ȡxml�ļ�һ�����ݵ��л���
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵���������ȡ��xml�ļ���һ�����ݡ�����xml
 * ���ݺϷ��Ե���֤�����ַ���ȡ����������
 * ǩ��ֵ�ĸ���Ϊ'<'�ͷ�'<'�����ͬ���ͱ�ǩ
 * �ĸ���Ϊ'<'�ַ���ĵ�һ���ɼ��ַ���
 *********************************************/
static bool DealOneRow(char* row);

/*********************************************
 * ����
 * file��xml�ļ�·��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵�������𴴽�LABEL_TREE,�ڴ�غ�xml�ļ������
 * ���ж�ȡ������xml�ļ����ļ��Ĵ�ģʽΪr+��
 *********************************************/
bool ReadXml(char* file);

/*********************************************
 * ����
 * key����ǩ��
 * value��ֵbuffer
 * property����ǩ����
 * property_value����ǩ����ֵ
 * value_type��ָ��Ҫ��ȡ��Ԫ�����ͣ�VALUE��PR-
 * OPERTY_VALUE��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵������LABEL_TREE��XML_LIST�л�ȡһ��ָ����
 * ����ֵ������ֵ����Ӧ����value��property_value��
 * (1)��value_type=VALUE��������ģʽ������ָ��key
 * ��δָ��property��property_value����Ϊģ����
 * ��������LABEL_TREE����������ƥ��key�Ľ��ĵ�
 * һ��ֵд��value���ٶȽϿ죬�ʺ�������ǩ������
 * Ψһ�Ե�ֵ����ͬʱָ��key��property��propert-
 * y_value��Ϊ��ȷ����������XML_LIST����������ƥ
 * ��key��peroperty��property_value�Ľ���ֵд
 * ��value���ٶȽ������ʺ�������ǩ��������Ψһ��
 * ����Ҫ��ȡ����ָ�����Ժ�����ֵ�ı�ǩ��ֵ��
 * (2)value_type=PROPERTY_VALUE��ֱ������LABEL_TR-
 * EE����ƥ��key�Ľ��ĵ�һ��ƥ��property������
 * ֵд��property_value��
 *********************************************/
bool GetValue(char* key,char* value,char* property,char* property_value,ELEMENT_TYPE value_type);

/*********************************************
 * ����
 * key����ǩ��
 * ����ֵ�����ɹ�������ϵ��ֵ��ͷ��㣬��ʧ�ܣ�
 * ����NULL��
 * ˵������LABEL_TREE�л�ȡָ��key�Ľ���µ�����
 * ֵ���ʺ�Ҫ��ȡxml�д���һϵ��ͬ��ͬ����ͬ��;
 * ��ǩ��ֵ�������
 *********************************************/
P_ValueNode GetValues(char* key);

/*********************************************
 * ����
 * key����ǩ��
 * property����ǩ����
 * property_value����ǩ����ֵ
 * ����ֵ�����ɹ�������ָ����㣬��ʧ�ܣ�����
 * NULL��
 * ˵������XML_LIST�л�ȡָ�������Ľ�㡣��ָ��
 * key��δָ��property��property_valueʱΪģ��
 * ������Ĭ�Ϸ��ص�һ��ƥ��key�Ľ�㣬��ͬʱָ
 * ��������������Ϊ��ȷ����������ͬʱƥ������
 * �����Ľ�㡣
 *********************************************/
P_XML GetXmlNode(char* key,char* property,char* property_value);

/*********************************************
 * ����
 * pk������ǩ��
 * pp������ǩ����
 * ppv������ǩ����ֵ
 * ����ֵ�����ɹ�������ָ���������ӽ������
 * ��ͷ��㣬��ʧ�ܻ����ӽ�㣬����NULL��
 * ˵������XML_LIST�л�ȡָ�������Ľ�㡣��ָ��
 * pk��δָ��pp��ppvʱΪģ��������Ĭ�Ϸ��ص�һ
 * ��ƥ��pk�Ľ����ӽ�������ͷ��㣬��ͬʱָ
 * ��������������Ϊ��ȷ����������ͬʱƥ��������
 * ���Ľ����ӽ�������ͷ��㡣
 *********************************************/
P_XML GetChilds(char* pk,char* pp,char* ppv);

/*********************************************
 * ����
 * xf��xml�ļ�·��
 * k����ǩ��
 * v��ֵBuffer
 * p����ǩ����
 * pv����ǩ����ֵ
 * vt��Ҫ��ȡԪ��ֵ���ͣ�VALUE��PROPERTY_VALUE��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵����ֱ�Ӵ�ָ����xf�ļ��л�ȡָ�������µ�
 * ֵ������ֵ����Ӧд��v��pv�С�����ģʽ��Get-
 * Value()����һ�¡��ú����ʺϷ�Ƶ����ȡ��ֻ��
 * ż����xml�л�ȡ����ֵ����������贴��LABEL-
 * _TREE��XML_LIST���ɵ���ʹ�á��û�ȡ��ֵ����
 * ��xmlԤ�����������
 *********************************************/
bool GetValueFromFile(char* xf,char* k,char* v,char* p,char* pv,ELEMENT_TYPE vt);

/*********************************************
 * ����
 * v��xmlͷ��Ϣ�еİ汾ֵ
 * e��xmlͷ��Ϣ�е��ַ���ֵ
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵��������xml�ļ���ͷ��Ϣ�еİ汾Ϊv���ַ���
 * Ϊe��
 *********************************************/
bool SetXmlHead(char* v,char* e);

/*********************************************
 * ����
 * k����ǩ��
 * p����ǩ����
 * pv����ǩ����ֵ
 * nv��Ҫ���õ��µ�ֵ�����Ի�����ֵ
 * vt��Ҫ���õ�Ԫ�����ͣ�VALUE��ROPERTY��PROPE-
 * RTY_VALUE��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵������LABEL_TREE��XML_LIST������ָ��������
 * �Ľ���ֵ�����Ի�����ֵΪnv��ָ������Ϊk��
 * p��pv����������ͬ�ϣ���Ϊģ�������;�ȷ������
 * ����ģ������ֻ���޸�ֵ��
 *********************************************/
bool SetValue(char* k,char* p,char* pv,char* nv,ELEMENT_TYPE vt);

/*********************************************
 * ����
 * pk������ǩ��
 * pp������ǩ����
 * ppv������ǩ����ֵ
 * nk��Ҫ����µı�ǩ�ı�ǩ��
 * nv��Ҫ����µı�ǩ��ֵ
 * ...:Ҫ����µı�ǩ��һϵ�����ԣ����һ������
 * ����ΪԪ�����͡�
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵������LABEL_TREE��XML_LIST��pk��pp��ppv��
 * ����һ����ǩ��Ϊnk����ǩֵΪnv������ֵΪ...��
 * ��㡣���б䳤�����е�һϵ�����ԡ�����ֵ��Ӧ
 * �ɶ����γ��֣��ظ������Խ���ʡ�ԡ�
 * ���һ��Ԫ�����Ϳ�ΪKEY��KEY|PROPERTY|PROPE-
 * RTY_VALUE��KEY|PROPERTY|PROPERTY_VALUE|VALUE��
 * PROPERTY|PROPERTY_VALUE��VALUE����ʾ������Ӧ
 * ��Ԫ�ء�����Ϊ������ʱ��ʾ��ָ����pk��pp��ppv
 * �������������Ի�ֵ����pk��pp��ppv����������
 * ����ͬ�ϡ�
 *********************************************/
bool AddOne(char* pk,char* pp,char* ppv,char* nk,char* nv,...);

/*********************************************
 * ����
 * k����ǩ��
 * p����ǩ����
 * pv����ǩ����ֵ
 * vt��Ҫɾ����Ԫ�����ͣ�KEY��PROPERTY��VALUE
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵������LABEL_TREE��XML_LIST��ɾ��ָ��������
 * ��Ԫ�ء�ָ������Ϊk��p��pv����������ͬ�ϡ�
 * ��ɾ���ı�ǩA��A��ǩ�������ӱ�ǩA_childs��
 * xml��������ǩB������������豣֤A��A_childs
 * ��B�ܹ�ͨ����һ�����Ժ�����ֵ���ָ���Ψһ��
 * ������ɾB�е�һ��ƥ��A��A_childs��ǩ����
 * ��ǩ��
 *********************************************/
bool DelOne(char* k,char* p,char* pv,ELEMENT_TYPE vt);

/*********************************************
 * ����
 * pk������ǩ��
 * pp������ǩ����
 * ppv������ǩ����ֵ
 * label��Ҫ����µı�ǩ�ַ���
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵������LABEL_TREE��XML_LIST��pk��pp��ppv��
 * ����һ����ǩ��label��labelΪһ���ɴ�ֵ������
 * �����Ե������ı�ǩ�ַ�����pk��pp��ppv������
 * ����ͬ�ϡ�
 *********************************************/
bool AddOneLabel(char* pk,char* pp,char* ppv,char* label);

/*********************************************
 * ����
 * file���ļ�·��
 * ����ֵ�����ɹ�������true����ʧ�ܣ�����false
 * ˵�����ύ��LABEL_TREE��XML_LIST�е���Ϣ�ĸ�
 * �䣬���޸ĺ��xml���ݴ�ӡ��file�ļ��С�
 *********************************************/
bool CommitToFile(char* file);

/*********************************************
 * ��������
 * ����ֵ����
 * ˵���������ͷ������ֹ�������й�LABEL_TREE��
 * XML_LIST���ڴ棬�ر�xml�ļ���
 *********************************************/
void CloseXml();


#endif



