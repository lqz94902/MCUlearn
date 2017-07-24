#include <reg52.h>
#include <intrins.h>
#include <stdlib.h>
#define uint  unsigned int
#define uchar unsigned char
#define flash 60   										//显示闪烁速度控制
#define secbuf 60											//时钟秒走时速度调试

uchar code disp_sm[]= {0xc0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,
                       0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10
                      };
//共阳段码表-----------"0"  "1"  "2"  "3"  "4"  "5"  "6"  "7"  "8"  "9"

uchar bdata flag;
sbit f300 =flag^0; 				// 300ms到标志位(用于位闪)
sbit flon =flag^1; 				// 闪总控位
sbit flonc=flag^2;

uchar  buf0,buf1=0;
uchar msta,cnt1s,beepcnt=0;
uchar bufs=0;							//初始化数码管选择位
uchar msta,tzsta,clksta,bufs,buf0,buf1,buf2,buf3,buf4,buff0,buff1,buff2,buff3,cnt1s;
uchar bufh0,bufh1,bufm0,bufm1,ledcnt=0;
bit f,ledf;

sbit  CS1=P2^5;
sbit  CS0=P2^3;
sbit  CS2=P2^2;
sbit  CS3=P2^1;
sbit  led=P2^7;
sbit  sound=P2^0;
sbit  poi=P0^7;

uchar bdata key,lastkey,key1,lastkey1;//电平有效、沿有效全局变量
sbit k0=key^5;
sbit k1=key^3;
sbit k2=key^2;
sbit k3=key^1;
sbit k4=key1^5;
sbit k5=key1^3;
sbit k6=key1^2;
sbit k7=key1^1;

sbit lk0=lastkey^5;
sbit lk1=lastkey^3;
sbit lk2=lastkey^2;
sbit lk3=lastkey^1;
sbit lk4=lastkey1^5;
sbit lk5=lastkey1^3;
sbit lk6=lastkey1^2;
sbit lk7=lastkey1^1;

/*子函数声明*/
void keytest();						//按键测试子程序
void keyscan();						//矩阵键盘扫描子函数
void display();						//数码管动态显示子函数
void delay(uint z);				//软件延时子函数
void keysound();					//有效按键音子函数
void work0();							//99秒计时模块
void work1();							//倒计时模块
void work2();							//报警模块
void work3();							//空闲散转模块
void work4();							//二位抽奖机模块
void work5();							//按位抽奖机模块
void work6();							//时钟模块

void main() {
	TH0=0xEC;
	TL0=0x78;								//T0赋初值
	TR0=1;
	TMOD=1;
	msta=0;
	cnt1s=0;
	sound=0;
	delay(200);							//上电提示音
	sound=1;
	while(1) {
		keyscan();
		display();
		while(!TF0);
		TF0=0;
		TH0=0xEC;
		TL0=0x78;							//T0重赋初值
		switch(msta) {
		case 0:
			work0();
			break;
		case 1:
			work1();
			break;
		case 2:
			work2();
			break;
		case 3:
			work3();
			break;
		case 4:
			work4();
			break;
		case 5:
			work5();
			break;
		case 6:
			work6();
			break;
		default:
			break;
		}
	}
}

/*软件延时子函数*/
void delay(uint z) {
	uint x,y;
	for(x=100; x>0; x--)
		for(y=z; y>0; y--);
}

/*动态显示子函数*/
void display() {
	static uchar dispcnt;
	dispcnt++;								//用于位闪
	if(dispcnt>=flash) {
		dispcnt=0;
		f300^=1;
	}
	bufs++;
	if(bufs>=2)
		bufs=0;
	CS0=CS1=1;
	switch(bufs) {
	case 0:
		CS0=0;
		P0=disp_sm[buf0];
		break;
	case 1:
		CS1=0;
		P0=disp_sm[buf1];
		break;
	default:
		break;
	}
	if(flon&&f300) {					//倒计时闪控制
		switch(tzsta) {
		case 0:
			CS0=1;
			break;
		case 1:
			CS1=1;
			break;
		default:
			break;
		}
	}

	if(flonc&&f300) {					//时钟闪控制
		switch(clksta) {
		case 0:
			CS0=1;
			break;
		case 1:
			CS1=1;
			break;
		case 3:
			CS0=1;
			break;
		case 4:
			CS1=1;
			break;
		default:
			break;
		}
	}

	if(msta==6&&clksta==5) {		//work6()时钟模块解决led闪烁亮度不足问题
		ledcnt++;									//led500ms闪烁一次
		if(ledcnt>=100) {
			ledf=!ledf;
			ledcnt=0;
		}
		if(ledf)
			led=0;
		else
			led=1;
	}
}

