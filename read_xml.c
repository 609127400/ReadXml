#include "read_xml.h"

static P_ValueNode CreateOneValueNode(void* addr,int size)
{
    P_ValueNode value = (P_ValueNode)malloc(sizeof(ValueNode));
    if(value == NULL){ printf("malloc value node failed\n"); return NULL; }
    value->size = size;
    value->value = addr;
    value->last = NULL;
    value->next = NULL;
    return value;
}

static P_PropertyNode CreateOnePropertyNode()
{
    P_PropertyNode property = (P_PropertyNode)malloc(sizeof(PropertyNode));
    if(property == NULL){ printf("malloc property failed\n"); return NULL; }
    property->last = NULL;
    property->next = NULL;
    return property;
}

static P_KeyNode CreateOneKeyNode()
{
    P_KeyNode key = (P_KeyNode)malloc(sizeof(KeyNode));
    if(key == NULL){ printf("malloc key node failed\n"); return NULL; }
    key->properties = NULL;
    key->parent = NULL;
    key->lchild = NULL;
    key->rchild = NULL;

    key->values = (P_ValueList)malloc(sizeof(ValueList));
    if(key->values == NULL)
    { free(key); printf("malloc value list faile"); return NULL; }
    key->values->head = NULL;
    key->values->tail = NULL;
    key->values->count = 0;
    
    return key;
}

static P_XML CreateOneXMLNode()
{
    P_XML xml = (P_XML)malloc(sizeof(XML));
    if(xml == NULL){ printf("malloc XML failed\n"); return NULL; }
    P_XML_List child_list = (P_XML_List)malloc(sizeof(XML_List));
    if(child_list == NULL){ SAFE_FREE(xml); printf("malloc XML_List failed\n"); return false; }
    child_list->head = NULL;
    child_list->tail = NULL;
    child_list->count = 0;
    
    xml->key = NULL;
    xml->value_node = NULL;
    xml->property_node = NULL;
    xml->property_num = 0;
    xml->parent = NULL;
    xml->last = NULL;
    xml->next = NULL;
    xml->child_list = child_list;

    return xml;
}

static P_KeyTree CreateOneKeyTree()
{
    P_KeyTree tree = (P_KeyTree)malloc(sizeof(KeyTree));
    if(tree == NULL){ printf("malloc key tree failed\n"); return NULL; }
    tree->root = NULL;
    tree->count = 0;
    return tree;
}

static P_XML XML_Search(P_XML xml_node,char* key,char* property,char* property_value)
{
    if(xml_node == NULL){ return NULL; }
    if(strcmp(key,xml_node->key) == 0)
    {
	if(property != NULL && property_value != NULL)
	{
	    P_PropertyNode pn = xml_node->property_node;
	    int i = 0;
	    while(i < xml_node->property_num)
	    {
		if(strcmp(property,pn->property) == 0 && strcmp(property_value,pn->value) == 0)
		{ 
		    return xml_node;
		}
		pn = pn->next;
		++i;
	    }
	    return NULL;
	}
	else
	{
	    if(xml_node->property_node == NULL){ return xml_node; }
	    else{ return NULL; }
	}
    }
    P_XML ret_xml_node = NULL;
    P_XML child = xml_node->child_list->head;
    while(child != NULL)
    {
	ret_xml_node = XML_Search(child,key,property,property_value);
	if(ret_xml_node != NULL){ return ret_xml_node; }
	child = child->next;
    }
    return NULL;
}

static void ST_Roll(P_KeyTree tree,P_KeyNode C,Roll_Direction d)
{
    if(C->parent == NULL) return;
    P_KeyNode G = C->parent->parent;
    P_KeyNode P = C->parent;
    P_KeyNode S = (C == P->lchild) ? P->rchild : P->lchild;
    if(G == NULL)
    {
	tree->root = C;
    }
    else
    {
	if(P == G->lchild){ G->lchild = C; }else{ G->rchild = C; }
    }
    C->parent = G;
    if(d == LEFT)
    {
	P->rchild = C->lchild;
	if(C->lchild != NULL) C->lchild->parent = P;
	C->lchild = P;
	P->parent = C;
    }
    else if(d == RIGHT)
    {
	P->lchild = C->rchild;
	if(C->rchild != NULL) C->rchild->parent = P;
	C->rchild = P;
	P->parent = C;
    }
    else
    {
	printf("Roll Direction is wrong\n");
    }
}

static bool ST_Splay(P_KeyTree tree,P_KeyNode N)
{
    if(tree == NULL || tree->root == NULL)
    {
	printf("Splay Tree is NULL\n");
	return false;
    }
    if(N == NULL)
    {
	printf("N is NULL\n");
	return false;
    }
    //根节点不做处理，直接返回
    if(N->parent == NULL){ return true; }
    P_KeyNode G = NULL;
    P_KeyNode P = NULL;

    while(N->parent != NULL)
    {
	//旋转之后相当于循环条件更新
	G = N->parent->parent;
	P = N->parent;
	if(G == NULL)
	{
	    //简单的单旋转
	    if(N == P->lchild)
	    {
	       //zig
	       ST_Roll(tree,N,RIGHT);
	    }
	    else
	    {
		//zag
		ST_Roll(tree,N,LEFT);
	    }
	}
	else if(N == P->lchild && P == G->lchild)
	{
	    //双层伸展-R
	    ST_Roll(tree,P,RIGHT);
	    ST_Roll(tree,N,RIGHT);
	}
	else if(N == P->rchild && P == G->rchild)
	{
	    //双层伸展-L
	    ST_Roll(tree,P,LEFT);
	    ST_Roll(tree,N,LEFT);
	}
	else if(N == P->lchild && P == G->rchild)
	{
	    //zig-zag
	    ST_Roll(tree,N,RIGHT);
	    ST_Roll(tree,N,LEFT);
	}
	else if(N == P->rchild && P == G->lchild)
	{
	    //zag-zig
	    ST_Roll(tree,N,LEFT);
	    ST_Roll(tree,N,RIGHT);
	}
    }
    return true;
}

