#include<reg51.h>
#define uint unsigned int 
#define uchar unsigned char
#define Data P0
sbit lcdrs=P2^4;
sbit lcdrw=P2^5;
sbit lcden=P2^6;

sbit DQ=P1^3;
uint value;

uchar code table3[17]="  Temperature:  ";
bit fg=0;
void delay(uint n)
{  uint x,y; 
   for(x=n;x>0;x--) 
       for(y=110;y>0;y--); 
} 

void write_com(uchar com)
{        
    delay(5);
    lcden=0;
        lcdrs=0;
        lcdrw=0;                        //rs=0时输入指令
        delay(5);
        lcden=1;                        //en=1时读取信息
        Data=com;
        lcden=0;                        //1 -> 0 执行指令
}


void write_dat(uchar dat)
{
    delay(5);
        lcden=0;
        lcdrs=1;
        lcdrw=0;                         //rs=1时输入数据
        delay(5);
        lcden=1;
        Data=dat;
        lcden=0;
        lcdrs=0;
}

void init_lcd()
{
        lcden=0;
        lcdrw=0;
        write_com(0x38);          //8位数据，双列，5*7字形
        write_com(0x0c);         //开启显示屏，关光标，光标不闪烁
        write_com(0x06);         //显示地址递增，即写一个数据后，显示位置右移一位
        write_com(0x01);
}

void delay_us(uchar t)//微妙延时
{
        while(t--);
}
void init_ds18b20()        //数据初始化
{
        DQ=1;
        delay_us(4);
        DQ=0;
        delay_us(80);
        DQ=1;
        delay_us(200);
}
void write_ds18b20(uchar dat) //写数据
{
        uchar i;
        for(i=0;i<8;i++)
        {
                DQ=0;                
                DQ=dat&0x01;
                delay_us(15);
                DQ=1;
                dat=dat>>1;
        }
        delay_us(10);
}
uchar read_ds18b20()  //读数据
{
        uchar i=0,readat=0;
        for(i=0;i<8;i++)
        {
                DQ=0;
                readat=readat>>1;      //八位全部都是0，向右移一位后，仍然全是0
                DQ=1;
                if(DQ)                //如果DQ为1，readat进行或运算，如果为0，则子语句不执行，直接for循环此时采集的数据是0
                        readat=readat|0x80;//此时最高位为1         ，然后再进行for循环 最高位成为第七位，依次往复
                delay_us(10);
        }
        return readat;
}

uchar change_ds18b20()
{
        uchar tl,th;
        init_ds18b20();
        write_ds18b20(0xcc);
        write_ds18b20(0x44);
        delay_us(200);
        init_ds18b20();
        write_ds18b20(0xcc);
        write_ds18b20(0xbe);
        tl=read_ds18b20();
        th=read_ds18b20();
        value=th;
        value=value<<8;
        value=value|tl;
        if(th < 0x80)
        {        fg=0;  }
        
        if(th>= 0x80)
        {        fg=1;
                 
                value=~value+1;
        }
                value=value*(0.0625*10);
            return value;       
}

void display_lcd1602()
{         
         uchar i;
                 write_com(0x80);
                 for(i=0;i<16;i++)
                 {
                         write_dat(table3[i]);
                        delay(3);
                 }
                 write_com(0xc0);
                 if(fg==1)
                 {
                         write_dat(0xb0);
                 }
                 if(fg==0)
                 {
                         write_dat(0x20);
                 }
                 write_dat(value/100+0x30);
                 write_dat(value%100/10+0x30);
                 write_dat(0x2e);
                 write_dat(value%10+0x30);
                 write_dat(0xdf);
                 write_dat(0x43);
}

void main()
{
        init_lcd();

        while(1)
        {
                change_ds18b20();
                display_lcd1602();                            
        }
}
