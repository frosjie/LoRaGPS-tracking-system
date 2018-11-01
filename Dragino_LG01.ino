#define BAUDRATE 115200 
#include <Console.h> 
#include <SPI.h> 
#include <RH_RF95.h> 
#include <FileIO.h> 
#include <Bridge.h> 
#include <YunClient.h> 

YunClient client; 
RH_RF95 rf95;   // Singleton instance of the radio driver 
 
int led = A2; 
float frequency = 868.0; 
String dataString = ""; 
const char *server = "10.130.1.239"; 
int port_number = 80; 
    
void setup()  
{ 
  pinMode(led, OUTPUT);      
  Bridge.begin(BAUDRATE); 
  Console.begin(); 
  FileSystem.begin(); 
   
  while (!Console) ; // Wait for console port to be available 
   
//*****************************Frequency connection setup**********************************//  

  if (!rf95.init()) 
    Console.println("init failed"); 
  // Setup ISM frequency 
  rf95.setFrequency(frequency); 
  // Setup Power,dBm 
  rf95.setTxPower(13); 
   
  // Setup Spreading Factor (6 ~ 12) 
  rf95.setSpreadingFactor(7); 
   
  // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000 
  rf95.setSignalBandwidth(125000); 
   
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8)  
  rf95.setCodingRate4(5); 
   
  Console.print("\nListening on frequency: "); 
  Console.println(frequency); 
} 
 
void loop()  
{ 
//*****************************Receive Message Setup**********************************// 
  dataString = ""; //to clear dataString before adding 
  
  if (rf95.available()) 
  { 
    // Should be a message for us now    
    Console.println("Data Receive From Client..."); 
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN]; 
    uint8_t len = sizeof(buf); 
     
    if (rf95.recv(buf, &len)) 
    { 
      digitalWrite(led, HIGH); 
      RH_RF95::printBuffer("request: ", buf, len); 
      Console.print("Data Recieved: "); Console.println((char*)buf); 
             
      // Send a reply 
      uint8_t data[] = "Data Recieved at Gateway"; 
      rf95.send(data, sizeof(data)); 
      rf95.waitPacketSent(); 
 
//*****************************Store in USB**********************************//  
//if not detected open dragino setup mount points tick on enables mount point 
 
      //make a string that start with a timestamp for assembling the data to log: 
      dataString += String((char*)buf); 
      dataString += ","; 
      dataString += getTimeStamp(); 
      dataString += ","; 
      dataString += String (rf95.lastRssi(), DEC); 

      File dataFile = FileSystem.open("/mnt/sda1/data/datalog1.csv", FILE_APPEND); 
 
      // if the file is available, write to it: 
      if (dataFile) { 
        //Console.println("Writing data."); 
        dataFile.println(dataString); 
        dataFile.close(); 
        // print to the serial port too: 
        Console.print("Data Stored in SD card: "); Console.println(dataString); 
        } 
       
      // if the file isn't open, pop up an error: 
      else { 
        Console.println("\nerror opening datalog.csv"); 
      }            
 
 //*****************************Upload to XAMPP**********************************//  

 //extract string coma data 
    String Latitude = getValue(dataString,',',0);
    String Longtitude = getValue(dataString,',',1);
    String Time = getValue(dataString,',',2);
    String Rssi = getValue(dataString,',',3);
    
    if(client.connect(server, port_number)){ 
        Console.print("Succesfully connected to server with port "); Console.println(port_number);
        client.print("GET /database/GPS.php?");         
        client.print("&Longtitude="); 
        client.print(Longtitude);        
        client.print("&Latitude="); 
        client.println(Latitude); 
        client.stop(); 
        Console.print("Data pushed to database!");
        } 
     else { 
      Console.print("Unable to connect database"); 
     } 
      Console.print("\n\n");   
    } 
  } 
} 
 
//*****************************Convert CSV data **********************************//  

String getValue(String data, char separator, int index) 
{ 
    int found = 0; 
    int strIndex[] = { 0, -1 }; 
    int maxIndex = data.length() - 1; 
 
    for (int i = 0; i <= maxIndex && found <= index; i++) { 
        if (data.charAt(i) == separator || i == maxIndex) { 
            found++; 
            strIndex[0] = strIndex[1] + 1; 
            strIndex[1] = (i == maxIndex) ? i+1 : i; 
        } 
    } 
    return found > index ? data.substring(strIndex[0], strIndex[1]) : ""; 
} 
 
//*****************************Time stamp fram LG01**********************************// 
//if not sync can setup in 10.130.1.1 
 
 
// This function return a string with the time stamp 
// Yun Shield will call the Linux "data" command and get the time stamp 
String getTimeStamp() { 
  String result; 
  Process time; 
  // date is a command line utility to get the date and the time  
  // in different formats depending on the additional parameter  
  time.begin("date"); 
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy 
                                //             T for the time hh:mm:ss     
  time.run();  // run the command 
 
 
  // read the output of the command 
  while(time.available()>0) { 
    char c = time.read(); 
    if(c != '\n') 
      result += c; 
  } 
  return result; 
}
