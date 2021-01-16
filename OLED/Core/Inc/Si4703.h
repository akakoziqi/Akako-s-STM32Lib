#ifndef __SI4703_H__
#define __SI4703_H__

#include "main.h"
#include <stdio.h>

#define _resetPin
#define _sdioPin GPIO_PIN_9
#define _sclkPin
#define _stcIntPin STC_Pin

typedef unsigned char byte;

extern uint8_t si4703_buffer[16]; //定义16个16位寄存器

void powerOn();                    // call in setup
void setChannel(int channel);    // 3 digit channel number
int seekUp();                    // returns the tuned channel or 0
int seekDown();

void setVolume(int volume);    // 0 to 15
void readRDS(char *message, long timeout);
// message should be at least 9 chars
// result will be null terminated
// timeout in milliseconds

void si4703_init();

void readRegisters();

byte updateRegisters();

int seek(byte seekDirection);

int getChannel();



static const uint8_t error = 0;//传输失败标志
static const uint8_t ok = 1;//传输成功标志

static const uint16_t SI4703_ADDR = 0x10; //Si4703的I2C地址0b._001.0000

static const uint8_t SEEK_DOWN = 0; //搜台方向向下
static const uint8_t SEEK_UP = 1;   //搜台方向向上

//下面几个分别代表Si4703的常用寄存器
static const uint8_t DEVICEID = 0x00;   //存储硬件的制造商编号和零件编号
static const uint8_t CHIPID = 0x01;     //存储芯片版本和固件版本信息
static const uint8_t POWERCFG = 0x02;   //硬件启动（初始化）配置
static const uint8_t CHANNEL = 0x03;    //调谐配置（启动调谐位和目标调谐频道位[9:0]）
static const uint8_t SYSCONFIG1 = 0x04; //系统配置
static const uint8_t SYSCONFIG2 = 0x05; //系统配置
static const uint8_t SYSCONFIG3 = 0x06; //系统配置，设置搜台信号质量用
static const uint8_t BOOTCONFIG = 0x09; //启动模式配置
static const uint8_t STATUSRSSI = 0x0A; //标识硬件当前状态（RDS数据就绪否/调谐完成否/RDSA误码率等）
static const uint8_t READCHAN = 0x0B;   //存储RDSB/RDSC/RDSD块的出错情况和当前电台频率

//接下来的四块是RDS读取过程存储的数据
static const uint8_t RDSA = 0x0C;
static const uint8_t RDSB = 0x0D;
static const uint8_t RDSC = 0x0E;
static const uint8_t RDSD = 0x0F;

//------以上是寄存器标识，以下为各存储器中要用到的重要位-------


//POWERCFG-寄存器 0x02中的重要位
static const uint8_t DSMUTE = 15; //软禁音位
static const uint8_t DMUTE = 14;  //禁用静音位
static const uint8_t MONO = 13;   //设置声道模式；=0立体声（默认）；=1单声道
static const uint8_t RDSMODE = 11;
static const uint8_t SKMODE = 10; //查找模式位：=0代表搜台到达一个频率范围边界时跳转到另一边界继续按同一个方向搜台；=1代表当搜台到了一个频率边界后就停止搜台
static const uint8_t SEEKUP = 9;  //搜台方向：=0是向下搜台；=1是向上搜台
static const uint8_t SEEK = 8;    //启动搜台位：=0代表不搜索；=1代表按所设置搜台方向搜索
static const uint8_t PWRDISABLE = 6;
static const uint8_t PWRENABLE = 0;

//CHANNEL-寄存器 0x03中的重要位
static const uint8_t TUNE = 15; //启动调谐位：=0不调谐；=1启动调谐

//SYSCONFIG1-寄存器 0x04中的重要位
static const uint8_t RDSIEN = 15;
static const uint8_t STCIEN = 14;
static const uint8_t RDS = 12; //允许RDS位：=0不允许；=1允许
static const uint8_t DE = 11;  //去加重位：=0去加重时间常数设置为75μs；=1时间常数设置为50μs
static const uint8_t AGCD = 10;

//SYSCONFIG2-寄存器 0x05中的重要位
static const uint32_t BAND0 = 6;
static const uint32_t BAND1 = 7;

static const uint8_t SPACE1 = 5; //SPACE1和SPACE0组成信道间隔位[1:0]：=00间隔为200kHz；=01间隔为100kHz；=10间隔为50kHz
static const uint8_t SPACE0 = 4;

//SYSCONFIG2-寄存器 0x06中的重要位
static const uint8_t SMUTER0 = 15;
static const uint8_t SMUTER1 = 14;

static const uint8_t SMUTEA0 = 13;
static const uint8_t SMUTEA1 = 12;

static const uint8_t SKSNR0 = 7;
static const uint8_t SKSNR1 = 6;
static const uint8_t SKSNR2 = 5;
static const uint8_t SKSNR3 = 4;

static const uint8_t SKCNT0 = 3;
static const uint8_t SKCNT1 = 2;
static const uint8_t SKCNT2 = 1;
static const uint8_t SKCNT3 = 0;

//STATUSRSSI-寄存器 0x0A中的重要位
static const uint8_t RDSR = 15; //RDS数据组就绪位：=0未就绪；=1代表已就绪
static const uint8_t STCP = 14;  //搜台/调谐完成位：=0未完成；=1完成
static const uint8_t SFBL = 13; //搜台失败/到达频率边界位：=0表示搜台成功；=1搜台失败



#endif
