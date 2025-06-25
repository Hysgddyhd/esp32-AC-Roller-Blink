#include <IRremote.hpp>

//ir receiver
int IR_RECEIVE_PIN = 22;


decode_results results;

void setup(){
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

}


void loop() {
  if (IrReceiver.decode()) {
    //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
    IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
    IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
    IrReceiver.resume(); // Enable receiving of the next value
  }
}
