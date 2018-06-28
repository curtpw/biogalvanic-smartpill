/*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*
*  KX022_SPI Accelerometer library
*
*  @author  Goran Mahovlić
*  @version 1.0
*  @date    26-May-2017
*
*  Library for "KX022_SPI Accelerometer library" from Kionix a Rohm group
*    http://www.kionix.com/product/KX022_SPI-1020
*
*/


#include "KX022_SPI.h"

KX022_SPI::KX022_SPI(int cs):_spi(SPI),_cs(cs)
{
   // init();
}
 
KX022_SPI::KX022_SPI(SPIClass &spi,int cs):_spi(spi),_cs(cs)
{
    //init();
} 
 
KX022_SPI::~KX022_SPI()
{
}
 
int KX022_SPI::init()
{
	int retVal;
	pinMode(_cs,OUTPUT);
	digitalWrite(_cs,HIGH);

    _spi.begin();
	
	retVal=getByte(KX022_WHO_AM_I);
	if (retVal!=0x14)
	{
		return -1;// Did not find device
	}
	else
	{
		writeTwoBytes(KX022_CNTL1_1,KX022_CNTL1_2);
		writeTwoBytes(KX022_ODCNTL_1,KX022_ODCNTL_2);
		writeTwoBytes(KX022_CNTL3_1,KX022_CNTL3_2);
		writeTwoBytes(KX022_TILT_TIMER_1,KX022_TILT_TIMER_2);
		writeTwoBytes(KX022_CNTL2_1,KX022_CNTL2_2);
		return 0;
	}
}

void KX022_SPI::writeTwoBytes (int address, int data)
{
	digitalWrite(_cs,LOW);
	_buf[0]=address;
	_buf[1]=data;
	_spi.transfer(_buf,2);
	digitalWrite(_cs,HIGH);
}

int KX022_SPI::getByte (int address)
{
	digitalWrite(_cs,LOW);	
	_buf[0]=address | 0x80;// Or-ed with "1" ms bit for read
	_spi.transfer(_buf,2);
	digitalWrite(_cs,HIGH);
	return _buf[1];
}

uint16_t KX022_SPI::getTwoBytes(int address)
{
	digitalWrite(_cs,LOW);	
	_buf[0]=address | 0x80;// Or-ed with "1" ms bit for read
	_spi.transfer(_buf,3);
	digitalWrite(_cs,HIGH);
	return ((int16_t)_buf[2]<<8) | _buf[1];
}

void KX022_SPI::readBytes (int address,uint8_t *buffer,int len)
{
	digitalWrite(_cs,LOW);	
	_buf[0]=address | 0x80;// Or-ed with "1" ms bit for read
	_spi.transfer(_buf,len+1);
	memcpy(buffer,&_buf[1],len);
	digitalWrite(_cs,HIGH);
}

float KX022_SPI::getAccel(int channelNum)
{
//return getTwoBytes(DATA_OUT_BASE + 2*channelNum)/ 16384.0;
  return ((int16_t)((getByte(DATA_OUT_BASE+1 + 2*channelNum)<<8) | (getByte(DATA_OUT_BASE + 2*channelNum)))) / 16384.0;  
}
