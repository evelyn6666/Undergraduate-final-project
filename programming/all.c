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
void system_init();					      	//ϵͳ��ʼ��
void displaywendu();
void lcdwrdata(uchar dat);	  			//lcd1602д���ݺ���
void lcdwrcom(uchar cdat);	  			//lcd1602д�����
void s1();
void s2();
void s3();
unsigned char BYTE;
unsigned int WORD;
unsigned int wd; 
unsigned char ge,shi,bai,qian,setge,setshi;   
sbit P20=P2^0;
sbit rs=P1^7;						  	//H���ݣ�L����
sbit lcden=P1^6;						//������ʹ�ܣ�ʹ��������С150ns
sbit key1=P3^4;
sbit key2=P3^5;
sbit key3=P3^6;
bit busy=0;
uint data time;             //�������������ڶ�ʱ
uchar data t0_h,t0_l;       //�洢��ʱ��0�ĳ�ֵ
char TK=100;                //�����������ڱ�����T=TK*10ms
char TC=0;                  //TK�ı���
char SPEC;               //�¶ȸ���ֵ
char IBAND=10;              //���ַ���ֵ
float kp=2.3;                 //����ϵ��
float ti=200;               //����ϵ��
float td=0;                 //΢��ϵ��
float Voltage0;
float YK;                     //�¶�ֵ
char TKMARK,ADMARK;         //����ʱ�̱�־��A/D��ɱ�־
int AAA1,VAA;               //PWM�ߵ�ƽ����
int ADVALUE;                //A/D����ֵ����
int temp,ss;
int cai_flag;
char EK,EK_1;               //��ǰ����һ�ε�ƫ��ֵ
float ZEK;                  //�����ۼӱ���
char AEK,AEK_1;             //ƫ��ı仯��
int UK;                     //��ǰʱ�̵�D/A���

/** AD��ʼ������ **/
void ad_init()
{
	P1ASF=0x01;		          //ѡ��P1.0����Ϊģ�⹦��A/Dʹ��
	ADC_RES = 0x00;		      //���ADת������Ĵ���
	ADC_RESL = 0x00;
	ADC_CONTR = 0x80;	      //��ADC�ϵ�,ת������Ϊ540��ʱ������,
}

/** T0��ʼ������ **/
void T0_init(void)
{
	TMOD |= 0x01;	           //ʹ��ģʽ1��16λ��ʱ����ʹ��"|"���ſ�����ʹ�ö����ʱ��ʱ����Ӱ��		     
	EA=1;                    //���жϴ�
	ET0=1;                   //����жϴ�  
	TH0=(65536-10000)/256;	 //������ֵ��10ms
	TL0=(65536-10000)%256;
	//TR0=1;                   //������ʱ��T0
	IPH = IPH&0xFD;        
	IP=IP|0x02;              //��ʱ��T0�ж�����Ϊ��1���ȼ�
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
/*******LCD��ʼ��*********/
	lcdwrcom(0x38);					//0x38������ʾģʽΪ��16X2 ��ʾ,5X7 ����,8 λ���ݽӿ�
	lcdwrcom(0x0c);					//����ʾ�����˸
	lcdwrcom(0x06);
	lcdwrcom(0x01);
}

