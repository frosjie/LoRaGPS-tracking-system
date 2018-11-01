#include <SoftwareSerial.h> 
#include <TinyGPS++.h>
#include <SPI.h> 
#include <RH_RF95.h> 

float frequency = 868.0; 
TinyGPSPlus gps;
SoftwareSerial ss(3, 4); 
RH_RF95 rf95; 

char databuf[100]; 
uint8_t dataoutgoing[100]; 
char gps_lon[20];  
char gps_lat[20];
String str_lat,str_lon;
float flat, flon;

void setup()  
{ 
  Serial.begin(115200); 
  while (!Serial) ; // Wait for serial port to be available 
  ss.begin(9600);
  Serial.println("Start LoRa Client GPS");
   
  if (!rf95.init()) 
    Serial.println("init failed"); 
    
    Serial.print("Simple TinyGPSPlus library v. ");  
    Serial.println(TinyGPSPlus::libraryVersion()); 
    Serial.println(); 
    // Setup ISM frequency 

    rf95.setFrequency(frequency); 
    // Setup Power,dBm 
    rf95.setTxPower(13); 
   
    // Setup Spreading Factor (6 ~ 12) 
    rf95.setSpreadingFactor(7); 
     
    // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000 
    //Lower BandWidth for longer distance. 
    rf95.setSignalBandwidth(125000); 
     
    // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)  
    rf95.setCodingRate4(5); 
} 
 
void loop() 
{  
  // Print Sending to rf95_server 
  Serial.println("Sending to rf95_server"); 
  bool newData = false; 
 
  // For one second we parse GPS data and report some key values 
  for (unsigned long start = millis(); millis() - start < 1000;) 
  { 
    while (ss.available() > 0) 
    { 
      if (gps.encode(ss.read())) // Did a new valid sentence come in? 
        newData = true; 

        if (millis() > 5000 && gps.charsProcessed() < 10)
        {
          Serial.println(F("No GPS detected: check wiring."));    
        }
     } 
  } 
  //Get the GPS data 
  if (newData) 
  { 
    print_gps_information();

    //Convert string to character array
    dtostrf(flat,0,6,gps_lat);
    dtostrf(flon,0,6,gps_lon);
    str_lat = gps_lat;
    str_lon = gps_lon;
    str_lat += ",";
    str_lat += gps_lon;
    
    str_lat.toCharArray(databuf,100);
    strcpy((char *)dataoutgoing,databuf);  

    // Send the data to server 
    rf95.send(dataoutgoing, sizeof(dataoutgoing)); 
    Serial.print("Data sent: "); Serial.println((char *)dataoutgoing);
    
    // Now wait for a reply 
    check_server_response();
    delay(400);  
  }
  Serial.println("\n");
}  

void print_gps_information()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    
    flat = gps.location.lat();
    flon = gps.location.lng(); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
    }
  else
  {
    Serial.print(F("INVALID"));
  }
}

void check_server_response()
{
  uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN]; 
  uint8_t len = sizeof(indatabuf);
      
      if (rf95.waitAvailableTimeout(3000)) 
     {  
       // Should be a reply message for us now    
       if (rf95.recv(indatabuf, &len)) 
      { 
         // Serial print "got reply:" and the reply message from the server 
         Serial.print("Reply From Server: "); 
         Serial.println((char*)indatabuf); 
      } 
      else 
      { 
      Serial.println("receive failed"); 
      } 
    } 
    else 
    { 
      // Serial print "No reply, is rf95_server running?" if don't get the reply . 
      Serial.println("No reply, is rf95_server running?"); 
    } 
}
