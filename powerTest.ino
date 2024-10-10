
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
  Get_Encoder_Estimates_msg_t last_feedback;
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
void onFeedback(Get_Encoder_Estimates_msg_t& msg, void* user_data) {
  ODriveUserData* odrv_user_data = static_cast<ODriveUserData*>(user_data);
  odrv_user_data->last_feedback = msg;
  odrv_user_data->received_feedback = true;
}

// Called for every message that arrives on the CAN bus
void onCanMessage(const CanMsg& msg) {
  for (auto odrive: odrives) {
    onReceive(msg, *odrive);
  }
}

bool getTorque(Get_Torques_msg_t& msg, uint16_t timeout_ms = 10);
Get_Torques_msg_t torqMsg;


void setup() {
  Serial.begin(115200);

  

  delay(10000);



  // Register callbacks for the heartbeat and encoder feedback messages
  odrv16.onFeedback(onFeedback, &odrv16_user_data);
  odrv16.onStatus(onHeartbeat, &odrv16_user_data);

  odrv19.onFeedback(onFeedback, &odrv19_user_data);
  odrv19.onStatus(onHeartbeat, &odrv19_user_data);

  // Configure and initialize the CAN bus interface. This function depends on
  // your hardware and the CAN stack that you're using.
  if (!setupCan()) {
    while (true); // spin indefinitely
  }

  //while (!odrv0_user_data.received_heartbeat) {
  //  pumpEvents(can_intf);
  //  delay(100);
  //}


  // request bus voltage and current (1sec timeout)
  Get_Bus_Voltage_Current_msg_t vbus;
  if (!odrv16.request(vbus, 1)) {
    //while (true); // spin indefinitely
  }

  odrv19.request(torqMsg, 1);
  

  //while (odrv0_user_data.last_heartbeat.Axis_State != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL) {
    odrv16.clearErrors();
    delay(1);
    odrv16.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
    odrv16.setControllerMode(2,2);

  //}

    odrv19.clearErrors();
    delay(1);
    odrv19.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
    odrv19.setControllerMode(2,2);

  Serial.println("vel estimate (rev/s), Milliseconds Since Start, Toruqe (NM)");

}

void loop() {
  pumpEvents(can_intf); // This is required on some platforms to handle incoming feedback CAN messages

  odrv16.setVelocity(10);//1 newton meters

  odrv19.setVelocity(0.00);//0 newton meters


  // print position and velocity for Serial Plotter
  if (odrv16_user_data.received_feedback) {
    Get_Encoder_Estimates_msg_t feedback = odrv16_user_data.last_feedback;
    odrv16_user_data.received_feedback = false;
    odrv16.request(torqMsg, 1);
    Serial.print(feedback.Vel_Estimate);
    Serial.print(",");
    Serial.print(millis());
    Serial.print(",");
    Serial.print(torqMsg.Torque_Estimate);
    Serial.println(",");

  }
  
}