static P_KeyNode LabelTree_Search(char* key)
{
    if(LABEL_TREE == NULL || LABEL_TREE->root == NULL)
    { printf("LabelTree_Search LABEL_TREE is NULL\n"); return NULL; }
    P_KeyNode root = LABEL_TREE->root;
    P_KeyNode temp_parent = NULL;
    int result;
    while(root != NULL)
    {
	temp_parent = root;
	result = strcmp(key,root->key);
	if(result < 0)
	{ root = root->lchild; }
	else if(result > 0)
	{ root = root->rchild; }
	else if(result == 0)
	{
	    ST_Splay(LABEL_TREE,root);
	    return root;
	}
    }
    return temp_parent;
}

static bool LabelTree_InsertProperty(char* p,P_XML xml_node)
{
    if(p == NULL){ return false; }
    int i = 0;
    if(LABEL_TREE->root->properties == NULL)
    {
	LABEL_TREE->root->properties = CreateOnePropertyNode();
	if(LABEL_TREE->root->properties == NULL){ return false; }
	i = 0;
	while(*p != '=' && *p !='\0')
	{
	    LABEL_TREE->root->properties->property[i] = *p;
	    ++p; ++i;
	}
	LABEL_TREE->root->properties->property[i] = '\0';
	i = 0; p += 2;
	while(*p != '"' && *p !='\0')
	{
	    LABEL_TREE->root->properties->value[i] = *p;
	    ++p; ++i;
	}
	LABEL_TREE->root->properties->value[i] = '\0';
	//维护xml_node的属性和属性值和属性个数
	if(xml_node->property_node == NULL)
	{ xml_node->property_node = LABEL_TREE->root->properties; xml_node->property_num = 1; }

	return true;
    }

    P_PropertyNode temp_node = LABEL_TREE->root->properties;
    while(temp_node->next != NULL){ temp_node = temp_node->next; }
    temp_node->next = CreateOnePropertyNode();
    if(temp_node->next == NULL){ return false; }
    temp_node->next->last = temp_node;
    i = 0;
    while(*p != '=' && *p !='\0')
    {
	temp_node->next->property[i] = *p;
	++p; ++i;
    }
    temp_node->next->property[i] = '\0';
    i = 0; p += 2;
    while(*p != '"' && *p !='\0')
    {
	temp_node->next->value[i] = *p;
	++p; ++i;
    }
    temp_node->next->value[i] = '\0';
    //维护xml_node的属性和属性值和属性个数
    if(xml_node->property_node == NULL)
    { xml_node->property_node = temp_node->next; xml_node->property_num = 1; }
    else{ ++(xml_node->property_num); }

    return true;
}

static bool LabelTree_InsertValue(char* value_addr,int size,P_XML xml_node,bool finished)
{
    if(value_addr == NULL){ printf("InsertValue value_addr is NULL\n"); return false; }
    if(size == 0){ printf("InsertValue size = 0"); return false; }
    //表示上一个值处理完毕，此次处理的是一个新值
    if(finished == true)
    {
	if(LABEL_TREE->root->values->head == NULL)
	{
	    LABEL_TREE->root->values->head = CreateOneValueNode(value_addr,size);
	    if(LABEL_TREE->root->values->head == NULL){ return false; }
	    LABEL_TREE->root->values->tail = LABEL_TREE->root->values->head;
	    LABEL_TREE->root->values->count = 1;
	    xml_node->value_node = LABEL_TREE->root->values->head;
	}
	else
	{
	    LABEL_TREE->root->values->tail->next = CreateOneValueNode(value_addr,size);
	    if(LABEL_TREE->root->values->tail->next == NULL){ return false; }
	    LABEL_TREE->root->values->tail->next->last = LABEL_TREE->root->values->tail;
	    LABEL_TREE->root->values->tail = LABEL_TREE->root->values->tail->next;
	    LABEL_TREE->root->values->count += 1;
	    xml_node->value_node = LABEL_TREE->root->values->tail;
	}
    }
    //表示追加一个未处理完的值
    else
    {
	LABEL_TREE->root->values->tail->size += size;
    }
    return true;
}

static bool LabelTree_InsertKey(char* key)
{
    //维护搜索树
    if(LABEL_TREE == NULL){ printf("LabelTree_Insert LABEL_TREE is NULL\n"); return false; }
    if(LABEL_TREE->root == NULL)
    {
	LABEL_TREE->root = CreateOneKeyNode();
	if(LABEL_TREE->root == NULL){ return false; }
	strcpy(LABEL_TREE->root->key,key);
	return  true;
    }

    P_KeyNode temp_parent = LabelTree_Search(key);
    if(temp_parent == NULL){ printf("InsertKey Search failed\n"); return false; }
    if(strcmp(temp_parent->key,key) == 0)
    {
	//printf("The Element has been in the Splay Tree\n");
	return true;
    }
    P_KeyNode new_node = CreateOneKeyNode();
    if(new_node == NULL){ return false; }
    strcpy(new_node->key,key);
    
    if(strcmp(key,temp_parent->key) < 0)
    { temp_parent->lchild = new_node; }
    else{ temp_parent->rchild = new_node; }
    new_node->parent = temp_parent;
    ++(LABEL_TREE->count);
    
    //伸展操作
    ST_Splay(LABEL_TREE,new_node);

    return true;
}

static void LabelTree_Show(P_KeyNode root)
{
    if(root == NULL){ return; }
    
    printf("Label: %s",root->key);
    P_PropertyNode properties = root->properties;
    while(properties != NULL)
    {
	printf(" %s=%s",properties->property,properties->value);
	properties = properties->next;
    }
    printf("\n");
    P_ValueNode list = root->values->head;
    while(list != NULL)
    {
	printf("    %s\n",list->value);
	list = list->next;
    }
    
    LabelTree_Show(root->lchild);
    LabelTree_Show(root->rchild);
}

static void LabelTree_Free(P_KeyNode root)
{
    if(root == NULL){ return; }
    LabelTree_Free(root->lchild);
    LabelTree_Free(root->rchild);
    root->values->tail = root->values->head;
    while(root->values->head != NULL)
    {
	root->values->tail = root->values->head;
	root->values->head = root->values->head->next;
	SAFE_FREE(root->values->tail);
    }
    SAFE_FREE(root->values);
    P_PropertyNode property;
    while(root->properties != NULL)
    {
	property = root->properties;
	root->properties = root->properties->next;
	SAFE_FREE(property);
    }
    //printf("%s\n",root->key);
    SAFE_FREE(root);
}

