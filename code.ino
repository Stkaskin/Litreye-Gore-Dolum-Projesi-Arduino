/*
Liquid flow rate sensor -DIYhacking.com Arvind Sanjeev

Measure the liquid/water flow rate using this code. 
Connect Vcc and Gnd of sensor to arduino, and the 
signal line to arduino digital pin 2.

*/
#include <LiquidCrystal.h>
#include <EEPROM.h>
LiquidCrystal ekran(8,9,4,5,6,7);
int basilan_tus=0;
int okunan_deger=0;
int i=0;

#define sag    0
#define yukari 1
#define asagi  2
#define sol    3
#define sec    4
#define yok    5





byte sayfa=1;
int solenoidPin = 3;
byte statusLed    = 6;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;
byte baslatdur=0;
// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 6.4;

volatile byte pulseCount;  
int totalcount=0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long dolum=1000;
unsigned long oldTime;
unsigned long birim=50;
unsigned long secimlitre;
void setup()
{ekran.begin(16,2);
ekran.setCursor(0,0);
ekran.print("Litre : ");
sayfa=1;
Serial.println();
Serial.print("sayfa: ");
Serial.println(sayfa);
// Initialize a serial connection for reporting values to the host
Serial.begin(38400);

// Set up the status LED line as an output
pinMode(statusLed, OUTPUT);
digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
pinMode(solenoidPin, OUTPUT);  
pinMode(sensorPin, INPUT);
digitalWrite(sensorPin, HIGH);

digitalWrite(solenoidPin, LOW);
pulseCount        = 0;
flowRate          = 0.0;
flowMilliLitres   = 0;
totalMilliLitres  = 0;
oldTime           = 0;
//EEPROM.write(10,6.4);
if (EEPROM.read(10)!=255){ 
  int sayi=EEPROM.read(10);
  float ondalik=(float)EEPROM.read(11)/100;
} 


// The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
// Configured to trigger on a FALLING state change (transition from HIGH
// state to LOW state)
attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}
byte buton_bul(){
  okunan_deger=analogRead(A0);
 
  if (okunan_deger > 1000){return yok;}
  if (okunan_deger < 50){return sag;}
  if (okunan_deger < 195){return yukari;}
  if (okunan_deger < 380){
     
    return asagi;
    }
  if (okunan_deger < 555){
   return yukari;
    }
  if (okunan_deger < 790){return sec;}

}