void delay20ms(void)   					//��� -0.000000000003us
{
    unsigned char a,b,c;
    for(c=193;c>0;c--)
        for(b=114;b>0;b--)
            for(a=1;a>0;a--);
}
void delay6ms(void)   					//��� -0.018084490741us
{
    unsigned char a,b;
    for(b=194;b>0;b--)
        for(a=84;a>0;a--);
    _nop_();  						//if Keil,require use intrins.h
}
/********����Ϊ��ʾ��������*********/
void lcdwrdata(uchar dat)			//д���ݵ�LCD
{	rs=1;							//д����ʱRS����
	lcden=1;
	P0=dat;
	delay6ms();
	lcden=0;
delay6ms();
}
void lcdwrcom(uchar cdat)			//дָ�����ݵ�LCD  
{	rs=0;							//дָ��ʱRS����
	lcden=1;
    P0=cdat;
delay6ms();				//���Ӵ���ʱ�����²���д��ָ�����д����ʾ����
	lcden=0;
	delay6ms();
}
void displaywendu()
{
   Voltage0=(float)temp*5/256;
   YK=Voltage0*20;
	 wd=YK*100;
	 ge=wd%10+0x30;		                    //�ֱ�ȡ�¶�ʮλ����λ��С�����һλ��С������λ
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

void s1()                        //��һ��������������70��C
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
  	 TR0=1;                    //������ʱ��	
		 if(cai_flag==1)
		   {
				 TR0=0;
				 cai_flag=0;
				 displaywendu();
	       ADC_CONTR = 0x88;                //����adת��
      	 while(!(ADC_CONTR&0x10));	       //�ж�ת���Ƿ����                
         ADC_CONTR = ADC_CONTR&0x80;	     //��ADC_FLAG��ADC_START
				 temp= ADC_RES;				 //��ת�����
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
	      ADC_CONTR = 0x88;                //����adת��
      	while(!(ADC_CONTR&0x10));	       //�ж�ת���Ƿ����                
        ADC_CONTR = ADC_CONTR&0x80;	     //��ADC_FLAG��ADC_START
        temp = ADC_RES;                  //��ת�����
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
	     ADC_CONTR = 0x88;                //����adת��
       while(!(ADC_CONTR&0x10));	       //�ж�ת���Ƿ����                
       ADC_CONTR = ADC_CONTR&0x80;	     //��ADC_FLAG��ADC_START
       temp = ADC_RES;                  //��ת�����
		   displaywendu();  
		 }
		pid();
		if(UK<=128)
		  P20=0;                        //���������̫С����ֵ��ʹ�¶�����
		else
			AAA1=(UK-128)*100/128;            //PWM�ߵ�ƽ����ʱ��
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
	time=10;                  //��ʱ10ms
	t0_h=(65536-1000*time)/256;//���㶨ʱ��0��ֵ
	t0_l=(65536-1000*time)%256;
	t0_l=t0_l+20;             //�������ֵ��װ������Ķ�ʱ���
	TH0=t0_h;                 
	TL0=t0_l;  
	IT1=1;                    //���ش����ж�
	EX1=1;                    //���ⲿ�ж�1
	ET0=1;                    //����ʱ�ж�0
	TR0=1;                    //������ʱ��
	ADVALUE=YK=EK=EK_1=AEK=ZEK=0;
	UK=ADMARK=TKMARK=TC=0;
	AAA1=VAA=TK;
	EA=1;                     //�����ж�	
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
 float K,P,I,D;							//����ϵ��΢��ϵ������ϵ��
 EK=SPEC-YK;                //����ƫ��
 AEK=EK-EK_1;
 if((abs(EK))>IBAND||ti==0)   //�л��ַ���ֵ
	 I=0;
 else
 {
  ZEK=ZEK+EK;               //���������
  if(ZEK>127)
		ZEK=127;
	if(ZEK<-128)
		ZEK=-128;
	I=ZEK*TK;
	I=I/ti;
 }
 P=EK;                      //������
 D=td*AEK;                  //����΢����
 D=D/TK;
 K=kp*(P+I+D);              //���������
 if(K>127)                  //�п������Ƿ�������������ֵ
	K=127;
 if(K<-128)
	K=-128;
 UK=K;
 EK_1=EK;
 UK=UK+128;
}

void Timer0() interrupt 1 using 1
{
	TH0=t0_h;                  //����װ���ֵ
	TL0=t0_l;
	if(TC<TK)
		TC++;                     
	else
	{
		TKMARK=0x01;             //ÿ����һ��
		TC=0x00;
		ADC_CONTR = 0x88;        //����adת��
		cai_flag=1;
	}
	if (ss==3)
	{
		if(VAA!=0)              //ͨ��ʱ��ֵδ���㣬����
		{
			VAA=VAA-1;
			P20=1;
		}
		else
			P20=0;
	}
}