static int tab = 0;
static void XML_Print(P_XML head,FILE* s)
{
    if(head == NULL){ return; }
    int i = 0;
    while(i < tab){ fputc(0x9,s); ++i; }
    fprintf(s,"<%s",head->key);
    if(head->property_node != NULL)
    {
	i = 0; P_PropertyNode pn = head->property_node;
	while(i < head->property_num)
	{
	    fprintf(s," %s=\"%s\"",pn->property,pn->value);
	    ++i; pn = pn->next;
	}
    }
    if(head->value_node != NULL){ fprintf(s,">"); }
    else
    {
	if(head->child_list->head == NULL)
	{ fprintf(s,">"); }
	else{ fprintf(s,">%s",BR); }
    }
    
    ++tab;
    P_XML temp = head->child_list->head;
    while(temp != NULL)
    {
	XML_Print(temp,s);
	temp = temp->next;
    }
    --tab;
    
    if(head->value_node != NULL)
    {
	fprintf(s,"%s</%s>%s",head->value_node->value,head->key,BR);
    }
    else
    {
	if(head->child_list->head == NULL)
	{ fprintf(s,"</%s>%s",head->key,BR); }
	else
	{
	    i = 0;
	    while(i < tab){ fputc(0x9,s); ++i; }
	    fprintf(s,"</%s>%s",head->key,BR);
       	}
    }
}

static void XML_Free(P_XML head)
{
    if(head == NULL){ return; }
    //printf("%s\n",head->key);
    P_XML node = head->child_list->head;
    SAFE_FREE(head->child_list);
    SAFE_FREE(head);
    P_XML temp_node;
    while(node != NULL)
    {
	temp_node = node;
	node = node->next;
	XML_Free(temp_node);
    }
}

static bool CreateRAM_Pool(unsigned int size)
{
    POOL = (P_RAM_POOL)malloc(sizeof(RAM_POOL));
    if(POOL == NULL){ printf("ram POOL malloc failed\n"); return false; }
    POOL->addr = (char*)malloc(size);
    if(POOL->addr == NULL){ SAFE_FREE(POOL); printf("ram POOL malloc failed\n"); return false; }
    POOL->size = size;
    POOL->read_addr = POOL->addr;
    POOL->write_addr = POOL->addr;
    POOL->data_len = 0;
    return true;
}

static char* Pool_GetWritePosition()
{
    return POOL->write_addr;
}

static int Pool_WriteData(char* v,bool is_add)
{
    if(v == NULL){ printf("Pool_WriteData data is NULL"); return 0; }
    int counter = 0;
    //若是追加写值，则把写指针后退一位，写的时候覆盖之前的'\0'
    if(is_add == false)
    {
	--(POOL->write_addr);
	--(POOL->data_len);
    }
    while(*v != '<' && *v != '\0' && *v != '\r' && *v != '\n' && *v != EOF)
    {
	++counter;
	if(POOL->data_len + counter > POOL->size)
	{
	    printf("Pool_WriteData POOL is full\n");
	    POOL->data_len = POOL->size;
	    //写指针复位到内存池开始的地方，若强制再写，则会覆盖数据
	    POOL->write_addr = POOL->addr;
	    return 0;
	}
	//处理引用
	if(*v == '&')
	{
	    if(*(v + 1) == 'l' && *(v + 2) == 't' && *(v + 3) == ';')
	    {
		*(POOL->write_addr) = '<';
		v += 4;
	    }
	    else if(*(v + 1) == 'g' && *(v + 2) == 't' && *(v + 3) == ';')
	    {
		*(POOL->write_addr) = '<';
		v += 4;
	    }
	    else if(*(v + 1) == 'a' && *(v + 2) == 'm' && *(v + 3) == 'p' && *(v + 4) == ';')
	    {
		*(POOL->write_addr) = '&';
		v += 5;
	    }
	    else if(*(v + 1) == 'a' && *(v + 2) == 'p' && *(v + 3) == 'o' && *(v + 4) == 's' && *(v + 5) == ';')
	    {
		*(POOL->write_addr) = '\'';
		v += 6;
	    }
	    else if(*(v + 1) == 'q' && *(v + 2) == 'u' && *(v + 3) == 'o' && *(v + 4) == 't' && *(v + 5) == ';')
	    {
		*(POOL->write_addr) = '"';
		v += 6;
	    }
	    else
	    {
		*(POOL->write_addr) = *v;
		++v;
	    }
	}
	else
	{
	    *(POOL->write_addr) = *v;
	    ++v;
	}
	++(POOL->write_addr);
    }
    //维护内存池数据
    POOL->data_len += counter;
    if(POOL->data_len < POOL->size)
    {
	*(POOL->write_addr) = '\0';
	(POOL->write_addr) += 1;
	POOL->data_len += 1;
    }
    else
    {
	printf("Pool_WriteData POOL is full\n");
	POOL->data_len = POOL->size;
	//写指针复位到内存池开始的地方，若强制再写，则会覆盖数据
	POOL->write_addr = POOL->addr;
	return 0;
    }

    return counter;
}

static void FreeRAM_Pool()
{
    if(POOL != NULL){ SAFE_FREE(POOL->addr); }
    SAFE_FREE(POOL);
}