/*按键扫描子程序*/
void keyscan() {
	uchar r1,r2;									//采用两个寄存器存键值
	static uchar keycnt;
	P2=0x00^0xef;
	_nop_();					//扫描k0-k3
	r1=P2;												//k0-k3键值存入r1
	r1&=0x2e;
	r1^=0x2e;

	P2=0x00^0xbf;
	_nop_();					//扫描k4-k7
	r2=P2;												//k4-k7键值存入r2
	r2&=0x2e;
	r2^=0x2e;

	if(r1) {											//判断寄存器r1
		keycnt++;
		if(keycnt>4)
			key=r1;
		else
			key=lastkey;
	} else if(r2) {									//判断寄存器r2
		keycnt++;
		if(keycnt>4)
			key1=r2;
		else
			key1=lastkey1;
	} else {
		key=0;
		key1=0;
		keycnt=0;
	}
	r1=key;
	key=key&(lastkey^key);
	lastkey=r1;
	r2=key1;
	key1=key1&(lastkey1^key1);
	lastkey1=r2;
}

/*有效按键音子函数*/
void keysound() {
	sound=0;
	delay(50);
	sound=1;
}

/*99秒计时*/
void work0() {
	cnt1s++;
	if(cnt1s>=200) {
		buf0++;
		cnt1s=0;
	}
	if(buf0>=10) {
		buf0=0;
		buf1++;
	}
	if(buf1>=10) {
		buf0=0;
		buf1=0;
	}
	if(k5) {
		buf0=buf1=0;
		tzsta=0;
		keysound();
		msta=1;
	}
	if(k6) {
		buf0=buf1=0;
		keysound();
		msta=4;
	}
	if(k7) {
		bufh0=bufh1=bufm0=bufm1=0;
		buf0=buf1=0;
		msta=6;
		clksta=0;
		keysound();
	}
}

/*————————————————————————————以下为功能模块————————————————————————————*/

/*倒计时模块*/
void work1() {
	static uchar flons,i;
	if(k0) {
		tzsta++;
		if(tzsta>=4) {
			tzsta=0;
		}
	}
	flons++;
	switch(tzsta) {
	case 0: {

		if(flons>=flash) {
			flons=0;
			flon=~flon;
		}
		if(flon==1)
			CS0=1;
		else
			buf0=buff0;
		if(k1)
			buff0++;
		if(buff0>=10)
			buff0=0;
	}
	break;

	case 1: {
		if(flons>=flash) {
			flons=0;
			flon=~flon;
		}
		if(flon==1)
			CS1=1;
		else
			buf1=buff1;
		if(k1)
			buff1++;
		if(buff1==10)
			buff1=0;
	}
	break;

	case 2: {
		i++;
		if(i>=200) {
			buf0--;
			if(buf0==255) {
				buf0=9;
				buf1--;
				if(buf1==255)
					buf1=9;
			}

			if(buf0+buf1==0) {	//计时报警
				tzsta=3;
			}
			i=0;
		}
		if(k4) {							//99秒计时模块
			buf0=buf1=0;
			flon=0;
			msta=0;
			tzsta=0;
			keysound();
		}
		if(k5) {							//倒计时模块
			flon=0;;
			msta=1;
			tzsta=0;
			keysound();
		}
		if(k6) {							//二位抽奖机模块
			flon=0;
			tzsta=0;
			keysound();
			msta=4;
		}

	}
	break;

	case 3: {
		buff0=buff1=0;
		buf0=buf1=0;
		tzsta=0;
		msta=2;
		flon=0;
	}
	}
}

/*报警模块*/
void work2() {
	static uint x=0;
	beepcnt++;
	if(beepcnt>=100) {
		beepcnt=0;
		sound=~sound;
		led=~led;
		delay(30);
		x++;
	} else
		led=1;

	if(x>=3) {
		buf0=buf1=9;
		msta=3;					//跳转空闲模块
		x=0;
	}
}

/*空闲散转*/
void work3() {
	buf0=buf1=0;
	if(k4) {					//99秒计时模块
		msta=0;
		keysound();
	}
	if(k5) {					//倒计时模块
		msta=1;
		tzsta=0;
		keysound();
	}
	if(k6) {					//二位抽奖机模块
		keysound();
		msta=4;
	}
	if(k7) {
		bufh0=bufh1=bufm0=bufm1=0;
		buf0=buf1=0;
		msta=6;
		clksta=0;
		keysound();
	}
}

/*二位抽奖机子函数*/
void work4() {
	uchar i;
	static bit p=0;
	if(k6) {
		p=~p;
		keysound();
	}
	if(p) {

		led=1;
		for(i=0; i<2; i++) {
			buf0=rand()%10;
			buf1=rand()%10;
		}
		delay(10);
	} else {
		led=0;
		if(k4) {
			keysound();
			buf0=buf1=0;
			msta=0;
		}
		if(k5) {
			buf0=buf1=0;
			buff0=buff1=0;
			keysound();
			msta=1;
		}
		if(k2) {
			msta=5;
			keysound();
		}
		if(k7) {
			bufh0=bufh1=bufm0=bufm1=0;
			buf0=buf1=0;
			msta=6;
			clksta=0;
			keysound();
		}
	}
}

