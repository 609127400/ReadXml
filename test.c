#include "read_xml.h"


#ifdef _WINDOWS_
#pragma comment(lib,"read_xml.lib")
#endif


int main()
{


    if(ReadXml("./xml.xml") == false){ return 0; }

    char buffer[100] = {0};
    
    if(GetValue("APP",buffer,"id","1",VALUE) == true){ printf("%s\n",buffer); }
    else{ printf("GetValue failed\n"); }

    if(GetValue("APP",buffer,NULL,NULL,VALUE) == true){ printf("%s\n",buffer); }
    else{ printf("GetValue failed\n"); }

    if(GetValue("APP",buffer,"id","3",VALUE) == true){ printf("%s\n",buffer); }
    else{ printf("GetValue failed\n"); }

    if(GetValue("APP",NULL,"id",buffer,PROPERTY_VALUE) == true){ printf("%s\n",buffer); }
    else{ printf("GetValue failed\n"); }

    if(GetValue("VacNbr",NULL,"bingo",buffer,PROPERTY_VALUE) == true){ printf("%s\n",buffer); }
    else{ printf("GetValue failed\n"); }

    CloseXml();

    return 0;
}