static bool DealOneRow(char* row)
{
    bool result;
    while((*row) != '\n' && (*row) != '\r' && (*row) != '\0' && (*row) != EOF)
    {
	//0x9为tab键值
	if((*row) == ' ' || (*row) == 0x9)
	{
	    //do nothing，可以清除 值开头结尾的空白字符
	}
	//标签
	else if((*row) == '<')
	{
	    key_counter = 0;
	    ++row;
	    while((*row) == ' ' || (*row) == 0x9){ ++row; }
	    //xml头标签
	    if((*row) == '?')
	    {
		++row;
		while((*row) == ' ' || (*row) == 0x9){ ++row; }
		row += 3;//跳过xml三个字符
		while((*row) != '>')
		{
		    while((*row) == ' ' || (*row) == 0x9){ ++row; }
		    if((*row == '?'))//标签头结尾?处理
		    { ++row; while((*row) == ' ' || (*row) == 0x9){ ++row; } continue; }
		    //此处借用一下key_buffer
		    key_counter = 0;
		    while((*row) != '=' && (*row) != ' ' && (*row) != 0x9)
		    {
			KEY_BUFFER[key_counter] = (*row);
			++key_counter;
			++row;
		    }
		    KEY_BUFFER[key_counter] = '\0';
		    while((*row) != '"'){++row; }
		    ++row;//值开始
		    //双引号中的属性值的前后空格在此不做处理
		    if(strcmp(KEY_BUFFER,"version") == 0)
		    {
			key_counter = 0;
			while(*(row) != '"')
			{ version[key_counter] = (*row); ++key_counter; ++row; }
		    }
		    else if(strcmp(KEY_BUFFER,"encoding") == 0)
		    {
			key_counter = 0;
			while(*(row) != '"')
			{ encoding[key_counter] = (*row); ++key_counter; ++row; }
		    }
		    ++row;
		}
	    }
	    //xml注释
	    else if((*row) == '!')
	    {
		while((*row) != '>'){ ++row; }
	    }
	    //结束标签
	    else if((*row) == '/')
	    {
		while((*row) != '>'){ ++row; }
		parent_pointer = parent_pointer->parent;
		value_finished = true;
	    }
	    //开始标签
	    else
	    {
		//分为有属性的开始标签和没有属性的开始标签
		//key
		while(((*row) != ' ') && ((*row) != 0x9) && ((*row) != '>'))
		{
		    KEY_BUFFER[key_counter] = (*row);
		    ++row;
		    ++key_counter;
		}
		KEY_BUFFER[key_counter] = '\0';
		//至此开始标签择出来,压入搜索树，由于伸展树的作用，此时LABEL_TREE->root即为处理的标签
		result = LabelTree_InsertKey(KEY_BUFFER);
		if(result == false){ printf("InsertKey falied\n"); return false; }

		//维护结构链表
		P_XML xml_node = CreateOneXMLNode();
		if(xml_node == NULL){ return false; }
		xml_node->key = LABEL_TREE->root->key;
		//根标签
		if(XML_LIST == NULL)
		{
		    XML_LIST = xml_node;
		}
		else
		{
		    if(parent_pointer->child_list->head == NULL)
		    {
			parent_pointer->child_list->head = xml_node;
			parent_pointer->child_list->tail = xml_node;
			parent_pointer->child_list->count = 1;
		    }
		    else
		    {
			parent_pointer->child_list->tail->next = xml_node;
			xml_node->last = parent_pointer->child_list->tail;
			parent_pointer->child_list->tail = xml_node;
			parent_pointer->child_list->count += 1;
		    }
		    xml_node->parent = parent_pointer;
		}
		//前进到当前结点，配合着结束标签的退回上一级
		parent_pointer = xml_node;

		//properties-将每个属性连带值(如 year="2016")一起攒给LabelTree_InsertProperty
		if((*row) == ' ' || (*row) == 0x9)
		{
		    int quotes_counter;
		    while((*row) != '>')
		    {
			if((*row) == ' ' || (*row) == 0x9)
			{
			    ++row;
			}
			else
			{
			    key_counter = 0;
			    quotes_counter = 0;
			    while((*row) != '>')
			    {
				//去除属性中存在的空格或tab键
				if((*row) != ' ' && (*row) != 0x9)
				{
				    if((*row) == '"'){ ++quotes_counter; }
				    KEY_BUFFER[key_counter] = (*row);
				    ++key_counter;
				}
				++row;
				if(quotes_counter == 2){ break; }
			    }
			    KEY_BUFFER[key_counter] = '\0';
			    result = LabelTree_InsertProperty(KEY_BUFFER,xml_node);
			    if(result == false){ printf("InsertProperty falied\n"); return false; }
			}
		    }
		}//开始标签属性处理结束
	    }//开始标签结束
	}//标签结束
	//值
	else
	{
	    char* start_addr = POOL->write_addr;
	    //将值写入pool中
	    value_counter = Pool_WriteData(row,value_finished);
	    if(value_counter == 0){ return false; }
	    //至此值写入pool中，将值插入标签树，此时的KEY_BUFFER中也
	    //必定是最新插入树中的，同时也伸展至了树根处
	    result = LabelTree_InsertValue(start_addr,value_counter,parent_pointer,value_finished);
	    if(result == false){ printf("InsertValue falied\n"); return false; }
	    //更新row到值的最后一个字符
	    char end = *(row+1);
	    while(end != '<' && end != '\0' && end != '\r' && end != EOF){ ++row; end = *(row+1); }
	    
	    //维护value_finshed，处理完一个值后，假设还没处理完，若值后跟的是结束标签，则会将之置true
	    value_finished = false;
	}
	//处理完毕，向前走一个字节
	if(row == NULL){ return false; }
	++row;
    }
    return true;
}

bool ReadXml(char* file)
{
    LABEL_TREE = CreateOneKeyTree();
    if(LABEL_TREE == NULL){ return false; }
    if(CreateRAM_Pool(RAM_POOL_SIZE) == false){ SAFE_FREE(LABEL_TREE); return false; }
    XML_FILE = fopen(file,"r+");
    if(XML_FILE == NULL)
    {
	printf("open file failed\n");
	SAFE_FREE(LABEL_TREE);
	SAFE_FREE(POOL->addr);
	SAFE_FREE(POOL);
	return false;
    }
    
    //处理报文体
    while(fgets(ROW,ROW_BUFFER_SIZE,XML_FILE) != NULL)
    {
	if(DealOneRow(ROW) == false)
	{
	    printf("DealOneRow error!\n");
	    CloseXml();
	    return false;
	}
    }
    return true;
}

bool GetValue(char* key,char* value,char* property,char* property_value,ELEMENT_TYPE value_type)
{
    if(value_type == VALUE)
    {
	if(key == NULL || value == NULL){ return false; }
	//传入的property与property_value必须同时为null或同时不为null
	if(property == NULL && property_value == NULL)
	{
	    //情况①：搜索tree
	    if(LABEL_TREE == NULL || LABEL_TREE->root == NULL){ return false; }
	    LabelTree_Search(key);
	    if(strcmp(LABEL_TREE->root->key,key) == 0)
	    {
		strncpy(value,LABEL_TREE->root->values->head->value,
			LABEL_TREE->root->values->head->size);
	    }
	    else{ return false; }
	}
	else if(property != NULL && property_value != NULL)
	{
	    //情况②：搜索list
	    if(XML_LIST == NULL){ return false; }
	    P_XML xn = XML_Search(XML_LIST,key,property,property_value);
	    if(xn != NULL)
	    {
		if(xn->value_node != NULL){ strncpy(value,xn->value_node->value,xn->value_node->size); }
		else{ printf("there is no value in the label\n"); return false; }
	    }
	    else
	    { return false; }
	}
	else
	{ printf("property and property_value must be null or not be null at same time\n"); }
    }
    //取属性值时，只保证取到第一个key的property的值，参数value不参与此分支
    else if(value_type == PROPERTY_VALUE)
    {
	if(key == NULL || property == NULL || property_value == NULL){ return false; }
	if(LABEL_TREE == NULL || LABEL_TREE->root == NULL){ return false; }
	LabelTree_Search(key);
	if(strcmp(LABEL_TREE->root->key,key) == 0)
	{
	    P_PropertyNode pn = LABEL_TREE->root->properties;
	    while(pn != NULL)
	    {
		if(strcmp(property,pn->property) == 0)
		{
		    strncpy(property_value,pn->value,MAX_PROPERTY_VALUE_LEN);
		    break;
		}
		pn = pn->next;
	    }
	    if(pn == NULL){ return false; }
	}
    }
    else
    {
	printf("ELEMENT_TYPE is illegal\n");
    }
    return true;
}

