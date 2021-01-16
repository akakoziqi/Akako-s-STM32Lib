

#include <Si4703.h>
#include "i2c.h"
#include "stm32f1xx_hal_i2c.h"
#include "stm32f1xx_hal_gpio.h"

const char *TAG_SI4703 = "Si4703";

uint8_t si4703_buffer[16]; //定义16个16位寄存器

void powerOn()
{
    si4703_init();
}

void setChannel(int channel)
{
    //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
    //97.3 = 0.2 * Chan + 87.5
    //9.8 / 0.2 = 49
    int newChannel = channel * 10; //973 * 10 = 9730
    newChannel -= 8750;            //9730 - 8750 = 980
    newChannel /= 10;              //980 / 10 = 98

    //These steps come from AN230 page 20 rev 0.5
    readRegisters();
    si4703_buffer[CHANNEL] &= 0xFE00;      //Clear out the channel bits
    si4703_buffer[CHANNEL] |= newChannel;  //Mask in the new channel
    si4703_buffer[CHANNEL] |= (1 << TUNE); //Set the TUNE bit to start
    updateRegisters();

    //delay(60); //Wait 60ms - you can use or skip this delay

    while (_stcIntPin == 1)
    {
    } //Wait for interrupt indicating STC (Seek/Tune Complete)

    readRegisters();
    si4703_buffer[CHANNEL] &= ~(1 << TUNE); //Clear the tune after a tune has completed
    updateRegisters();

    //Wait for the si4703 to clear the STC as well
    while (1)
    {
        readRegisters();
        if ((si4703_buffer[STATUSRSSI] & (1 << STCP)) == 0)
            break; //Tuning complete!
    }
}

int seekUp()
{
    return seek(SEEK_UP);
}

int seekDown()
{
    return seek(SEEK_DOWN);
}

