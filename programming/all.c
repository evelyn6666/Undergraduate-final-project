#include"STC12C5A60S2.h"
#include"intrins.h"
#include"string.h"
#include "absacc.h"
#include "math.h"
#define FOSC 11059200L
#define BAUD 9600
#define uchar unsigned char 
#define uint unsigned int
void pid(void);
void delay20ms(void);
void delay6ms(void);
void system_init();					      	//系统初始化
void displaywendu();
void lcdwrdata(uchar dat);	  			//lcd1602写数据函数
void lcdwrcom(uchar cdat);	  			//lcd1602写命令函数
void s1();
void s2();
void s3();
unsigned char BYTE;
unsigned int WORD;
unsigned int wd; 
unsigned char ge,shi,bai,qian,setge,setshi;   
sbit P20=P2^0;
sbit rs=P1^7;						  	//H数据，L命令
sbit lcden=P1^6;						//高脉冲使能，使能脉宽最小150ns
sbit key1=P3^4;
sbit key2=P3^5;
sbit key3=P3^6;
bit busy=0;
uint data time;             //声明变量，用于定时
uchar data t0_h,t0_l;       //存储定时器0的初值
char TK=100;                //声明采样周期变量，T=TK*10ms
char TC=0;                  //TK的变量
char SPEC;               //温度给定值
char IBAND=10;              //积分分离值
float kp=2.3;                 //比例系数
float ti=200;               //积分系数
float td=0;                 //微分系数
float Voltage0;
float YK;                     //温度值
char TKMARK,ADMARK;         //采样时刻标志，A/D完成标志
int AAA1,VAA;               //PWM高电平变量
int ADVALUE;                //A/D采样值变量
int temp,ss;
int cai_flag;
char EK,EK_1;               //当前和上一次的偏差值
float ZEK;                  //积分累加变量
char AEK,AEK_1;             //偏差的变化量
int UK;                     //当前时刻的D/A输出

/** AD初始化程序 **/
void ad_init()
{
	P1ASF=0x01;		          //选择P1.0口作为模拟功能A/D使用
	ADC_RES = 0x00;		      //清除AD转换结果寄存器
	ADC_RESL = 0x00;
	ADC_CONTR = 0x80;	      //给ADC上电,转换周期为540个时钟周期,
}

/** T0初始化程序 **/
void T0_init(void)
{
	TMOD |= 0x01;	           //使用模式1，16位定时器，使用"|"符号可以在使用多个定时器时不受影响		     
	EA=1;                    //总中断打开
	ET0=1;                   //溢出中断打开  
	TH0=(65536-10000)/256;	 //给定初值，10ms
	TL0=(65536-10000)%256;
	//TR0=1;                   //启动定时器T0
	IPH = IPH&0xFD;        
	IP=IP|0x02;              //定时器T0中断设置为第1优先级
}

 void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) 
 { 
  if (y == 0) 
 	{     
	 lcdwrcom(0x80 + x);    
 	}
  else 
 	{      
 	 lcdwrcom(0xC0 + x);     
 	}        
 while (*s) 
 	{     
   lcdwrcom( *s);     
   s ++;     
 	}
}


void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data) 
{     
 if (y == 0) 
 {     
  lcdwrcom(0x80 + x);     
 }    
 else 
 {     
 	lcdwrcom(0xC0 + x);     
 }        
lcdwrdata( Data);  
}

void system_init()
{	
/*******LCD初始化*********/
	lcdwrcom(0x38);					//0x38设置显示模式为：16X2 显示,5X7 点阵,8 位数据接口
	lcdwrcom(0x0c);					//打开显示光标闪烁
	lcdwrcom(0x06);
	lcdwrcom(0x01);
}

