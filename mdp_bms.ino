// MDP Harris 2016
// Developed my Maria Karstens
// Fall 2016

#include <Servo.h>
#include <SoftwareSerial.h>

/*------- DEFINE PIN OUTS-------*/ 
#define VOLTAGE_SENSE A1
// #define CURRENT_SENSE A6
#define POT_SENSE_1 A2
#define POT_SENSE_2 A3
#define POT_SENSE_3 A4
#define POT_SENSE_4 A5
#define ENABLE_SWITCH_SENSE A7
#define CMOS_PIN 4
#define DMOS_PIN 2
#define BAT_LED1 10  // indicates charge level of battery pack
#define BAT_LED2 8
#define BAT_LED3 6
#define MOTOR_SOUTH 11
#define MOTOR_WEST 12
#define TEMP_SENSE A8
Servo G1;
Servo G2;

/*------- DEFINE PARAMTER BOUNDARIES-------*/ 
#define MAX_VOLTAGE 7.9 // in V
#define MIN_VOLTAGE 6.3 // in V
//#define MAX_CURRENT 1000 // in mA

/*------- INITIALIZE PARAMTER BOUNDARIES-------*/ 
double voltage = 0.0;
// double current = 0.0;
int top = 0;          // top is defined as PV coincident with spring
int bot = 0;          // bot is opposite to and coincident with the motor
int right = 0;        // right is defined as PV coincident with spring
int left = 0;         // left is opposite top and coincident with the motor
int topBotDiff = 0;
int rightLeftDiff = 0;
int posG1 = 0;
int posG2 = 0;
int voltageDiffComp = 143; //voltage difference comparison (1000 degrees from PR sensor reading / 7 possible rotation angles (15, 10, 5...)
int positionTB = 0;
int positionLR = 0;
int enable_R = 0.0;
int rotate = 0;
int dmos = 0;
int cmos = 0;
//double temp = 0;

/*------- SETUP-------*/ 
void setup() {
    Serial.begin(9600); //baud rate is the rate at which information will pass from Arudino to computer
    
    pinMode(DMOS_PIN, OUTPUT);
    digitalWrite(DMOS_PIN, LOW);
    
    pinMode(CMOS_PIN, OUTPUT);
    digitalWrite(CMOS_PIN, LOW);
    
    pinMode(MOTOR_SOUTH, OUTPUT);
    pinMode(MOTOR_WEST, OUTPUT);
    
    G1.attach(MOTOR_SOUTH);
    G1.write(circleFunc(0)); // default to horizontel position
    G2.attach(MOTOR_WEST);
    G2.write(circleFunc2(3)); // default to horizontel position
}

/*--------------MAIN LOOP--------------*/ 
void loop() {
    read_sensors();
    C_MOS_enable();
    D_MOS_enable();
    battery_LEDs();
    drive_motors();
    output_readings();
    delay(5000);
}

/*--------------OUTPUT READINGS--------------*/ 
void output_readings(void){
    Serial.print(voltage);
    Serial.print(",");
    //Serial.print(current-2.5);
    //Serial.print(",");
    Serial.print(rotate);
    Serial.print(",");
    Serial.print(top);
    Serial.print(",");
    Serial.print(bot);
    Serial.print(",");
    Serial.print(right);
    Serial.print(",");
    Serial.print(left);
    Serial.print(",");
    Serial.print(dmos);
    Serial.print(",");
    Serial.print(cmos);
    Serial.print("\n");
}
/*-------READ ALL SENSE MEASUREMENTS-------*/ 
void read_sensors(void){
  read_bat_voltage();
  //read_current();
  //read_temp();
  read_enable_rotation(); // Rotational Mount Enable Switch
}

/*-------MEASURE BATTERY PACK VOLTAGE-------*/ 
void read_bat_voltage(void){
  double r_top = 98530.0;
  double r_bottom = 98947.0;
  double sum = 0;
  int i = 0;
  for (i = 0;i<5;i++){
      sum +=  analogRead(VOLTAGE_SENSE) * 5.0 / 1023.0 * (r_top + r_bottom) / r_bottom + 0.05;
  }
  voltage = sum / i;
}

