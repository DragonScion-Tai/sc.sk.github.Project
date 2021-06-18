#include "utils.h"
#include <ctype.h> // isalnum

#define LOG_MODULE 0x00

CommandProcess::CommandProcess()
	: m_status(C_PARSE_SYNC_START)
{
	//buff = new char[getRingBufferSize()];

	readPtr = buff;
	writePtr = readPtr;

	bMoreBytes = false;
}

CommandProcess::~CommandProcess()
{
	//if(buff)
	//	delete[] buff;
	//buff = NULL;
}

int CommandProcess::freeSize()
{
	if(writePtr < readPtr)
		return readPtr-writePtr-1;
	else
		return readPtr+getRingBufferSize()-writePtr-1;
}

int CommandProcess::dataSize()
{
	if(readPtr <= writePtr)
		return writePtr-readPtr;
	else
		return writePtr+getRingBufferSize()-readPtr;
}

int CommandProcess::dataArrived()
{
	// buff is full; and more bytes arrived;
	// go on read after consumed
	//bMoreBytes = true;
	return 0;
}

int CommandProcess::append(char c)
{
	//if(freeSize() <= 0)
	//{
	//	FTRACE("###CommandProcess::append() freeSize is 0\r\n");
	//	return -1;
	//}

	*writePtr++ = c;

	if(writePtr >= buff+getRingBufferSize())
		writePtr = buff;

	return 0;
}

int CommandProcess::pop(char& c)
{
	if(dataSize() < 1)
		return -1;

	c = *readPtr++;

	if(readPtr >= buff+getRingBufferSize())
		readPtr = buff;

	return 0;
}

int CommandProcess::pop(short& s)
{
	char c;
	if(dataSize() < 2)
		return -1;

	pop(c);
	s = c << 8;
	pop(c);
	s |= (unsigned char)c;

	return 0;
}

int CommandProcess::pop(int& i)
{
	short s;
	if(dataSize() < 4)
		return -1;

	pop(s);
	i = s;
	i <<= 16;
	pop(s);
	i |= (unsigned short)s;

	return 0;
}

bool CommandProcess::hasNext(const char c, int times)
{
	int i;
	int hits = 0;
	const char* p = readPtr;
	for(i=0; i<dataSize(); i++, p++)
	{
		if(p >= buff + getRingBufferSize())
			p = buff;

		if(*p == '\n' || *p == '\r')
		{
			if(c == '\n' || c == '\r')
				return true;

            // comment out; for case search for '#' but no '#' to '\n'
			//break;
		}

		if(*p == c)
			hits ++;

		if(hits == times)
			return true;
	}

	return false;
}

int CommandProcess::popInteger(int& val)
{
	return -1;
}

int CommandProcess::popString(char* str, int size)
{
	return -1;
}

int CommandProcess::peek(char& c)
{
	if(dataSize() < 1)
		return -1;

	c = *readPtr;

	return 0;
}

int CommandProcess::processEnd()
{
	//if(bMoreBytes && m_pPort)
	//{
	//	bMoreBytes = false;
	//	append(m_pPort->getc());
	//}

	return 0;
}

int CommandProcess::flush()
{
	readPtr = buff;
	writePtr = readPtr;
	return 0;
}

void CommandProcess::recv_interrupt(void)
{
	if(m_pPort)
	{
		if(freeSize())
			append(m_pPort->getc());
		else
		//	dataArrived();
			// drop
			m_pPort->getc();
	}
}

int CommandProcess::sendData(const char* buff, int size)
{
	int i = 0;
	if(!m_pPort || !buff || size <=0)
		return -1;

#if DUMP_COMMAND_PACKET
	FTRACE("###CommandProcess::sendData() length=%d\r\n", size);
	dumpData(buff, size);
#endif

	for(i=0; i<size; )
	{
		if(m_pPort->writeable())
		{
			
			m_pPort->putc(buff[i]);

			i ++;
		}
		else
		{
			//wait some time?
			wait_us(10);
		}
	}

	return 0;
}

int CommandProcess::dumpData()
{
#if DUMP_COMMAND_PACKET
	const char* p = readPtr;
	char data[64];
	int cnt = 0;
	int size = 0;

	FTRACE("###CommandProcess::dumpData() buff: 0x%p readPtr: 0x%p writePtr: 0x%p\r\n", buff, readPtr, writePtr);
	// while(p != writePtr)
	// {
	// 	FTRACE("###Received byte[%d] = 0x%02X  '%c'\r\n", cnt++, *p, isalnum(*p) ? *p : '.');
    //     //FTRACE("###Received byte[%d] = %d  '%c'\r\n", cnt++, *p, isalnum(*p) ? *p : '.');

	// 	p ++;

	// 	if(p >= buff+getRingBufferSize())
	// 		p = buff;
	// }
	FTRACE("-------------------------receive-----------------------------------\n");
	FTRACE("| ");
	while(p != writePtr)
	{
		FTRACE("0x%02X ", *p);
		data[size] = isalnum(*p) ? *p : '.';
		
		p ++;
		size++;

		if(p >= buff+getRingBufferSize())
			p = buff;
	}
	FTRACE("  |\n");
	FTRACE("|  %s  |\n",data);
	FTRACE("--------------------------------------------------------------------\n");

#endif
	return 0;
}

int CommandProcess::dumpData(const char* d, int size)
{
#if DUMP_COMMAND_PACKET
	int i, j;

	FTRACE("      | +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | ASCII\r\n");
	FTRACE(" -----|-------------------------------------------------|-----------------\r\n");
	for(i=0; i<size; i+=16)
	{
		FTRACE(" %04X | ", i);
		for(j=0; (j<16)&&(i+j<size); j++)
		{
			FTRACE("%02X ", d[i+j]);
		}
		for(; j<16; j++)
		{
			FTRACE("   ");
		}
		FTRACE("| ");
		for(j=0; (j<16)&&(i+j<size); j++)
		{
			FTRACE("%c", isalnum(d[i+j]) ? d[i+j] : '.');
		}

		FTRACE("\r\n");
	}
#endif

	return 0;
}