/**
* Main program loop
*/
void loop()
{       
 klavye();
if (sayfa == 2){
  sayfa2();}
else if (sayfa == 1){
  sayfa1();delay(100);}
else if (sayfa==3){
  sayfa3();delay(100);}
//  Serial.println();
//Serial.print("sayfa: ");
// Serial.println(sayfa);

}
void klavye(){

 Serial.println();
Serial.print("DEĞİŞTİ: ");
 Serial.println(analogRead(A0));

  basilan_tus= buton_bul();
   Serial.print("birim: ");
 Serial.println(basilan_tus);
  switch (basilan_tus){

  case sag:{ 
  //sag tusuna basınca ilk sayfadaysa kalibrasyona  
  //2 sayfadaysa bitirmeye ardından sayfa1e
   //3. dolum bitti sayfasında ise sayfa 1 e
   if (sayfa==2 || sayfa==3){baslatdur=0;totalMilliLitres=0; digitalWrite(solenoidPin, LOW);sayfa1();  delay(200);}
    else if (sayfa==1){kalibrasyon();delay(500);}


       break; }  
  case sol:{
    //ml birim değiştirir
    if (sayfa==1 ){
    if (birim==50){birim=100;}
    else if (birim==100){birim=500;}
    else if (birim==500){birim=1000;}
    else if (birim==1000){birim=5000;}
    else if (birim==5000){birim=50;}
    
   
    ekransecim();
    delay(100);}
    
    break;
       }
  case yukari:{//sayfa 1 de dolumu artırır
    if (sayfa==1){
    dolum+=birim;
    ekransecim();
    delay(100);}

    break;}
  case asagi:{//dolumu azaltır
    if (sayfa==1){
    if (dolum-birim>0 && dolum-birim<500000){
    dolum-=birim;
    ekransecim(); delay(200);}
    }

    break;}

  case sec:{
    
    if (sayfa==1){
      totalMilliLitres=0;
      ekran.clear();
      ekran.print("basliyor..");
      delay(1000);     ekran.clear();

      // belirlenen litre için vafli acıyoruz baslattıgını belirtiroyur (baslatdur)
    totalcount=0; digitalWrite(solenoidPin, HIGH);sayfa=2;  sayfa2();
      baslatdur=1;
    }
    else if (sayfa==2){ 
      //eğer doluma başladıysa durdurma başlatma komutu ve baslatıgını durdurdugunu belirtiyoruz (baslatdur)
      if (baslatdur==0){baslatdur=1; digitalWrite(solenoidPin, HIGH);}
      else {baslatdur=0; digitalWrite(solenoidPin, LOW);delay(100);}
      

    }
    else if (sayfa==3) {
      //eğer diğer sayfadaysa direk sayfa 1 aktarır.
      sayfa1();


    }
    else if (sayfa==99){sayfa1();}
   
    break;}
  case yok:{  break;}
  }




}
void sayfa2(){


      
    if((millis() - oldTime) > 1000) {      // Only process counters once per second
      sayfa=2;
     
      detachInterrupt(sensorInterrupt);
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
     oldTime = millis();
     flowMilliLitres = (flowRate / 60) * 1000;
     totalMilliLitres += flowMilliLitres;
      totalcount+=pulseCount;
      if (totalMilliLitres>=dolum)   { digitalWrite(solenoidPin, LOW); sayfa=1;  }
      pulseCount = 0;  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
     ekranyukleme();

   /*   unsigned int frac; 
    *    // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.print(".");             // Print the decimal point
      // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
      frac = (flowRate - int(flowRate)) * 10;
      Serial.print(frac, DEC) ;      // Print the fractional part of the variable
      Serial.print("L/min");
      // Print the number of litres flowed in this second
      Serial.print("  Current Liquid Flowing: ");             // Output separator
      Serial.print(flowMilliLitres);
      Serial.print("mL/Sec");
    */
      // Print the cumulative total of litres flowed since starting
   /*   Serial.print("  Output Liquid Quantity: ");             // Output separator
      Serial.print(totalMilliLitres);
      Serial.print("mL");
      Serial.print(" Totalcount: ");
      Serial.println(totalcount); 
      ekran.clear();
      ekran.print("acik:");
      ekran.println(totalMilliLitres); 
      ekran.setCursor(0,1);
      ekran.print("count");
      ekran.println(totalcount);*/
    
       
      /* Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.print(".");             // Print the decimal point
      // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
      frac = (flowRate - int(flowRate)) * 10;
   /  Serial.print(0, DEC) ;      // Print the fractional part of the variable
      Serial.print("L/min");
      // Print the number of litres flowed in this second
      Serial.print("  Current Liquid Flowing: ");             // Output separator
      Serial.print(flowMilliLitres);
      Serial.print("mL/Sec");
     
      // Print the cumulative total of litres flowed since starting
      Serial.print("  Output Liquid Quantity: ");             // Output separator
      Serial.print(totalMilliLitres);
      Serial.print("mL");
      Serial.print(" Totalcount: ");
      Serial.println(totalcount); 
      ekran.clear();
      ekran.print("acik:");
      ekran.println(totalMilliLitres); 
      ekran.setCursor(0,1);
      ekran.print("count");
      ekran.println(totalcount);*/
        


      // Reset the pulse counter so we can start incrementing again
     

      // Enable the interrupt again now that we've finished sending output

 
   
    }
   

  
    
}