P_XML GetXmlNode(char* key,char* property,char* property_value)
{
    if(XML_LIST == NULL){ return NULL; }
    if(key == NULL){ return NULL; }
    P_XML xn = XML_Search(XML_LIST,key,property,property_value);
    return xn;
}

P_XML GetChilds(char* pk,char* pp,char* ppv)
{
    if(XML_LIST == NULL){ return NULL; }
    if(pk == NULL){ return NULL; }
    P_XML xn = XML_Search(XML_LIST,pk,pp,ppv);
    if(xn == NULL){ return NULL; }
    return xn->child_list->head;
}

P_ValueNode GetValues(char* key)
{
    if(LABEL_TREE == NULL || LABEL_TREE->root == NULL){ return NULL; }
    LabelTree_Search(key);
    if(strcmp(key,LABEL_TREE->root->key) == 0)
    { return LABEL_TREE->root->values->head; }
    return NULL;
}

bool GetValueFromFile(char* xf,char* k,char* v,char* p,char* pv,ELEMENT_TYPE vt)
{
    if(xf == NULL || k == NULL){ return false; }
    if(vt == VALUE && v == NULL){ return false; }
    if(vt == PROPERTY_VALUE && (p == NULL || pv == NULL)){ return false; }
    FILE *xml = fopen(xf,"r");
    if(xml == NULL){ printf("open file failed\n"); return false; }
    
    bool is_match = false;
    bool is_finished = true;
    while(fgets(ROW,ROW_BUFFER_SIZE,xml) != NULL)
    {
	char* row = ROW;
	while((*row) != '\n' && (*row) != '\r' && (*row) != '\0' && (*row) != EOF)
	{
	    if((*row) == ' ' || (*row) == 0x9)
	    {
		//do nothing
	    }
	    else if((*row) == '<')
	    {
		key_counter = 0;
		++row;
		while((*row) == ' ' || (*row) == 0x9){ ++row; }
		//非开始标签
		if((*row) == '?' || (*row) == '!')
		{
		    while((*row) != '>'){ ++row; }
		}
		else if((*row) == '/')
		{
		    while((*row) != '>'){ ++row; }
		    is_finished = true;
		}
		//开始标签
		else
		{
		    //分为有属性的开始标签和没有属性的开始标签
		    //key
		    while(((*row) != ' ') && ((*row) != 0x9) && ((*row) != '>'))
		    {
			KEY_BUFFER[key_counter] = (*row);
			++row;
			++key_counter;
		    }
		    KEY_BUFFER[key_counter] = '\0';
		    if(strcmp(k,KEY_BUFFER) == 0)
		    {
			if(vt == VALUE)
			{
			    if(p != NULL && pv != NULL)
			    {
				//检查是否匹配属性，若匹配就地返回，若不匹配则继续
				if((*row) == ' ' || (*row) == 0x9)
				{
				    int quotes_counter;
				    while((*row) != '>')
				    {
					if((*row) == ' ' || (*row) == 0x9)
					{
					    ++row;
					}
					else
					{
					    key_counter = 0;
					    quotes_counter = 0;
					    while((*row) != ' ' && (*row) != 0x9 && (*row) != '=')
					    {
						KEY_BUFFER[key_counter] = *(row);
						++key_counter; ++row;
					    }
					    KEY_BUFFER[key_counter] = '\0';
					    if(strcmp(p,KEY_BUFFER) == 0)
					    {
						key_counter = 0;
						while((*row) != '>')
						{
						    if((*row) != ' ' && (*row) != 0x9 && (*row) != '"' && (*row) != '=')
						    {
							KEY_BUFFER[key_counter] = (*row);
							++key_counter;
						    }
						    if((*row) == '"'){ ++quotes_counter; }
						    if(quotes_counter == 2){ break; }
						    ++row;
						}
						KEY_BUFFER[key_counter] = '\0';
						if(strcmp(pv,KEY_BUFFER) == 0)
						{
						    is_match = true;
						    is_finished = false;
						}
					    }
					    else
					    {
						while((*row) != '>')
						{
						    if((*row) == '"'){ ++quotes_counter; }
						    if(quotes_counter == 2){ break; }
						    ++row;
						}
					    }
					    ++row;
					}
				    }
				}
			    }
			    else
			    {
				is_match = true;
				is_finished = false;
				while((*row) != '>'){ ++row; }
			    }
			}
			else if(vt == PROPERTY_VALUE)
			{
			    //查找属性是否匹配，若匹配就地返回，若不匹配则继续
			    if((*row) == ' ' || (*row) == 0x9)
			    {
				int quotes_counter;
				while((*row) != '>')
				{
				    if((*row) == ' ' || (*row) == 0x9)
				    {
					++row;
				    }
				    else
				    {
					key_counter = 0;
					quotes_counter = 0;
					while((*row) != ' ' && (*row) != 0x9 && (*row) != '=')
					{
					    KEY_BUFFER[key_counter] = *(row);
					    ++key_counter; ++row;
					}
					KEY_BUFFER[key_counter] = '\0';
					if(strcmp(p,KEY_BUFFER) == 0)
					{
					    key_counter = 0;
					    while((*row) != '>')
					    {
						if((*row) != ' ' && (*row) != 0x9 && (*row) != '"' && (*row) != '=')
						{
						    KEY_BUFFER[key_counter] = (*row);
						    ++key_counter;
						}
						if((*row) == '"'){ ++quotes_counter; }
						if(quotes_counter == 2){ break; }
						++row;
					    }
					    KEY_BUFFER[key_counter] = '\0';
					    strncpy(pv,KEY_BUFFER,MAX_PROPERTY_VALUE_LEN);
					    fclose(xml);
					    return true;
					}
					else
					{
					    while((*row) != '>')
					    {
						if((*row) == '"'){ ++quotes_counter; }
						if(quotes_counter == 2){ break; }
						++row;
					    }
					}
					++row;
				    }
				}
			    }
			}
		    }
		    else
		    {
			is_match = false;
			while((*row) != '>'){ ++row; }
		    }
		}//开始标签结束
	    }//标签结束
	    //值
	    else
	    {
		if(is_match == true)
		{
		    if(is_finished == false)
		    {
			while((*row) != '\n' && (*row) != '\r' && (*row) != '\0' && (*row) != EOF && *(row) != '<')
			{
			    (*v) = (*row);
			    ++v; ++row;
			}
			*v = '\0';
			if(*(row) == '<'){ --row; }
		    }
		    else
		    {
			fclose(xml);
			return true;
		    }
		}
		else
		{
		    while((*row) != '\n' && (*row) != '\r' && (*row) != '\0' && (*row) != EOF && *(row+1) != '<')
		    { ++row; }
		}
	    }
	    //处理完毕，向前走一个字节
	    if(row == NULL){ fclose(xml); return false; }
	    ++row;
	}//处理行循环结束
    }//处理文本结束
    fclose(xml);
    return false;
}

