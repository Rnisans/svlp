
#include "socket_1.h"

//static cycle_buf i_fifo;
//static cycle_buf o_fifo;

volatile cycle_buf i_fifo;
volatile cycle_buf o_fifo;

static void port_init(void){
	FlushBuf(&i_fifo);
	FlushBuf(&o_fifo);
}

// Function to process USB VCP received data into input FIFO
void USB_VCP_DataReceived() {
    uint8_t received_data[64];
    uint32_t received_len = 0;
    
    if (VCP_Received) {
        Read_VCP(received_data, &received_len);
        
        // Put received data into input FIFO
        for (uint32_t i = 0; i < received_len; i++) {
            PutChar(&i_fifo, received_data[i]);
        }
    }
}

// Function to send data from output FIFO to USB VCP
void USB_VCP_DataTransmit() {
    static uint8_t tx_buffer[64];
    uint32_t tx_count = 0;
    
    // Get data from output FIFO
    while (GetLen(&o_fifo) > 0 && tx_count < 64) {
        tx_buffer[tx_count] = GetChar(&o_fifo);
        tx_count++;
    }
    
    // Send data if we have any
    if (tx_count > 0) {
        Write_VCP(tx_buffer, tx_count);
    }
}

Socket::Socket():
    ABstractSocket()
{
}

uint32_t Socket::read(void * const buf, const uint32_t len)
{
	unsigned short size = GetLen(&i_fifo);
	if(len < size) size = len;
	unsigned short i;
	for (i = 0; i < size; i++)
	{
		//__disable_irq();
		(static_cast< char* const>(buf))[i] = GetChar(&i_fifo);
		//__enable_irq();
	}
	return size;
}

uint32_t Socket::skip(uint32_t const )
{
    return 0;
}

bool Socket::is_ready_read()
{
	return GetLen(&i_fifo);
}

void Socket::flush()
{
	//while (USART_GetITStatus(USART1, USART_IT_TC) != SET);
}

uint32_t Socket::write(const void *buf, const uint32_t len)
{
	uint32_t i = 0;
	//__disable_irq();
		for (i = 0; i < len; i++) {
			PutChar(&o_fifo, ((uint8_t*) buf)[i]);
		}
		//__enable_irq();
		if(i != len) while(1);
		return i;
}

bool Socket::open()
{
	port_init();
    return true;
}

bool Socket::close()
{
     return true;
}
