  
void sound_next()
{   
    pinMode(next, OUTPUT);
    digitalWrite(next, 0);
    delay(100);
    digitalWrite(next, 1);
    pinMode(next, INPUT);
}

void sound_pause()
{
    pinMode(pause, OUTPUT);
    digitalWrite(pause, 0);
    delay(100);
    digitalWrite(pause, 1);
    pinMode(pause, INPUT);
}

void start_fn()
{
  int apin = A15;
  float val = 0;
  float volt = 0;
  int perVolt = 0;

  pinMode(apin, INPUT);
  
  Serial.begin(115200);
  Serial.println("_____________\n Start point \n-------------");

  lcd.begin();
  //lcd.noBacklight();
  lcd.backlight();
  do
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
    lcd.setCursor(is, 1);
    lcd.print("*");
    is ++; delay(188);  //125 = 2 วิ, 188 = 3 วิ, 250 = 4 วิ, | วิ/16

  }while(is <= 16);
  is = 0; 

  sound_pause();

  digitalWrite(relay, 1);// temp on.
  digitalWrite(uvc, 1);Serial.println("uvc on");
do
{ val = 0;
  volt = 0; 
  val = analogRead(apin);
  volt = map(val, 230, 980, 327, 420);
  volt = volt/100;
//volt = val*(4.2/930.0);//950

  if(volt >= 4.20){ perVolt = 100;} //100
  if((volt <= 4.15)&&(volt > 4.11)){ perVolt = 95;} //95
  if((volt <= 4.11)&&(volt > 4.08)){ perVolt = 90;} //90
  if((volt <= 4.08)&&(volt > 4.02)){ perVolt = 85;} //85
  if((volt <= 4.02)&&(volt > 3.98)){ perVolt = 80;} //80
  if((volt <= 3.98)&&(volt > 3.95)){ perVolt = 75;} //75
  if((volt <= 3.95)&&(volt > 3.91)){ perVolt = 70;} //70
  if((volt <= 3.91)&&(volt > 3.87)){ perVolt = 65;} //65
  if((volt <= 3.87)&&(volt > 3.85)){ perVolt = 60;} //60
  if((volt <= 3.85)&&(volt > 3.84)){ perVolt = 55;} //55
  if((volt <= 3.84)&&(volt > 3.82)){ perVolt = 50;} //50
  if((volt <= 3.82)&&(volt > 3.80)){ perVolt = 45;} //45
  if((volt <= 3.80)&&(volt > 3.79)){ perVolt = 40;} //40
  if((volt <= 3.79)&&(volt > 3.77)){ perVolt = 35;} //35
  if((volt <= 3.77)&&(volt > 3.75)){ perVolt = 60;} //30
  if((volt <= 3.75)&&(volt > 3.73)){ perVolt = 25;} //25
  if((volt <= 3.73)&&(volt > 3.81)){ perVolt = 50;} //20
  if((volt <= 3.71)&&(volt > 3.69)){ perVolt = 15;} //15
  if((volt <= 3.69)&&(volt > 3.61)){ perVolt = 10;} //10
  if((volt <= 3.61)&&(volt > 3.27)){ perVolt = 5.0;} //5
delay(1);
Serial.println("wait cal. Volt");
}while(perVolt == 0);

  Serial.print("Volt :");Serial.println(volt);
  Serial.print("perVolt :");Serial.println(perVolt);

  lcd.setCursor(0, 0);
  lcd.print("Healh Box [");

  if(perVolt < 10){lcd.setCursor(13, 0);}
  if((perVolt >= 10)&&(perVolt < 100)){lcd.setCursor(12, 0);}
  if(perVolt == 100){lcd.setCursor(11, 0);}
  lcd.print(perVolt);
  lcd.setCursor(14, 0);lcd.print("%]");
  lcd.setCursor(0, 1);
  lcd.print("touch sensor--->");
  lcd.home();
  

}


