#include "mr_common.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp;
    char *path = "/home/kip/Desktop/wanghao";

    int id;
/*
    fp = fopen(path,"w");

    printf("please input the string you want to write in the file\n");
    scanf("%s",str1);
    len = strlen(str1);
    fwrite(str1,len,1,fp);
    fclose(fp);
*/
    
    fp = fopen(path,"r");
    while(!feof(fp))
    {
        fscanf(fp, "%d", &id);
        
        msgctl(id, IPC_RMID, 0);
    }
    fclose(fp);


    return 0;
}