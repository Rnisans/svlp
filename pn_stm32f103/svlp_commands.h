#ifndef SVLP_COMMANDS_H
#define SVLP_COMMANDS_H

#include "svlp_lib/headers/svlp_parser.h"
#include "svlp_lib/headers/svlp_writer.h"

// Command codes for your temperature control system
#define CMD_SET_TARGET_TEMP    0x10
#define CMD_GET_CURRENT_TEMP   0x11
#define CMD_GET_STATUS         0x12
#define CMD_SET_PID_PARAMS     0x13
#define CMD_EMERGENCY_STOP     0x14

// Response codes
#define RESP_OK                0x20
#define RESP_ERROR             0x21
#define RESP_TEMP_DATA         0x22
#define RESP_STATUS            0x23

// Structure for setting PID parameters
struct PIDParams {
    float Kp;
    float Ki;
    float Kd;
    float IntMax;
    float IntMin;
};

// Response structures
struct TempResponse {
    float current_temperature;
    float target_temperature;
    int power_output;
    uint32_t timestamp;
};

// Example command handler class
class TemperatureCommandHandler {
public:
    static void handleSetTargetTemp(svlp::SVLP_Writer* writer, const void* params, size_t len) {
        if (len != sizeof(float)) {
            sendErrorResponse(writer, "Invalid parameter size");
            return;
        }
        
        float newTarget = *(const float*)params;
        
        // Update global target temperature (you'll need to make this accessible)
        extern float targetTemperature;
        targetTemperature = newTarget;
        
        // Send confirmation
        sendOKResponse(writer, "Target temperature updated");
    }
    
    static void handleGetCurrentTemp(svlp::SVLP_Writer* writer, const void* params, size_t len) {
        extern float temperature;
        extern float targetTemperature;
        extern int lastPowerOutput; // You'll need to track this
        
        TempResponse response;
        response.current_temperature = temperature;
        response.target_temperature = targetTemperature;
        response.power_output = lastPowerOutput;
        response.timestamp = 0; // You can add timestamp if needed
        
        svlp::svlp_message msg = writer->create_svlp_message(
            0x00,
            RESP_TEMP_DATA,
            response
        );
        
        writer->send_message(msg);
    }
    
    static void handleSetPIDParams(svlp::SVLP_Writer* writer, const void* params, size_t len) {
        if (len != sizeof(PIDParams)) {
            sendErrorResponse(writer, "Invalid PID parameters size");
            return;
        }
        
        const PIDParams* newParams = (const PIDParams*)params;
        
        // Update global PID parameters (you'll need to make these non-const)
        // extern float Kp, Ki, Kd, IntMax, IntMin;
        // Kp = newParams->Kp;
        // Ki = newParams->Ki;
        // Kd = newParams->Kd;
        // IntMax = newParams->IntMax;
        // IntMin = newParams->IntMin;
        
        sendOKResponse(writer, "PID parameters updated");
    }
    
    static void handleEmergencyStop(svlp::SVLP_Writer* writer, const void* params, size_t len) {
        // Emergency stop - turn off heater immediately
        extern float targetTemperature;
        targetTemperature = 0;
        
        // You might want to set a flag to stop PID control
        // extern bool emergencyStop;
        // emergencyStop = true;
        
        sendOKResponse(writer, "Emergency stop activated");
    }

private:
    static void sendOKResponse(svlp::SVLP_Writer* writer, const char* message) {
        svlp::svlp_message msg = writer->create_svlp_message(
            0x00,
            RESP_OK,
            message,
            strlen(message)
        );
        writer->send_message(msg);
    }
    
    static void sendErrorResponse(svlp::SVLP_Writer* writer, const char* error) {
        svlp::svlp_message msg = writer->create_svlp_message(
            0x00,
            RESP_ERROR,
            error,
            strlen(error)
        );
        writer->send_message(msg);
    }
};

// Function to process incoming SVLP commands
inline void processSVLPCommands(svlp::SVLP_Writer* writer) {
    // This would be called in your main loop to check for incoming commands
    // You would need to implement an SVLP parser for incoming messages
    // This is just an example of how command handling would work
}

#endif // SVLP_COMMANDS_H