void UltraSonic_fn()
{
    static unsigned long time1 = micros();
    static unsigned long time2 = micros();

    if(t==0)
    {
      digitalRead(Echo_PIN);
      digitalWrite(Trig_PIN, LOW);
    }
    if(((micros()-time1)>5)&&(t==0))
    {
      time1 = micros();
      d = d+1;
    }
    if((d = 5)&&(t==0)){t=1;d=0;}
    if(t==1)
    {
      digitalRead(Echo_PIN);
      digitalWrite(Trig_PIN, HIGH);
    }
      
    if(((micros()-time2)>10)&&(t==1))
    {
      time2 = micros();
      d = d+1;
    }   
    
    if((d = 10)&&(t==1)){t=2;d=0;}
    if(t==2)
    {
      digitalRead(Echo_PIN);
      digitalWrite(Trig_PIN, LOW);
      digitalRead(Echo_PIN);
      t=0;

      UltraCheck ++;
    }

    unsigned int PulseWidth = pulseIn(Echo_PIN, HIGH);
    unsigned int distance = PulseWidth * 0.0173681;
    cm = distance;
    delay(35);

   // Serial.print("Ultrafn | ");
    //Serial.println(cm);
    UltraCheck_fn(); //เรียกฟังก์ชั่นเช็ค
}

void UltraSonic_fn2()
{
  long duration, distance;
  
  digitalWrite(Trig_PIN, LOW); 
  delayMicroseconds(5); 
  digitalWrite(Trig_PIN, HIGH); 
  delayMicroseconds(5); 
  digitalWrite(Trig_PIN, LOW);

  UltraCheck ++;

  duration = pulseIn(Echo_PIN, HIGH);
  distance = (duration/2) / 29.1;
  cm = distance;

 // Serial.print("Ultrafn 2 | ");
 // Serial.println(cm);
  delay(35);

  UltraCheck_fn();
}

void UltraCheck_fn()
{

  if((UltraCheck >= 2)&&(cm == 0))
  {
    e_code = 2;
    error_code();
  }
  
  if((UltraCheck >= 2)&&(cm > 70))
  {
    e_code = 3;
    error_code();
  }

  else if(cm != 0)
  {
    static unsigned long time3 = micros();

    if((micros()-time3)>50000)
    {
      time3 = micros();
         dd = dd+1;
    } 
    if(dd == 1)
    {
      //analogWrite(led13, 50);
    }
    if(dd >= 3)
    {
      //analogWrite(led13, 0);
      dd = 0;
      
    }
  } 

}

void error_code()
{ 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error");

  Serial.println();
  Serial.println("error stat");

  if(e_code == 1)
  {
    Serial.println(" [Hr_Spo2 sensor] not detected");
    do
    { lcd.setCursor(6, 0);
      lcd.print("code 1");
      lcd.setCursor(0, 1);
      lcd.print("Hr_spo2             ");

      LEDStat_1();
    }
    while( e_code != 0);
  }

  if(e_code == 2)
  {
    Serial.println(" [UltraSonic sensor] not detected");
   // do
    { lcd.setCursor(6, 0);
      lcd.print("code 2");
      lcd.setCursor(0, 1);
      lcd.print("UltraSonic            ");

      LEDStat_2();
    }
    //while( cm != 0);
    
  }

  if(e_code == 3)
  {
    Serial.println(" [UltraSonic sensor] Gnd loy");
   // do
    { lcd.setCursor(6, 0);
      lcd.print("code 3");
      lcd.setCursor(0, 1);
      lcd.print("UltraSonic   Gnd loy   ");

      LEDStat_3();
    }
   // while( cm != 0);
    
  }
  setup();
  loop();
}



void LEDStat_1()
{
    int d = 400;
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);delay(d);
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);delay(d);
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);
}

void LEDStat_2()
{
    int d = 400;
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);delay(d);
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);
}

void LEDStat_3()
{
    int d = 200;
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);delay(d);
    digitalWrite(led13, 1);delay(d);
    digitalWrite(led13, 0);
}
