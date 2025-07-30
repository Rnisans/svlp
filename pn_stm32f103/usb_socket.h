#ifndef USB_SOCKET_H
#define USB_SOCKET_H

extern "C" {
    #include "USB-VCP.h"
}

#include "svlp_lib/headers/abstract_socket.h"

namespace svlp {

class USBSocket : public ABstractSocket
{
public:
    USBSocket() : ABstractSocket(0, 0, raw_socket), m_isOpen(false) {}
    
    virtual ~USBSocket() {}

    // Read data from USB VCP
    virtual uint32_t read(void* const buf, const uint32_t len) override
    {
        if (!buf || len == 0) return 0;
        
        uint32_t bytes_read = 0;
        Read_VCP((uint8_t*)buf, &bytes_read);
        
        // Make sure we don't exceed the requested length
        return (bytes_read > len) ? len : bytes_read;
    }

    // Skip bytes (for USB VCP, we'll implement by reading and discarding)
    virtual uint32_t skip(uint32_t const len) override
    {
        if (len == 0) return 0;
        
        uint8_t temp_buffer[64]; // USB VCP max packet size
        uint32_t total_skipped = 0;
        
        while (total_skipped < len) {
            uint32_t to_skip = (len - total_skipped > 64) ? 64 : (len - total_skipped);
            uint32_t bytes_read = 0;
            Read_VCP(temp_buffer, &bytes_read);
            
            if (bytes_read == 0) break; // No more data available
            
            total_skipped += bytes_read;
        }
        
        return total_skipped;
    }

    // Check if data is ready to read
    virtual bool is_ready_read() override
    {
        return VCP_Received != 0;
    }

    // Flush USB buffer (wait for transmission to complete)
    virtual void flush() override
    {
        Wait_VCP_TX(); // Wait until transmission is complete
    }

    // Write data to USB VCP
    virtual uint32_t write(const void* buf, const uint32_t len) override
    {
        if (!buf || len == 0) return 0;
        
        Write_VCP((uint8_t*)buf, len);
        return len; // USB VCP doesn't return actual bytes written, assume success
    }

    // Open the USB connection (always available for USB VCP)
    virtual bool open() override
    {
        m_isOpen = true;
        return true;
    }

    // Close the USB connection
    virtual bool close() override
    {
        m_isOpen = false;
        return true;
    }

    bool isOpen() const { return m_isOpen; }

private:
    bool m_isOpen;
};

} // namespace svlp

#endif // USB_SOCKET_H