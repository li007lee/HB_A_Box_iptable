//
//  ipcheck.c
//  Mapport
//
//  Created by MoK on 15-2-14.
//  Copyright (c) 2015年 MoK. All rights reserved.
//
#include "my_include.h"
#include "ipcheck.h"

/*该函数能够验证合法的ip地址，ip地址中可以有前导0，也可以有空格*/

int ip_rightful_check(char *inip)
{
    int part1, part2, part3, part4;
    int inaddr;
    char tail = 0;
    int field;

    if(inip == NULL)
    {
        return 0;
    }
    field=sscanf(inip, "%d . %d . %d . %d %c", &part1, &part2, &part3, &part4, &tail);
    if(field < 4 || field > 5)
    {
        TRACE_ERR("expect 4 field,get %d\n", field);
        return 0;
    }
    if(tail != 0)
    {
        TRACE_ERR("ip address mixed with non number\n");
        return 0;
    }
    if( (part1 >= 0 && part1 <= 255) && (part2>=0 && part2<=255) &&  (part3>=0 && part3<=255) &&  (part4>=0 && part4<=255) )
    {
        inaddr = part4<<24 | part3<<16 | part2<<8 | part1;/*转换成网络序*/
        return 1;
    }
    else
    {
        TRACE_ERR("not good ip %d:%d:%d:%d\n",part1,part2,part3,part4);
    }
    return 0;
}


int isvalidstr(char *buf,int *pAddr)
{
    int part1,part2,part3,part4;
    char tail=0;
    int field;
    if(buf==NULL)
    {
        return 0;
    }
    field=sscanf(buf,"%d . %d . %d . %d %c",&part1,&part2,&part3,&part4,&tail);
    if(field<4|| field>5)
    {
        TRACE_ERR("expect 4 field,get %d\n",field);
        return 0;
    }
    if(tail!=0)
    {
        TRACE_ERR("ip address mixed with non number\n");
        return 0;
    }
    if( (part1>=0 && part1<=255) &&  (part2>=0 && part2<=255) &&  (part3>=0 && part3<=255) &&  (part4>=0 && part4<=255) )
    {
        if(pAddr)
            *pAddr= part4<<24 | part3<<16 | part2<<8 | part1;/*转换成网络序*/
        return 1;
    }
    else
    {
        TRACE_ERR("not good ip %d:%d:%d:%d\n",part1,part2,part3,part4);
    }
    return 0;
}

int testip(char *buf,int expect)
{
    int result=0;
    int addr=0;
    result=isvalidstr(buf,&addr);
    if(result==expect)
    {
        TRACE_ERR("OK:valid ip %s,expect %d,get %d\n",buf,expect,result);
        if(expect==1)
        {
            TRACE_ERR("\twe convert %s to 0x%x\n",buf,addr);
        }
    }
    else
    {
        TRACE_ERR("ERROR:valid ip %s,expect %d,get %d\n",buf,expect,result);
    }
    return 0;
}

void checkip_abcd(char *addr)
{
    int ip_addr;
    char *a = addr;

    ip_addr = atoi(a);

    TRACE_ERR("%d\n", ip_addr);

    if((ip_addr >> 7) == 0)
    {
        TRACE_ERR("A\n");
    }
    else if((ip_addr >> 6) == 2)
    {
        TRACE_ERR("B\n");
    }
    else if((ip_addr >> 5) == 6)
    {
        TRACE_ERR("C\n");
    }
    else if((ip_addr >> 4) == 14)
    {
        TRACE_ERR("D\n");
    }
    else
    {
        TRACE_ERR("E\n");
    }

}
//int main (int argc, char const* argv[])
//{
//    /*空ip*/
//    testip(NULL,0);
//    /*正常ip*/
//    testip("10.129.43.244",1);
//    /*带空格的ip*/
//    testip(" 10.129.43.244",1);
//    testip("10 .129 .43.244 ",1);
//    /*带前导0的ip*/
//    testip("010.129.043.244",1);
//    testip("010.0129.043.0244",1);
//    /*在前面带非法字符的ip*/
//    testip("x10.129.43.244",0);
//    /*最后带非法字符的ip*/
//    testip("10.129.43.24y",0);
//    testip("10.129.43.y",0);
//    return 0;
//}