void delay20ms(void)   					//误差 -0.000000000003us
{
    unsigned char a,b,c;
    for(c=193;c>0;c--)
        for(b=114;b>0;b--)
            for(a=1;a>0;a--);
}
void delay6ms(void)   					//误差 -0.018084490741us
{
    unsigned char a,b;
    for(b=194;b>0;b--)
        for(a=84;a>0;a--);
    _nop_();  						//if Keil,require use intrins.h
}
/********以下为显示函数定义*********/
void lcdwrdata(uchar dat)			//写数据到LCD
{	rs=1;							//写数据时RS拉高
	lcden=1;
	P0=dat;
	delay6ms();
	lcden=0;
delay6ms();
}
void lcdwrcom(uchar cdat)			//写指令数据到LCD  
{	rs=0;							//写指令时RS拉低
	lcden=1;
    P0=cdat;
delay6ms();				//不加此延时将导致不能写入指令，但能写入显示数据
	lcden=0;
	delay6ms();
}
void displaywendu()
{
   Voltage0=(float)temp*5/256;
   YK=Voltage0*20;
	 wd=YK*100;
	 ge=wd%10+0x30;		                    //分别取温度十位、个位、小数点后一位、小数点后二位
	 wd=wd/10;
	 shi=wd%10+0x30;
	 wd=wd/10;
	 bai=wd%10+0x30;
	 wd=wd/10;
   qian=wd%10+0x30;	
  LCD_Write_Char(4,0,qian);
  LCD_Write_Char(5,0,bai);	
	LCD_Write_Char(6,0,'.');
	LCD_Write_Char(7,0,shi);
	LCD_Write_Char(8,0,ge);
}	 	 

void s1()                        //第一个按键：保温在70°C
{
	 ss=1;
	 P20=1;
	 SPEC=70;
	 setge=SPEC%10+0x30;
   SPEC=SPEC/10;
   setshi=SPEC%10+0x30;
   LCD_Write_Char(1,0,'P'); 
	 LCD_Write_Char(2,0,'V'); 
   LCD_Write_Char(3,0,':'); 
   LCD_Write_Char(9,0,0xDF);
   LCD_Write_Char(10,0,'C'); 
	 LCD_Write_Char(1,1,'S'); 
	 LCD_Write_Char(2,1,'V');
	 LCD_Write_Char(3,1,':');
	 LCD_Write_Char(4,1,setshi); 
	 LCD_Write_Char(5,1,setge); 
	 LCD_Write_Char(6,1,0xDF); 
	 LCD_Write_Char(7,1,'C');
   while(1)
	  {
  	 TR0=1;                    //启动定时器	
		 if(cai_flag==1)
		   {
				 TR0=0;
				 cai_flag=0;
				 displaywendu();
	       ADC_CONTR = 0x88;                //启动ad转换
      	 while(!(ADC_CONTR&0x10));	       //判断转换是否结束                
         ADC_CONTR = ADC_CONTR&0x80;	     //清ADC_FLAG与ADC_START
				 temp= ADC_RES;				 //读转换结果
		   }
		 if(YK<70)
		  	P20=1;
	   if(YK>70)
			  P20=0;
     }		 
}
void s2()
{
	 ss=2;	
   SPEC=99;
   setge=SPEC%10+0x30;
   SPEC=SPEC/10;
	 setshi=SPEC%10+0x30;
   LCD_Write_Char(1,0,'P'); 
	 LCD_Write_Char(2,0,'V'); 
   LCD_Write_Char(3,0,':'); 
   LCD_Write_Char(9,0,0xDF);
   LCD_Write_Char(10,0,'C'); 
	 LCD_Write_Char(1,1,'S'); 
	 LCD_Write_Char(2,1,'V');
	 LCD_Write_Char(3,1,':');
	 LCD_Write_Char(4,1,setshi); 
	 LCD_Write_Char(5,1,setge); 
	 LCD_Write_Char(6,1,0xDF); 
	 LCD_Write_Char(7,1,'C');
	 while(1)
	 {		 
		 P20=1;
		 TR0=1;
	   if(cai_flag==1)
		   {
		  	cai_flag=0;
	      ADC_CONTR = 0x88;                //启动ad转换
      	while(!(ADC_CONTR&0x10));	       //判断转换是否结束                
        ADC_CONTR = ADC_CONTR&0x80;	     //清ADC_FLAG与ADC_START
        temp = ADC_RES;                  //读转换结果
		    displaywendu();  
	   	 }
		 if(YK>99)
		 {
			 P20=0;
			 break;
		 }
   }
}
void s3()
{
	ss=3;
	SPEC=50;
	setge=SPEC%10+0x30;
  SPEC=SPEC/10;
  setshi=SPEC%10+0x30;
   LCD_Write_Char(1,0,'P'); 
	 LCD_Write_Char(2,0,'V'); 
   LCD_Write_Char(3,0,':'); 
   LCD_Write_Char(9,0,0xDF);
   LCD_Write_Char(10,0,'C'); 
	 LCD_Write_Char(1,1,'S'); 
	 LCD_Write_Char(2,1,'V');
	 LCD_Write_Char(3,1,':');
	 LCD_Write_Char(4,1,setshi); 
	 LCD_Write_Char(5,1,setge); 
	 LCD_Write_Char(6,1,0xDF); 
	 LCD_Write_Char(7,1,'C');
	while(1)
	 {
		if(cai_flag==1)
		 {
			 cai_flag=0;
	     ADC_CONTR = 0x88;                //启动ad转换
       while(!(ADC_CONTR&0x10));	       //判断转换是否结束                
       ADC_CONTR = ADC_CONTR&0x80;	     //清ADC_FLAG与ADC_START
       temp = ADC_RES;                  //读转换结果
		   displaywendu();  
		 }
		pid();
		if(UK<=128)
		  P20=0;                        //如果控制量太小，此值能使温度启动
		else
			AAA1=(UK-128)*100/128;            //PWM高电平持续时间
		VAA=AAA1;
	 }	  	
}

