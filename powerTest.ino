#include <Arduino.h>
#include "ODriveCAN.h"

// Documentation for this example can be found here:
// https://docs.odriverobotics.com/v/latest/guides/arduino-can-guide.html


/* Configuration of example sketch -------------------------------------------*/

// CAN bus baudrate. Make sure this matches for every device on the bus
#define CAN_BAUDRATE 250000

/* Board-specific includes ---------------------------------------------------*/

// See https://github.com/tonton81/FlexCAN_T4
// clone https://github.com/tonton81/FlexCAN_T4.git into /src
#include <FlexCAN_T4.h>
#include "ODriveFlexCAN.hpp"
struct ODriveStatus; // hack to prevent teensy compile error

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can_intf;

bool setupCan() {
  can_intf.begin();
  can_intf.setBaudRate(CAN_BAUDRATE);
  can_intf.setMaxMB(16);
  can_intf.enableFIFO();
  can_intf.enableFIFOInterrupt();
  can_intf.onReceive(onCanMessage);
  return true;
}

/* Example sketch ------------------------------------------------------------*/

// Instantiate ODrive objects
ODriveCAN odrv16(wrap_can_intf(can_intf), 16); // Standard CAN message ID
ODriveCAN odrv19(wrap_can_intf(can_intf), 19); // Standard CAN message ID

ODriveCAN* odrives[] = {&odrv16, &odrv19}; // Make sure all ODriveCAN instances are accounted for here

struct ODriveUserData {
  Heartbeat_msg_t last_heartbeat;
  bool received_heartbeat = false;
  Get_Encoder_Estimates_msg_t lastEncoderFeedback;
  bool received_feedback = false;
};

// Keep some application-specific user data for every ODrive.
ODriveUserData odrv16_user_data;
ODriveUserData odrv19_user_data;

// Called every time a Heartbeat message arrives from the ODrive
void onHeartbeat(Heartbeat_msg_t& msg, void* user_data) {
  ODriveUserData* odrv_user_data = static_cast<ODriveUserData*>(user_data);
  odrv_user_data->last_heartbeat = msg;
  odrv_user_data->received_heartbeat = true;
}

// Called every time a feedback message arrives from the ODrive
void onFeedback(Get_Encoder_Estimates_msg_t& encoderMsg, void* user_data) {
  ODriveUserData* odrv_user_data = static_cast<ODriveUserData*>(user_data);
  odrv_user_data->lastEncoderFeedback = encoderMsg;
  odrv_user_data->received_feedback = true;
}

// Called for every message that arrives on the CAN bus
void onCanMessage(const CanMsg& msg) {
  for (auto odrive: odrives) {
    onReceive(msg, *odrive);
  }
}

bool getTorque(Get_Torques_msg_t& msg, uint16_t timeout_ms = 10);
// bool getBusVI(Get_Bus_Voltage_Current_msg_t& msg, uint16_t timeout_ms = 10); //already creatd in can.h
// bool getCurrents(Get_Iq_msg_t& msg, uint16_t timeout_ms = 10); //already created in can.h

Get_Torques_msg_t torqMsg;
Get_Bus_Voltage_Current_msg_t vbusMsg;
Get_Iq_msg_t iqMsg;




void setup() {
  Serial.begin(115200);

  

  delay(10000);



  // Register callbacks for the heartbeat and encoder feedback messages
  odrv16.onFeedback(onFeedback, &odrv16_user_data);
  odrv16.onStatus(onHeartbeat, &odrv16_user_data);

  odrv19.onFeedback(onFeedback, &odrv19_user_data);
  odrv19.onStatus(onHeartbeat, &odrv19_user_data);

  if (!setupCan()) {
    while (true); // spin indefinitely
  }


  // request bus voltage and current (1sec timeout)
  if (!odrv16.request(vbusMsg, 1)) {
    //while (true); // spin indefinitely
  }  

  //while (odrv0_user_data.last_heartbeat.Axis_State != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL) {
    odrv16.clearErrors();
    delay(1);
    odrv16.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
    odrv16.setControllerMode(2,1);

  //}

    odrv19.clearErrors();
    delay(1);
    odrv19.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
    odrv19.setControllerMode(2,1);

  Serial.println("vel estimate (rev/s), Milliseconds Since Start, Toruqe  Target (NM), Torque estimate [nm], VBUS DC Voltage [V], vbuS DC Current [A], Set Current [A], Measured Current [A], nodeID,");

}

void loop() {
  pumpEvents(can_intf); // This is required on some platforms to handle incoming feedback CAN messages
  float velTarget = 10*sin(2*PI/4000*millis());

  odrv16.setVelocity(velTarget);
  odrv19.setVelocity(-velTarget);

  if (odrv16_user_data.received_feedback) 
  {
    Get_Encoder_Estimates_msg_t encoderFeedback = odrv16_user_data.lastEncoderFeedback;
    odrv16_user_data.received_feedback = false;
    odrv16.request(torqMsg, 1);
    odrv16.request(vbusMsg, 1);
    odrv16.request(iqMsg  , 1);
    Serial.print(encoderFeedback.Vel_Estimate);
    Serial.print(",");
    Serial.print(millis());
    Serial.print(",");
    Serial.print(torqMsg.Torque_Target);
    Serial.print(",");
    Serial.print(torqMsg.Torque_Estimate);
    Serial.print(",");
    Serial.print(vbusMsg.Bus_Voltage);
    Serial.print(",");
    Serial.print(vbusMsg.Bus_Current);
    Serial.print(",");
    Serial.print(iqMsg.Iq_Setpoint);
    Serial.print(",");
    Serial.print(iqMsg.Iq_Measured);
    Serial.println(",16,");
  }

  if (odrv19_user_data.received_feedback) 
  {
    Get_Encoder_Estimates_msg_t encoderFeedback = odrv19_user_data.lastEncoderFeedback;
    odrv19_user_data.received_feedback = false;
    odrv19.request(torqMsg, 1);
    odrv19.request(vbusMsg, 1);
    odrv19.request(iqMsg  , 1);
    Serial.print(encoderFeedback.Vel_Estimate);
    Serial.print(",");
    Serial.print(millis());
    Serial.print(",");
    Serial.print(torqMsg.Torque_Target);
    Serial.print(",");
    Serial.print(torqMsg.Torque_Estimate);
    Serial.print(",");
    Serial.print(vbusMsg.Bus_Voltage);
    Serial.print(",");
    Serial.print(vbusMsg.Bus_Current);
    Serial.print(",");
    Serial.print(iqMsg.Iq_Setpoint);
    Serial.print(",");
    Serial.print(iqMsg.Iq_Measured);
    Serial.println(",19,");
  }
  
}