/*-------MEASURE CHARGE/DISCHARGE CURRENT FROM THE BATTERY-------*/ 
//void read_current(void){
//    current = analogRead(CURRENT_SENSE) * 5.0 / 1023.0; // mV reading
//}

/*-------ROTATIONAL MOUNT ENABLE SWITCH-------*/ 
void read_enable_rotation(void){
   enable_R = analogRead(ENABLE_SWITCH_SENSE) / 1023.0 * 5.0;
   if (enable_R > 1.0){ // allow rotational mount to activate
      rotate = 1;
   }
   else{
      rotate = 0;
   }
}

//*-------SET BATTEY CHARGE LEVEL INDICATOR LEDS-------*/ 
void battery_LEDs(void){
  if (voltage <= (MIN_VOLTAGE+0.5)){ // battery is near empty
      digitalWrite(BAT_LED3,LOW);
      digitalWrite(BAT_LED2,LOW);
      digitalWrite(BAT_LED1,HIGH);
  }
  else if(voltage <= (MAX_VOLTAGE-1.5)){
      digitalWrite(BAT_LED3,LOW);
      digitalWrite(BAT_LED2,HIGH);
      digitalWrite(BAT_LED1,LOW);
  }
  else {
      digitalWrite(BAT_LED3,HIGH);
      digitalWrite(BAT_LED2,HIGH);
      digitalWrite(BAT_LED1,LOW);
  }
}

//*-------DISCHARGE BIDIRECTIONAL MOSFET-------*/
void D_MOS_enable(void) {
  if (voltage >= MIN_VOLTAGE){
      digitalWrite(DMOS_PIN, HIGH);
      dmos = 1;
  }
  else{
      digitalWrite(DMOS_PIN, LOW);
      dmos = 0;
  }
}

//*-------CHARGE BIDIRECTIONAL MOSFET-------*/
void C_MOS_enable(void){
  if (voltage <= MAX_VOLTAGE){
      digitalWrite(CMOS_PIN, HIGH);
      cmos = 1;
  }
  else{
      digitalWrite(CMOS_PIN, LOW);
      dmos = 0;
  }
}

///*-------MEASURE TEMPERATURE-------*/ 
//void read_temp(void){
//   double resistor_bottom = 0; // change when soldered to board...
//   temp = analogRead(TEMP_SENSE) / 1023 * 5;
//   Serial.print("\nTemperature: ");
//   Serial.print(temp,5);
//   Serial.print("C");
//}
//

/*-------ROTATIONAL MOUNT MOTORS-------*/ 
void drive_motors(void){
    top = analogRead(POT_SENSE_1);
    bot = analogRead(POT_SENSE_3);
    right = analogRead(POT_SENSE_2);
    left = analogRead(POT_SENSE_4);
    
    topBotDiff = top - bot;  //difference between PV voltage readings
    rightLeftDiff = right - left;  // ...
    
    //  Check if reading dff is too high, then recalculate appropriate position and send motor to it
    if(rotate){
        if (abs(topBotDiff) > 60){
          positionTB += topBotDiff/voltageDiffComp;
          if(positionTB > 6 || positionTB < 0) positionTB = 3;
              G1.write(circleFunc(positionTB));
        }
        
        if (abs(rightLeftDiff) > 60){
          positionLR += rightLeftDiff/voltageDiffComp;
          if(positionLR > 6 || positionLR < 0) positionLR = 3;
              G2.write(circleFunc2(positionLR));
        }
    }
 }

/*--------------DRIVE MOTOR1--------------*/ 
int circleFunc(int pos) {
  int angle = 5;  
  if(pos == 0) angle = 30;
  else if(pos == 1) angle = 48; 
  else if(pos == 2) angle = 70;
  else if(pos == 3) angle = 90;
  else if(pos == 4) angle = 110;
  else if(pos == 5) angle = 132;
  else if(pos == 6) angle = 175;
  return angle;
}

/*--------------DRIVE MOTOR2--------------*/
int circleFunc2(int pos) {
  int angle = 5;  
  if(pos == 6) angle = 5;
  else if(pos == 5) angle = 48; 
  else if(pos == 4) angle = 70;
  else if(pos == 3) angle = 90;
  else if(pos == 2) angle = 110;
  else if(pos == 1) angle = 132;
  else if(pos == 0) angle = 175;
  return angle;
}