bool SetXmlHead(char* v,char* e)
{
    if(v != NULL){ strncpy(version,v,sizeof(version)); }
    else{ printf("version is not setted\n"); return false; }
    if(e != NULL){ strncpy(encoding,e,sizeof(encoding)); }
    else{ printf("encoding is not setted"); return false; }

    return true;
}

bool SetValue(char* k,char* p,char* pv,char* nv,ELEMENT_TYPE vt)
{
    if(k == NULL || nv == NULL){ return false; }
    if((vt == PROPERTY || vt == PROPERTY_VALUE) && (p == NULL || pv == NULL)){ return false; }
    LabelTree_Search(k);
    if(strcmp(LABEL_TREE->root->key,k) != 0){ return false; }
    P_XML xn = XML_Search(XML_LIST,k,p,pv);
    if(xn == NULL){ return false; }
    //修改值
    if(vt == VALUE)
    {
	if(xn->value_node != NULL)
	{
	    xn->value_node->value = POOL->write_addr;
	    //旧值内存地址未回收
	    xn->value_node->size = Pool_WriteData(nv,true);
	    if(xn->value_node->size == 0){ return false; }
	}
	else
	{ return false; }
    }
    else if(vt == PROPERTY || vt == PROPERTY_VALUE)
    {
	//能走到这儿说明肯定存在
	P_PropertyNode pn = xn->property_node;
	int i = 0;
	while(i < xn->property_num)
	{
	    if(strcmp(pn->property,p) == 0){ break; }
	    pn = pn->next;
	    ++i;
	}
	//修改属性值
	if(vt == PROPERTY_VALUE)
	{
	    strncpy(pn->value,nv,MAX_PROPERTY_VALUE_LEN);
	}
	//修改属性
	else
	{
	    strncpy(pn->property,nv,MAX_PROPERTY_LEN);
	}
	
    }
    return true;
}

