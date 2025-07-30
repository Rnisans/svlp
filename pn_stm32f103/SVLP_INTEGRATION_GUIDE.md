# SVLP Library Integration Guide

## Overview

This guide explains how to integrate the SVLP (structured messaging protocol) library with your USB-based STM32F103 temperature control system. The integration replaces simple text-based messaging with a structured, robust protocol that supports CRC verification and typed data transmission.

## What Was Changed

### 1. Created USB Socket Implementation (`usb_socket.h`)

A new `USBSocket` class that implements the `ABstractSocket` interface required by SVLP:

```cpp
svlp::USBSocket usbSocket;
```

**Key Features:**
- Wraps your existing USB VCP functions (`Write_VCP`, `Read_VCP`)
- Implements all required abstract methods: `read()`, `write()`, `flush()`, etc.
- Handles USB-specific packet size limitations (64 bytes max)
- Provides proper error handling and flow control

### 2. Updated Main Application (`main.cpp`)

**Before (simple text):**
```cpp
char message[20];
snprintf(message, sizeof(message), "%.2f,%d\r\n", temperature, PID());
SendMsg(message);  // Sent raw text: "25.30,45"
```

**After (structured SVLP):**
```cpp
SendTemperatureData(temperature, targetTemperature, powerOutput);
// Sends structured binary data with CRC verification
```

### 3. Added Structured Data Types

```cpp
struct TemperatureData {
    float current_temperature;
    float target_temperature;
    int power_output;
};
```

## How SVLP Works

### Message Structure
Every SVLP message contains:
1. **Header** - Protocol identification and routing info
2. **Message Code** - Identifies the type of data/command
3. **Data Payload** - Your actual data (structured)
4. **CRC** - Error detection and data integrity verification

### Message Codes Defined
```cpp
#define MSG_CODE_TEMPERATURE_DATA  0x01  // Temperature sensor data
#define MSG_CODE_STATUS           0x02   // Status messages
#define MSG_CODE_ERROR            0x03   // Error notifications
```

## Usage Examples

### 1. Sending Temperature Data
```cpp
// Create and send structured temperature data
SendTemperatureData(25.5f, 30.0f, 75);
```

This sends a binary message containing:
- Current temperature: 25.5°C
- Target temperature: 30.0°C  
- Power output: 75%
- Plus SVLP header and CRC

### 2. Sending Status Messages
```cpp
SendStatusMsg("PID controller active");
```

### 3. Backward Compatibility
Your old `SendMsg()` function still works:
```cpp
SendMsg("Hello World");  // Now uses SVLP internally
```

## Advantages of SVLP Integration

### 1. **Data Integrity**
- CRC verification ensures data isn't corrupted
- Binary encoding is more efficient than text
- Structured data prevents parsing errors

### 2. **Type Safety**
- No more string parsing on receiver side
- Compiler-enforced data structure alignment
- Clear separation between different message types

### 3. **Extensibility**
- Easy to add new message types
- Protocol versioning support
- Command/response pattern support

### 4. **Performance**
- More efficient than text encoding
- Reduced bandwidth usage
- Better real-time performance

## Receiving SVLP Messages (Optional)

If you need bidirectional communication, you can use the example command handlers in `svlp_commands.h`:

```cpp
// Commands your device can receive:
CMD_SET_TARGET_TEMP    // Set new target temperature
CMD_GET_CURRENT_TEMP   // Request current readings
CMD_SET_PID_PARAMS     // Update PID coefficients
CMD_EMERGENCY_STOP     // Emergency shutdown
```

## Protocol Details

### Message Flow
1. **Application** calls `SendTemperatureData()`
2. **SVLP Writer** creates structured message with header
3. **CRC calculation** ensures data integrity
4. **USB Socket** sends binary data via USB VCP
5. **Receiver** gets structured, verified data

### Data Format Comparison

**Old Format (Text):**
```
"25.30,30.00,75\r\n"  // 15 bytes, string parsing required
```

**New Format (SVLP Binary):**
```
[Header][MsgCode][float][float][int][CRC]  // ~20 bytes, direct memory access
```

## Integration Steps Summary

1. ✅ **Created `USBSocket`** - Bridge between USB VCP and SVLP
2. ✅ **Updated `main.cpp`** - Replace text messaging with SVLP calls
3. ✅ **Added data structures** - Defined `TemperatureData` for structured messaging
4. ✅ **Configured SVLP Writer** - Enabled CRC, connected to USB socket
5. ✅ **Preserved compatibility** - Old `SendMsg()` still works

## Compilation Requirements

Make sure your build system includes:
- `svlp_lib/` directory in include path
- All SVLP source files (.cpp files in `svlp_lib/src/`)
- C++ compilation enabled
- Proper linking order (svlp_lib before usb_lib)

## Testing the Integration

1. **Compile** the updated code
2. **Connect** USB to PC
3. **Send temperature data** - Should see structured SVLP messages
4. **Verify CRC** - Messages should have integrity verification
5. **Monitor performance** - Should be more efficient than text mode

## Troubleshooting

### Common Issues:

1. **"socket undefined"** - Fixed by creating `USBSocket` instance
2. **Compilation errors** - Ensure all SVLP headers are included
3. **USB not working** - Check that USB VCP functions are still available
4. **CRC errors** - Verify data structure alignment and endianness

### Debug Tips:

- Start with simple status messages
- Verify USB VCP still works independently  
- Check message sizes don't exceed USB packet limits
- Use debug output to verify SVLP message creation

## Next Steps

1. **Test the basic integration** with current functionality
2. **Add bidirectional communication** if needed (using `svlp_commands.h`)
3. **Optimize message frequency** for your specific requirements
4. **Add error handling** for USB disconnection scenarios
5. **Implement message acknowledgment** if critical reliability is needed

The integration maintains all your existing functionality while providing a robust, extensible messaging foundation for future enhancements.