void main()
{
	P20=0;
	T0_init();
  ad_init();
	lcden=0;
	system_init();
	TMOD=0X01;
	time=10;                  //定时10ms
	t0_h=(65536-1000*time)/256;//计算定时器0初值
	t0_l=(65536-1000*time)%256;
	t0_l=t0_l+20;             //修正因初值重装而引起的定时误差
	TH0=t0_h;                 
	TL0=t0_l;  
	IT1=1;                    //边沿触发中断
	EX1=1;                    //开外部中断1
	ET0=1;                    //开定时中断0
	TR0=1;                    //启动定时器
	ADVALUE=YK=EK=EK_1=AEK=ZEK=0;
	UK=ADMARK=TKMARK=TC=0;
	AAA1=VAA=TK;
	EA=1;                     //开总中断	
	while(1)
	{
	if(!key1)
	  s1();
	if(!key2)
	  s2();
	if(!key3)
	  s3(); 		 
  }
}
void pid()
{
 float K,P,I,D;							//比例系数微分系数积分系数
 EK=SPEC-YK;                //计算偏差
 AEK=EK-EK_1;
 if((abs(EK))>IBAND||ti==0)   //判积分分离值
	 I=0;
 else
 {
  ZEK=ZEK+EK;               //计算积分项
  if(ZEK>127)
		ZEK=127;
	if(ZEK<-128)
		ZEK=-128;
	I=ZEK*TK;
	I=I/ti;
 }
 P=EK;                      //比例项
 D=td*AEK;                  //计算微分项
 D=D/TK;
 K=kp*(P+I+D);              //计算比例项
 if(K>127)                  //判控制量是否溢出，溢出赋极值
	K=127;
 if(K<-128)
	K=-128;
 UK=K;
 EK_1=EK;
 UK=UK+128;
}

void Timer0() interrupt 1 using 1
{
	TH0=t0_h;                  //重新装入初值
	TL0=t0_l;
	if(TC<TK)
		TC++;                     
	else
	{
		TKMARK=0x01;             //每秒做一次
		TC=0x00;
		ADC_CONTR = 0x88;        //启动ad转换
		cai_flag=1;
	}
	if (ss==3)
	{
		if(VAA!=0)              //通电时间值未到零，升温
		{
			VAA=VAA-1;
			P20=1;
		}
		else
			P20=0;
	}
}