bool AddOne(char* pk,char* pp,char* ppv,char* nk,char* nv,...)
{
    if(pk == NULL){ return false; }
    P_XML xn = XML_Search(XML_LIST,pk,pp,ppv);
    if(xn == NULL){ return false; }
    va_list list;
    va_start(list,nv);
    ELEMENT_TYPE vt;
    //ELEMENT_TYPE枚举值和最大值为15，若某参数地址的值为15，则会出错
    if(*((int*)list) < 16)
    {
	vt = *((int*)list);
	if((vt & (PROPERTY|PROPERTY_VALUE)) != 0){ return false; }
    }
    else
    {
	while(1)
	{
	    va_arg(list,char*);
	    if(*((int*)list) < 16){ vt = *((int*)list); break; }
	}
    }
    //printf("vt=%d\n",vt);
    //重置变长参数
    va_start(list,nv);
    //增加属性
    if(vt == (PROPERTY|PROPERTY_VALUE))
    {
	char* tmp;
	while(1)
	{
	    P_PropertyNode pn = CreateOnePropertyNode();
	    if(pn == NULL){ return false; }
	    tmp = va_arg(list,char*);
	    strncpy(pn->property,tmp,MAX_PROPERTY_LEN);
	    tmp = va_arg(list,char*);
	    strncpy(pn->value,tmp,MAX_PROPERTY_VALUE_LEN);
	    //XML_LIST能搜出来，TREE中肯定能存在
	    LabelTree_Search(pk);
	    if(xn->property_node == NULL)
	    {
		if(LABEL_TREE->root->properties == NULL)
		{
		    LABEL_TREE->root->properties = pn;
		}
		else
		{
		    P_PropertyNode tmp = LABEL_TREE->root->properties;
		    while(tmp->next != NULL){ tmp = tmp->next; }
		    tmp->next = pn;
		    pn->last = tmp;
		}
		xn->property_node = pn;
		xn->property_num = 1;
	    }
	    else
	    {
		P_PropertyNode tpn = xn->property_node;
		P_PropertyNode end = NULL;
		int i = 0;
		while(i < xn->property_num)
		{
		    end = tpn;
		    if(strcmp(tpn->property,pn->property) == 0){ break; }
		    tpn = tpn->next; ++i;
		}
		//若已存在相同的属性值，则返回
		if(i < xn->property_num)
		{ SAFE_FREE(pn); if(*((int*)list) < 16){ return true; }else{ continue; } }
		pn->next = end->next;
		pn->last = end;
		if(end->next != NULL){ end->next->last = pn; }
		end->next = pn;
		
		xn->property_num += 1;
	    }
	    if(*((int*)list) < 16){ break; }
	}
    }
    else if(vt == VALUE)
    {
	if(nv == NULL){ return false; }
	if(xn->value_node != NULL){ return false; }
	if(xn->child_list != NULL){ return false; }
	LabelTree_Search(pk);
	char* value_addr = POOL->write_addr;
	int size = Pool_WriteData(nv,true);
	if(size == 0){ return false; }
	LabelTree_InsertValue(value_addr,size,xn,true);
    }
    else if(vt == (KEY) || vt == (KEY|PROPERTY|PROPERTY_VALUE) || vt == (KEY|PROPERTY|PROPERTY_VALUE|VALUE))
    {
	if(nk == NULL){ return false; }
	if(xn->value_node != NULL){ return false; }
	LabelTree_InsertKey(nk);
	P_XML nxn = CreateOneXMLNode();
	//key
	nxn->key = LABEL_TREE->root->key;
	//值
	if((vt & VALUE) != 0)
	{
	    if(nv == NULL){ SAFE_FREE(nxn); return false; }
	    char* value_addr = POOL->write_addr;
	    int size = Pool_WriteData(nv,true);
	    if(size == 0){ return false; }
	    LabelTree_InsertValue(value_addr,size,nxn,true);
	}
	//添加属性值
	if((vt & (PROPERTY|PROPERTY_VALUE)) != 0)
	{
	    char* tmp;
	    while(1)
	    {
		P_PropertyNode pn = CreateOnePropertyNode();
		if(pn == NULL){ return false; }
		tmp = va_arg(list,char*);
		strncpy(pn->property,tmp,MAX_PROPERTY_LEN);
		tmp = va_arg(list,char*);
		strncpy(pn->value,tmp,MAX_PROPERTY_VALUE_LEN);
		P_PropertyNode tpn;
		if(nxn->property_node == NULL)
		{
		    if(LABEL_TREE->root->properties == NULL)
		    {
			LABEL_TREE->root->properties = pn;
		    }
		    else
		    {
			tpn = LABEL_TREE->root->properties;
			while(tpn->next != NULL){ tpn = tpn->next; }
			tpn->next = pn;
			pn->last = tpn;
		    }
		    nxn->property_node = pn;
		    nxn->property_num = 1;
		}
		else
		{
		    tpn = nxn->property_node;
		    P_PropertyNode end = NULL;
		    int i = 0;
		    while(i < nxn->property_num)
		    {
			end = tpn;
			if(strcmp(tpn->property,pn->property) == 0){ break; }
			tpn = tpn->next; ++i;
		    }
		    //若已存在相同的属性值，则略过或返回
		    if(i < nxn->property_num)
		    { SAFE_FREE(pn); if(*((int*)list) < 16){ break; }else{ continue; } }
		    pn->next = end->next;
		    pn->last = end;
		    if(end->next != NULL){ end->next->last = pn; }
		    end->next = pn;
		    nxn->property_num += 1;
		}
		if(*((int*)list) < 16){ break; }
	    }
	}

	//与父节点挂接
	nxn->parent = xn;
	P_XML txn = xn->child_list->head;
	if(txn == NULL)
	{
	    xn->child_list->head = nxn;
	    xn->child_list->tail = nxn;
	    xn->child_list->count = 1;
	}
	else
	{
	    xn->child_list->tail->next = nxn;
	    nxn->last = xn->child_list->tail;
	    xn->child_list->tail = nxn;
	    xn->child_list->count += 1;
	}
    }
    else
    {
	printf("illegal ELEMENT_TYPE\n");
	return false;
    }
    
    va_end(list);
    return true;
}

bool DelOne(char* k,char* p,char* pv,ELEMENT_TYPE vt)
{
    if(k == NULL){ return false; }
    if(vt == PROPERTY && (p == NULL || pv == NULL)){ return false; }
    P_XML xn = XML_Search(XML_LIST,k,p,pv);
    if(xn == NULL){ return false; }
    int i = 0;
    LabelTree_Search(k);

    if(vt == PROPERTY)
    {
	//能走到这里，这个属性一定存在
	P_PropertyNode pn = xn->property_node;
	for(i = 0; i < xn->property_num; ++i)
	{
	    if(strcmp(p,pn->property) == 0){ break; }
	    pn = pn->next;
	}
	
	if(pn == LABEL_TREE->root->properties)
	{
	    //也一定是xn的属性链表的头
	    if(LABEL_TREE->root->properties->next == NULL){ LABEL_TREE->root->properties = NULL; }
	    else{ LABEL_TREE->root->properties = pn->next; }
	}
	
	if(i == 0)
	{
	    if(xn->property_num == 1){ xn->property_node = NULL; }
	    else
	    {
		//next肯定存在
		xn->property_node = pn->next;
		if(pn->last != NULL){ pn->next->last = pn->last; pn->last->next = pn->next; }
		else{ pn->next->last = NULL; }
	    }
	}
	else
	{
	    //last肯定存在
	    pn->last->next = pn->next;
	    if(pn->next != NULL){ pn->next->last = pn->last; }
	}

	xn->property_num -= 1;
	SAFE_FREE(pn);
    }
    else if(vt == VALUE)
    {
	if(xn->value_node == NULL){ return true; }
	
	if(LABEL_TREE->root->values->count == 1)
	{
	    LABEL_TREE->root->values->head = NULL;
	    LABEL_TREE->root->values->tail = NULL;
	}
	else
	{
	    if(xn->value_node == LABEL_TREE->root->values->head)
	    {
		LABEL_TREE->root->values->head = xn->value_node->next;
		xn->value_node->next->last = NULL;
	    }
	    else if(xn->value_node == LABEL_TREE->root->values->tail)
	    {
		LABEL_TREE->root->values->tail = xn->value_node->last;
		xn->value_node->last->next = NULL;
	    }
	    else
	    {
		xn->value_node->last->next = xn->value_node->next;
		xn->value_node->next->last = xn->value_node->last;
	    }
	}
	//值内存池该处地方未回收
	LABEL_TREE->root->values->count -= 1;
	SAFE_FREE(xn->value_node);
	xn->value_node = NULL;
    }
    else if(vt == KEY)
    {
	if(xn->parent == NULL){ printf("root can not be deleted\n"); return false; }
	//从XML_LIST中删除xn
	if(xn->parent->child_list->count == 1)
	{
	    xn->parent->child_list->head = NULL;
	    xn->parent->child_list->tail = NULL;
	}
	else
	{
	    if(xn == xn->parent->child_list->head)
	    {
		xn->parent->child_list->head = xn->next;
		xn->next->last = NULL;
	    }
	    else if(xn == xn->parent->child_list->tail)
	    {
		xn->parent->child_list->tail = xn->last;
		xn->last->next = NULL;
	    }
	    else
	    {
		xn->last->next = xn->next;
		xn->next->last = xn->last;
	    }
	}
	//从LABEL_TREE中删除属性和值，KeyNode自身不删除
	if(xn->property_node != NULL)
	{
	    P_PropertyNode pn;
	    P_PropertyNode tpn = xn->property_node->last;
	    for(i = 0; i < xn->property_num; ++i)
	    {
		pn = xn->property_node;
		xn->property_node = xn->property_node->next;
		SAFE_FREE(pn);
	    }
	    if(tpn == NULL)
	    {
		LABEL_TREE->root->properties = xn->property_node;
	    }
	    else
	    {
		tpn->next = xn->property_node;
		if(xn->property_node != NULL){ xn->property_node->last = tpn; }
	    }
	}
	if(xn->value_node != NULL)
	{   
	    if(LABEL_TREE->root->values->count == 1)
	    {
		LABEL_TREE->root->values->head = NULL;
		LABEL_TREE->root->values->tail = NULL;
	    }
	    else
	    {
		if(xn->value_node == LABEL_TREE->root->values->head)
		{
		    LABEL_TREE->root->values->head = xn->value_node->next;
		    xn->value_node->next->last = NULL;
		}
		else if(xn->value_node == LABEL_TREE->root->values->tail)
		{
		    LABEL_TREE->root->values->tail = xn->value_node->last;
		    xn->value_node->last->next = NULL;
		}
		else
		{
		    xn->value_node->last->next = xn->value_node->next;
		    xn->value_node->next->last = xn->value_node->last;
		}
	    }
	    //值内存池该处地方未回收
	    LABEL_TREE->root->values->count -= 1;
	    SAFE_FREE(xn->value_node);
	    xn->value_node = NULL;
	}
	
	P_XML child = xn->child_list->head;
	SAFE_FREE(xn->child_list);
	SAFE_FREE(xn);

	//尾递归释放子标签
	while(child != NULL)
	{
	    if(child->property_node != NULL)
	    {
		DelOne(child->key,child->property_node->property,child->property_node->value,KEY);
	    }
	    else
	    {
		DelOne(child->key,NULL,NULL,KEY);
	    }
	    child = child->next;
	}
    }
    else
    {
	printf("illegal vt\n");
	return false;
    }
    return true;
}

