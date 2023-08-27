#include <avr/wdt.h>

#define next 34
#define pause 36
int p = 0;

int v = 4; //รอบวัด
int minBPM = 60, maxBPM = 100; //เกณฑ์เก็บค่า

int uvc = 23;
int check_uvc;

#include <Servo.h>
Servo myservo;
int s_off = 145; //องศาservo *ปิด
int s_on = 0; //องศาservo *เปิด
int ds = 5; //รอบนับถอยหลังก่อนฝาปิด
int ds_delay = 700; //ระยะเวลาต่อรอบ = 5รอบ

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 
int lcd_count = 0; //i เลื่อนสเต็ป LCD
int is = 0; //i start lcd

int lcdbpm,lcdspo2 ; //ก้อปข้อมูลค่าเฉลี่ยม bpm กับ spo2 ไปใช้นอกลูป 

int per = 0;

#define irf 22
int v_ir = 0;
int ir_check = 0;
int ir_count = 0;

int relay = 40;

int ledCSN = 12;
int led13 = 13;

//int reset = 12;

//#define Trig_PIN 2 //ดำ
//#define Echo_PIN 3 //น้ำเงิน
const int Trig_PIN = 2;//ดำ
const int Echo_PIN = 3;//น้ำเงิน
int cm = 0;

int t = 0;
int d = 0;
int dd = 0;
int ddd = 0;

int nn = 1;
int ul = 0; //0

int i = 0;
int A, B;
int nA[11],nB[11];

int e_code;
int UltraCheck = 1;




#include <SoftwareSerial.h>
SoftwareSerial MegaSerial(10, 11); // RX | TX

#include <MAX3010x.h>
#include "filters.h"

// Sensor (adjust to your sensor type)
MAX30105 sensor;
const auto kSamplingRate = sensor.SAMPLING_RATE_400SPS;
const float kSamplingFrequency = 400.0;

// Finger Detection Threshold and Cooldown
const unsigned long kFingerThreshold = 10000;
const unsigned int kFingerCooldownMs = 500; //

// Edge Detection Threshold (decrease for MAX30100)
const float kEdgeThreshold = -2000.0;

// Filters
const float kLowPassCutoff = 5.0;
const float kHighPassCutoff = 0.5;

// Averaging
const bool kEnableAveraging = false;
const int kAveragingSamples = 5;
const int kSampleThreshold = 5;

void ArduinoReset()
{

 wdt_enable(WDTO_15MS);
  while(1)
  {

  }
}

void setup() {



start_fn();

//Servo  
  myservo.attach(7);
  myservo.write(s_off);

  pinMode(relay, OUTPUT);
  pinMode(uvc, OUTPUT);
  

//UltraSonic
  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);
//ir
  pinMode(irf, INPUT);


//led  
  pinMode(ledCSN, OUTPUT);
  pinMode(led13, OUTPUT);
//Serial  
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);


  MegaSerial.begin(57600);
  Serial.begin(115200);

  if(sensor.begin() && sensor.setSamplingRate(kSamplingRate))
  {
    Serial.println(); 
    Serial.println("Sensor initialized");
  }
  else
  {
    Serial.println("Sensor not found");  
    
    e_code = 1;
    error_code();

    while(1);
  }

}

// Filter Instances
LowPassFilter low_pass_filter_red(kLowPassCutoff, kSamplingFrequency);
LowPassFilter low_pass_filter_ir(kLowPassCutoff, kSamplingFrequency);
HighPassFilter high_pass_filter(kHighPassCutoff, kSamplingFrequency);
Differentiator differentiator(kSamplingFrequency);
MovingAverageFilter<kAveragingSamples> averager_bpm;
MovingAverageFilter<kAveragingSamples> averager_r;
MovingAverageFilter<kAveragingSamples> averager_spo2;

// Statistic for pulse oximetry
MinMaxAvgStatistic stat_red;
MinMaxAvgStatistic stat_ir;

// R value to SpO2 calibration factors
// See https://www.maximintegrated.com/en/design/technical-documents/app-notes/6/6845.html
float kSpO2_A = 1.5958422;
float kSpO2_B = -34.6596622;
float kSpO2_C = 112.6898759;

// Timestamp of the last heartbeat
long last_heartbeat = 0;

// Timestamp for finger detection
long finger_timestamp = 0;
bool finger_detected = false;

// Last diff to detect zero crossing
float last_diff = NAN;
bool crossed = false;
long crossed_time = 0;

//____________________________________________