void work5() {			//按位抽奖机子函数
	static uchar i;
	static uchar randcnt=0;
	if(k2) {
		randcnt++;
		keysound();
	}
	if(randcnt>=4)
		randcnt=0;
	switch(randcnt) {
	case 0: {
		led=1;
		for(i=0; i<2; i++) {
			buf0=rand()%10;
			buf1=rand()%10;
		}
		delay(10);
	}
	break;
	case 1: {
		led=1;
		for(i=0; i<2; i++) {
			buf1=rand()%10;
		}
		delay(10);
	}
	break;
	case 2: {
		led=0;
	}
	case 3: {
		led=0;
		if(k4) {
			keysound();
			buf0=buf1=0;
			msta=0;
		}
		if(k5) {
			buf0=buf1=0;
			buff0=buff1=0;
			keysound();
			msta=1;
		}
		if(k2) {
			msta=5;
			keysound();
		}
		if(k7) {
			bufh0=bufh1=bufm0=bufm1=0;
			buf0=buf1=0;
			msta=6;
			clksta=0;
			keysound();
		}
		if(k6) {
			keysound();
			randcnt=0;
			msta=4;
		}
	}
	break;
	}
}

void work6() {				//时钟模块
	static uchar flons,sec,lkbuf;
	static uint  clkswitch;
	static bit   c;
	if(k7) {
		clksta++;
		if(clksta>=6) {
			clksta=0;
		}
	}
	flons++;
	switch(clksta) {
	case 0: {
		buf0=bufh0,buf1=bufh1;
		if(flons>=flash) {
			flons=0;
			flonc=~flonc;
		}
		if(flonc==1)
			CS0=1;
		else
			buf0=bufh0;
		if(lk3) {								//按键电平有效实现快速加
			lkbuf++;
			if(lkbuf>=50) {
				bufh0++;
				lkbuf=0;
			}
		}
		if(bufh0>=10)
			bufh0=0;
		if(bufh1==2) {
			if(bufh0>=4) {
				bufh0=0;
			}
		}
	}
	break;

	case 1: {
		if(flons>=flash) {
			flons=0;
			flonc=~flonc;
		}
		if(flonc==1)
			CS1=1;
		else
			buf1=bufh1;
		if(k3)
			bufh1++;
		if(bufh0>=4)
			if(bufh1>=2)
				bufh1=0;
		if(bufh1>=3)
			bufh1=0;
	}
	break;

	case 2: {					//设置切换
		sound=0;
		delay(50);
		sound=1;
		clksta++;
	}

	case 3: {
		buf0=bufm0,buf1=bufm1;
		if(flons>=flash) {
			flons=0;
			flonc=~flonc;
		}
		if(flonc==1)
			CS0=1;
		else
			buf0=bufm0;
		if(lk3) {
			lkbuf++;
			if(lkbuf>=50) {
				bufm0++;
				lkbuf=0;
			}
		}
		if(bufm0>=10)
			bufm0=0;
	}
	break;

	case 4: {
		if(flons>=flash) {
			flons=0;
			flonc=~flonc;
		}
		if(flonc==1)
			CS1=1;
		else
			buf1=bufm1;
		if(lk3) {
			lkbuf++;
			if(lkbuf>=50) {
				bufm1++;
				lkbuf=0;
			}
		}
		if(bufm1>=6)
			bufm1=0;
	}
	break;
	case 5: {
		cnt1s++;
		clkswitch++;
		if(cnt1s>=200) {
			sec++;
			cnt1s=0;
		}
		if(sec>=secbuf) {
			bufm0++;
			sec=0;
			if(bufm0>=10) {
				bufm1++;
				bufm0=0;
				if(bufm1>=6) {
					bufh0++;
					bufm1=0;
					if(bufh1>=2&&bufh0>=4)
						bufm0=bufm1=bufh0=bufh1=0;
					else if(bufh0>=10) {
						bufh1++;
						bufh0=0;
						if(bufh1>=3)
							bufm0=bufm1=bufh0=bufh1=0;
					}
				}
			}
		}
		if(clkswitch>=600) {					//3s切换显示小时、分钟
			c=!c;
			clkswitch=0;
		}
		if(c) {
			buf0=bufh0+10;
			buf1=bufh1;
		} else {
			buf0=bufm0;
			buf1=bufm1;
		}

//			ledcnt++;								//led闪烁
//			if(ledcnt>=100)
//			{
//				ledf=!ledf;
//				ledcnt=0;
//			}
//			if(ledf)
//				led=0;
//			else
//				led=1;

		if(k4) {
			buf0=buf1=0;
			msta=0;
			keysound();
		}
		if(k5) {
			buf0=buf1=0;
			msta=1;
			keysound();
		}
		if(k6) {
			flon=0;
			tzsta=0;
			keysound();
			msta=4;
		}
	}
	}
}