void setVolume(int volume)
{
    readRegisters(); //Read the current register set
    if (volume < 0)
        volume = 0;
    if (volume > 15)
        volume = 15;
    si4703_buffer[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
    si4703_buffer[SYSCONFIG2] |= volume; //Set new volume
    updateRegisters();                      //Update
}

void readRDS(char *buffer, long timeout)
{
//    long endTime = millis() + timeout;
//    boolean completed[] = {false, false, false, false};
//    int completedCount = 0;
//    while (completedCount < 4 && millis() < endTime)
//    {
//      readRegisters();
//      if (si4703_buffer[STATUSRSSI] & (1 << RDSR))
//      {
//        // ls 2 bits of B determine the 4 letter pairs
//        // once we have a full set return
//        // if you get nothing after 20 readings return with empty string
//        uint16_t b = si4703_buffer[RDSB];
//        int index = b & 0x03;
//        if (!completed[index] && b < 500)
//        {
//          completed[index] = true;
//          completedCount++;
//          char Dh = (si4703_buffer[RDSD] & 0xFF00) >> 8;
//          char Dl = (si4703_buffer[RDSD] & 0x00FF);
//          buffer[index * 2] = Dh;
//          buffer[index * 2 + 1] = Dl;
//          // Serial.print(si4703_buffer[RDSD]); Serial.print(" ");
//          // Serial.print(index);Serial.print(" ");
//          // Serial.write(Dh);
//          // Serial.write(Dl);
//          // Serial.println();
//        }
//        delay(40); //Wait for the RDS bit to clear
//      }
//      else
//      {
//        delay(30); //From AN230, using the polling method 40ms should be sufficient amount of time between checks
//      }
//    }
//    if (millis() >= endTime)
//    {
//      buffer[0] = '\0';
//      return;
//    }
//
//    buffer[8] = '\0';




}

//To get the Si4703 inito 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
//The Si4703 will be in an unknown state. RST must be controlled
void si4703_init()
{
    printf("si4703 init start\n");
    printf("si\n");

    GPIO_InitTypeDef sdio_conf;
    printf("1");
    sdio_conf.Mode = GPIO_MODE_OUTPUT_PP;
    printf("2");
    sdio_conf.Pin = _sdioPin;
    sdio_conf.Pull = GPIO_NOPULL;
    sdio_conf.Speed = GPIO_SPEED_FREQ_HIGH;
    printf("sdio init start");

    HAL_GPIO_Init(GPIOB, &sdio_conf);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STC_GPIO_Port, STC_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_DeInit(GPIOB, _sdioPin);

    printf("i2c2 init start");
    MX_I2C2_Init();
    printf("i2c2 init ok");

    readRegisters(); //Read the current register set


    //si4703_buffer[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
    si4703_buffer[0x07] = 0x8100;  //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
    si4703_buffer[0x04] |= 0x2000; //Set bit 14 to high to enable STC Interrupt on GPIO2
    updateRegisters();                //Update

    HAL_Delay(500);; //Wait for clock to settle - from AN230 page 9

    readRegisters();                     //Read the current register set
    si4703_buffer[POWERCFG] = 0x4001; //Enable the IC
    //  si4703_buffer[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute
    si4703_buffer[SYSCONFIG1] |= (1 << RDS); //Enable RDS

    si4703_buffer[SYSCONFIG1] |= (1 << DE);     //50kHz Europe setup
    si4703_buffer[SYSCONFIG2] |= (1 << SPACE0); //100kHz channel spacing for Europe

    si4703_buffer[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
    si4703_buffer[SYSCONFIG2] |= 0x0001; //Set volume to lowest
    updateRegisters();                      //Update

    HAL_Delay(110);; //Max powerup time, from datasheet page 13
}

//Read the entire register control set from 0x00 to 0x0F
void readRegisters()
{
    HAL_I2C_Master_Receive(&hi2c2, SI4703_ADDR<<1, si4703_buffer,16,50 );
}

//Write the current 9 control registers (0x02 to 0x07) to the Si4703
//It's a little weird, you don't write an I2C addres
//The Si4703 assumes you are writing to 0x02 first, then increments
byte updateRegisters()
{
    HAL_I2C_Master_Transmit(&hi2c2, SI4703_ADDR<<1, si4703_buffer,16,50 );
    return 1;
}

//Seeks out the next available station
//Returns the freq if it made it
//Returns zero if failed
int seek(byte seekDirection)
{
    readRegisters();
    //Set seek mode wrap bit
    si4703_buffer[POWERCFG] |= (1 << SKMODE); //Allow wrap
    //si4703_buffer[POWERCFG] &= ~(1<<SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first
    if (seekDirection == SEEK_DOWN)
        si4703_buffer[POWERCFG] &= ~(1 << SEEKUP); //Seek down is the default upon reset
    else
        si4703_buffer[POWERCFG] |= 1 << SEEKUP; //Set the bit to seek up

    si4703_buffer[POWERCFG] |= (1 << SEEK); //Start seek
    updateRegisters();                         //Seeking will now start

    while (_stcIntPin == 1)
    {
    } //Wait for interrupt indicating STC (Seek/Tune complete)

    readRegisters();
    int valueSFBL = si4703_buffer[STATUSRSSI] & (1 << SFBL); //Store the value of SFBL
    si4703_buffer[POWERCFG] &= ~(1 << SEEK);                 //Clear the seek bit after seek has completed
    updateRegisters();

    //Wait for the si4703 to clear the STC as well
    while (1)
    {
        readRegisters();
        if ((si4703_buffer[STATUSRSSI] & (1 << STCP)) == 0)
            break; //Tuning complete!
    }

    if (valueSFBL)
    { //The bit was set indicating we hit a band limit or failed to find a station
        return (0);
    }
    return getChannel();
}

//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
int getChannel()
{
    readRegisters();
    int channel = si4703_buffer[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
    //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
    //X = 0.1 * Chan + 87.5
    channel += 875; //98 + 875 = 973
    return (channel);
}
