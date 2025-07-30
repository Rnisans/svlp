#include "headers/abstract_socket.h"

extern"C"{
#include "cycle_buf.h"
#include "USB-VCP.h"  // Add USB VCP header
}
#include <stdint.h>

// External declarations for FIFO buffers that connect to USB VCP
extern volatile cycle_buf i_fifo;
extern volatile cycle_buf o_fifo;

// Functions to bridge USB VCP with socket FIFO buffers
void USB_VCP_DataReceived();   // Process USB VCP received data into input FIFO
void USB_VCP_DataTransmit();   // Send data from output FIFO to USB VCP

class Socket: public svlp::ABstractSocket
{
public:
	Socket();
public:
    uint32_t read(void * const buf, const uint32_t len);
    uint32_t skip(uint32_t const  len);
    bool is_ready_read();
    void flush();
    uint32_t write(const void *buf, const uint32_t len);

    bool open();
    bool close();
private:
};