bool AddOneLabel(char* pk,char* pp,char* ppv,char* label)
{
    if(label == NULL){ return false; }
    P_XML pxn = NULL;
    P_XML nxn = NULL;
    if(pk != NULL){ pxn = XML_Search(XML_LIST,pk,pp,ppv); }   
    int result;
    while((*label) != '\0')
    {
	if((*label) == ' ' || (*label) == 0x9)
	{
	    //do nothing
	}
	//标签
	else if((*label) == '<')
	{
	    key_counter = 0;
	    ++label;
	    while((*label) == ' ' || (*label) == 0x9){ ++label; }
	    //xml头标签/注释
	    if((*label) == '?' || (*label) == '!')
	    {
		return false;
	    }
	    //结束标签
	    else if((*label) == '/')
	    {
		while((*label) != '>'){ ++label; }
	    }
	    //开始标签
	    else
	    {
		while(((*label) != ' ') && ((*label) != 0x9) && ((*label) != '>'))
		{
		    KEY_BUFFER[key_counter] = (*label);
		    ++label;
		    ++key_counter;
		}
		KEY_BUFFER[key_counter] = '\0';
		result = LabelTree_InsertKey(KEY_BUFFER);
		if(result == false){ printf("InsertKey falied\n"); return false; }

		nxn = CreateOneXMLNode();
		if(nxn == NULL){ return false; }
		nxn->key = LABEL_TREE->root->key;
		//根标签
		if(XML_LIST == NULL)
		{
		    XML_LIST = nxn;
		}
		else
		{
		    if(pxn != NULL && pxn->value_node == NULL)
		    {
			if(pxn->child_list->head == NULL)
			{
			    pxn->child_list->head = nxn;
			    pxn->child_list->tail = nxn;
			    pxn->child_list->count = 1;
			}
			else
			{
			    pxn->child_list->tail->next = nxn;
			    nxn->last = pxn->child_list->tail;
			    pxn->child_list->tail = nxn;
			    pxn->child_list->count += 1;
			}
			nxn->parent = pxn;
		    }
		}
		
		if((*label) == ' ' || (*label) == 0x9)
		{
		    int quotes_counter;
		    while((*label) != '>')
		    {
			if((*label) == ' ' || (*label) == 0x9)
			{
			    ++label;
			}
			else
			{
			    key_counter = 0;
			    quotes_counter = 0;
			    while((*label) != '>')
			    {
				if((*label) != ' ' && (*label) != 0x9)
				{
				    if((*label) == '"'){ ++quotes_counter; }
				    KEY_BUFFER[key_counter] = (*label);
				    ++key_counter;
				}
				++label;
				if(quotes_counter == 2){ break; }
			    }
			    KEY_BUFFER[key_counter] = '\0';
			    result = LabelTree_InsertProperty(KEY_BUFFER,nxn);
			    if(result == false){ printf("InsertProperty falied\n"); return false; }
			}
		    }
		}
	    }
	}
	//值
	else
	{
	    if(nxn == NULL){ return false; }
	    char* start_addr = POOL->write_addr;
	    value_counter = Pool_WriteData(label,value_finished);
	    if(value_counter == 0){ return false; }
	    result = LabelTree_InsertValue(start_addr,value_counter,nxn,value_finished);
	    if(result == false){ printf("InsertValue falied\n"); return false; }
	    char end = *(label+1);
	    while(end != '<' && end != '\0'){ ++label; end = *(label+1); }
	}
	//处理完毕，向前走一个字节
	if(label == NULL){ return false; }
	++label;
    }

    return true;
}

bool CommitToFile(char* file)
{
    FILE* tmp = fopen(file,"w");
    if(tmp == NULL){ return false; }
    fprintf(tmp,"<?xml version=\"%s\" encoding=\"%s\"?>%s",version,encoding,BR);
    XML_Print(XML_LIST,tmp);
    fclose(tmp);
    return true;
}

void CloseXml()
{
    XML_Free(XML_LIST);
    LabelTree_Free(LABEL_TREE->root);
    SAFE_FREE(LABEL_TREE);
    FreeRAM_Pool();
    fclose(XML_FILE);
}