void loop()
{
  //  digitalWrite(uvc, 1);delay(500);
   // digitalWrite(uvc, 0);delay(500);
  auto sample = sensor.readSample(1000);//1000
  float current_value_red = sample.red;
  float current_value_ir = sample.ir;

  v_ir = digitalRead(irf);
  
  if(UltraCheck < 10) //แว้บนึงตอนเปิดเครื่อง จะเรียกอัลต้าโซนิคมาเช็คการทำงาน
  {
    UltraSonic_fn2();
  }

    //ลูปเปิด/ปิดฝาเฉพาะกิจ ทำงานเฉพาะเวลาที่ไม่ได้แตะ Hr/spo2
  if(sample.red < kFingerThreshold)
  {            
          if(v_ir == 1) 
          { 
            ir_count ++;delay(500);
            Serial.println(ir_count);
          }
          if(ir_count >= 5)
          {
            myservo.write(s_on);
           // Serial.println(ir_count);
          }
          
          if(ir_count >= 8)
          {
            myservo.write(s_off);
            ir_count = 0;
          }
  }

  if(sample.red > kFingerThreshold)
  { 
    ir_check = 0; //}เคลียร์ตัวแปรของเปิดปิดฝาเฉพาะกิจถ้านิ้วแตะ Hr/spo2
    ir_count = 0; //}

        if(lcd_count == 0) //พอนิ้วแตะ แสดงจอ
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Please wait..");
          lcd_count++ ;
          
        }

    if(millis() - finger_timestamp > kFingerCooldownMs) 
    {
      finger_detected = true;
      
      e_code = 0;
      
    }
  }
  else //นิ้วออก & หลังจากวัดเส็ด
  {  
    do//ล้างค่าตัวแปรและช่องในอาเรย์เท่ากับจำนวนนับตอนที่แตะวัดค่า
    {
      nA[i] = 0;
      nB[i] = 0;
      i--;
    }
    while(i > 0);

    A = 0;
    B = 0;
    i = 0;
    lcd_count = 0;

    differentiator.reset();
    averager_bpm.reset();
    averager_r.reset();
    averager_spo2.reset();
    low_pass_filter_red.reset();
    low_pass_filter_ir.reset();
    high_pass_filter.reset();
    stat_red.reset();
    stat_ir.reset();
    
    finger_detected = false;
    finger_timestamp = millis();
    
  }

  if(finger_detected) //ลูปวัดค่า เมื่อนิ้วแตะ
  {              
    current_value_red = low_pass_filter_red.process(current_value_red);
    current_value_ir = low_pass_filter_ir.process(current_value_ir);

    stat_red.process(current_value_red);
    stat_ir.process(current_value_ir);

    float current_value = high_pass_filter.process(current_value_red);
    float current_diff = differentiator.process(current_value);

    if(!isnan(current_diff) && !isnan(last_diff)) 
    {
      if(last_diff > 0 && current_diff < 0) 
      {
        crossed = true;
        crossed_time = millis();
        
      }
      
      if(current_diff > 0)
      {
        crossed = false;
      }
  
      if(crossed && current_diff < kEdgeThreshold)
      {    
        if(last_heartbeat != 0 && crossed_time - last_heartbeat > 300)
        {
          int bpm = (60000/(crossed_time - last_heartbeat));
          float rred = (stat_red.maximum()-stat_red.minimum())/stat_red.average();
          float rir = (stat_ir.maximum()-stat_ir.minimum())/stat_ir.average();
          float r = rred/rir;
          float spo2 = (kSpO2_A * r * r + kSpO2_B * r + kSpO2_C); //float

          if(bpm >= minBPM && bpm <= maxBPM)
          {
            if( i != v)
            {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Please wait..");
                
              lcd.setCursor((i*(16/v))-2, 1);  // (16/รอบวัด)
              lcd.print("***");

              Serial.print("HR : ");    Serial.print(bpm);Serial.print("  ");
              Serial.print("SpO2 : ");  Serial.print(spo2);Serial.print("  ");

              per = map(i, 0, v, 0, 100);
              lcd.setCursor(12, 0);
              lcd.print(per);
              lcd.setCursor(15, 0);
              lcd.print("%");

//_________________________________________________________________________________________

              nA[i] = bpm;
              A = A + nA[i];

              nB[i] = spo2;
              B = B + nB[i];

              i++;
//_________________________________________________________________________________________

              Serial.print("ir =  ");     Serial.print(v_ir);
              Serial.print("  Ultra =  ");Serial.println(cm);

              Serial.print(" i =  ");     Serial.println(i);
            
            }
          
            else// if(i >= v)
            {
              Serial.println("uvc off");digitalWrite(uvc, 0);
              
              int average_bpm = A / v;
             // int average_bpm2 = averager_bpm.process(bpm);
              //int average_spo2 = B / (v-1); 
              int average_spo2 = averager_spo2.process(spo2);
              int average_r = averager_r.process(r);
              
             
              Serial.println("\n__________________________________");
              Serial.print("Avg bpm: ");  Serial.print(average_bpm);Serial.print("  "); 
              Serial.print("Spo2,%): ");  Serial.println(average_spo2);

              Serial.println("__________________________________");

              while((i >= v)&&(average_bpm <= minBPM && average_bpm > maxBPM))
              {
                i = 0;  //ถ้าครบจำนวนครั้งแล้วค่าต่ำ/สูงกว่าเกณฑ์ก็ไล่ให้ไปวัดมาใหม่ตั้งแต่ต้น
              }

              if((i == v)&&(average_bpm >= minBPM && average_bpm <= maxBPM)&&
                (average_spo2 <= 100))
              {
                Serial.println("\n_______Send_to_Node __--> --> --> \n");

                MegaSerial.print(average_bpm);
                MegaSerial.print(" ");
                MegaSerial.print(average_spo2);
                MegaSerial.print("s");

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Avg. BPM  : ");
                lcd.print(average_bpm);
                lcd.print("  "); 
                lcd.setCursor(0, 1);
                lcd.print("Avg. SpO2 : ");
                lcd.print(average_spo2);
                lcd.print("  ");
                
                lcdbpm = average_bpm;   //} คัดลอกค่าตัวแปรไปใช้นอกลูป
                lcdspo2 = average_spo2; //}

                myservo.write(s_on);//15

                Serial.println("Check loop");
                Serial.println(" ON ON ON ON ON ON ON ON ON ON ON ON");

                digitalWrite(ledCSN, 1);

                sound_next();


                delay(2000);

                nn = 2;
                ul = 1;

                do
                {
                  nA[i] = 0;
                  nB[i] = 0;
                  i--;
                }
                while(i > 0);

               A = 0;
               B = 0;
               i = 0;
              }

            }
    
          }
          // Reset statistic
          stat_red.reset();
          stat_ir.reset();
        }
  
        crossed = false;
        last_heartbeat = crossed_time;
      }
    }

    last_diff = current_diff;
    
  }
  
    if(nn == 2)
    {
      digitalWrite(ledCSN, 0);
      Serial.println(cm);
      Serial.println("Check loop");
      Serial.println(" เตรียมค่า เตรียมค่า เตรียมค่า รียมค่า เตรียมค่า เตรียมค่า");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("get medicine");
      lcd.setCursor(13, 0);
      lcd.print(lcdbpm);
      lcd.setCursor(13, 1);
      lcd.print(lcdspo2);

      lcd.setCursor(0, 1);

      nn = 1;
      
    }

    if((ul == 1)||(ul == 2))
    {  
      UltraSonic_fn2();
      
      if((ul == 1)&&(cm <= 17))
      { 
        //lcd.clear();
        //lcd.setCursor(12, 1);
        lcd.setCursor(0, 1);
        lcd.print("done !!");
        //lcd.print("OK, done !!");
        
        Serial.println(cm);
        Serial.println("Check loop");
        Serial.println("มือวาง มือวาง มือวาง มือวาง มือวาง มือวาง มือวาง ");
        
        ul = 2;
        
        Serial.println(cm);
        delay(1000);
      }
    }
    
    if((ul == 2)&&(cm > 20))
    { ul = 0;
      Serial.println(cm);
      Serial.println("Check loop");
      Serial.println(" OFF OFF OFF OFF OFF OFF OFF OFFOFF OFF OFF");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("All right !!!");
      lcd.setCursor(0, 1);
      lcd.print("close in");
      
      sound_next();

      do
      {
        lcd.setCursor(9, 1);
        lcd.print(ds);
        
        delay(ds_delay);
        ds--;
      }
      while(ds > 0);
      ds = 5;
      //delay(500);

      sound_next();

      MegaSerial.print("2");
      MegaSerial.print("3");
      MegaSerial.print("4");
      MegaSerial.print("s");
      
      
     // delay(1000);
      Serial.println("Node let go");
      myservo.write(s_off);//162
      
      
      Serial.println(cm);
      
      ul = 0;
      
      delay(500);

      p = 2;

      start_fn();
      //ArduinoReset();
    }

}