void kalibrasyon(){
  totalMilliLitres=0;
ekran.clear();
  ekran.setCursor(0,0);
  ekran.print("500 Ml sise doldurun");
  ekran.setCursor(0,1);
        ekran.setCursor(0,1);
        ekran.print("Baslayin.");
sayfa=99;
   while(1){
         if (basilan_tus==sec){ digitalWrite(solenoidPin, LOW); break;}    
    if((millis() - oldTime) > 1000)    // Only process counters once per second
    { 
      // Disable the interrupt while calculating flow rate and sending the value to
      // the host
      detachInterrupt(sensorInterrupt);

      // Because this loop may not complete in exactly 1 second intervals we calculate
      // the number of milliseconds that have passed since the last execution and use
      // that to scale the output. We also apply the calibrationFactor to scale the output
      // based on the number of pulses per second per units of measure (litres/minute in
      // this case) coming from the sensor.
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;


      // Note the time this processing pass was executed. Note that because we've
      // disabled interrupts the millis() function won't actually be incrementing right
      // at this point, but it will still return the value it was set to just before
      // interrupts went away.
      oldTime = millis();

      // Divide the flow rate in litres/minute by 60 to determine how many litres have
      // passed through the sensor in this 1 second interval, then multiply by 1000 to
      // convert to millilitres.
      flowMilliLitres = (flowRate / 60) * 1000;

      // Add the millilitres passed in this second to the cumulative total
      totalMilliLitres += flowMilliLitres;
     
      unsigned int frac;


      frac = (flowRate - int(flowRate)) * 10;
   
      totalcount+=pulseCount;
      // Print the cumulative total of litres flowed since starting
     
          pulseCount = 0;  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

          //burayı stop olarak değişecek
          sayfa=99;
 
    
    if (totalMilliLitres!=0){ekran.setCursor(0,1);
        ekran.print("Basladi. ");}
      
  ekran.clear();
  ekran.setCursor(0,0);
  ekran.print("ML : ");
  ekran.print( totalMilliLitres );
  }
  basilan_tus= buton_bul();
  }
  float x=(float)totalMilliLitres/500.00;


  calibrationFactor = calibrationFactor * x;

 /* EEPROMA kaydedilecek veri */
 calibrationFactor-=0.01;
 int sayi=calibrationFactor;
 int ondalik =(calibrationFactor-sayi)*100;
 
 EEPROM.write(10, sayi); 
Serial.print("sayi");Serial.println(  EEPROM.read(10));
 
  EEPROM.write(11, ondalik);
Serial.print("ondalik");Serial.println(  EEPROM.read(11));
      Serial.print(" x :");
  Serial.println(x);
  sayfa=1;  
  Serial.print(" kalibrasyon: ");
  Serial.println(calibrationFactor);
    Serial.print(" x :");
  Serial.println(x);
      Serial.print(" total :");
  Serial.println(500/totalMilliLitres);
  ekran.clear();
  ekran.setCursor(0,0);
  ekran.print("calibrasyon");
  ekran.setCursor(0,1);
  ekran.print("  ayarlandi.");
  delay(5000);
  sayfa1();
  }




void sayfa1(){sayfa=1;totalMilliLitres=0;
if (digitalRead(solenoidPin)){digitalWrite(solenoidPin,LOW);}delay(50); ekransecim();}
void sayfa3(){
sayfa=3;
}

void ekransecim(){
   ekran.clear();
  ekran.print("Litre:");
  ekran.setCursor(8,0);
  ekran.print(dolum);
  ekran.setCursor(14,0);
  ekran.print("ML");
  ekran.setCursor(0,1);
  ekran.print("BIRIM:");
  ekran.setCursor(8,1);
  ekran.print(birim);
  ekran.setCursor(14,1);
  ekran.print("ML");


}
void ekranyukleme(){
  ekran.clear();
  ekran.setCursor(0,0);
  ekran.print("TOPLAM:");
  ekran.setCursor(8,0);
  ekran.print(dolum);
  ekran.setCursor(14,0);
  ekran.print("ML");

  ekran.setCursor(0,1);
  ekran.print("YUKLE:");
  ekran.setCursor(8,1);
  ekran.print(totalMilliLitres);
  ekran.setCursor(14,1);
  ekran.print("ML");
}

/*
Insterrupt Service Routine
*/
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}