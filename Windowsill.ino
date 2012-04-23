/*
 Sketch snaps photos of birdfeeder, saves to SD card, sends to web server, emails to digital photo frame
 
 This sketch connects to a a web server and makes a request
 using a WiFi shield. 
 This example uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 19 Apr 2012
 by Tom Igoe
 
 http://arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.
 
 */


#include <Adafruit_VC0706.h>
#include <SD.h>
#include <SoftwareSerial.h>  
#include <SPI.h>
#include <WiFi.h>

char ssid[] = "itpsandbox"; //  your network SSID (name) 
char pass[] = "NYU+s0a!+P?";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

char server[] = "courtney-mitchell.com";

String currentLine = "";
String dateString = "";
boolean readingDate = false;

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000;  // delay between updates, in milliseconds (every hour)


#define chipSelect 4 //define SD pin
// On Uno: camera TX connected to pin 2, camera RX to pin 5:
SoftwareSerial cameraconnection = SoftwareSerial(2, 5);

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);


void setup() {

  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
  if(chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif

  // start serial port:
  Serial.begin(9600);
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  Serial.println("Hi there");
  status = WiFi.begin(ssid, pass);
  Serial.println("THIS IS A LINE");
  while( status != WL_CONNECTED) { 
    status = WiFi.begin(ssid, pass);
    Serial.println("attempting to connect");
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());
  // reserve space for the server response string:
  currentLine.reserve(100);
  dateString.reserve(100);
}

void loop() {

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  int reading = false;
  String numberString = "";
  int theTime = 0;

  while (client.available()) {
    // read a character:
    char c = client.read();
    // pointy bracket opens the numberstring:
    if (c == '<') {
      // empty the string for new data:
      numberString = "";
      // start reading the number:
      reading = true; 
    } 
    else {
      // if you're reading, and you haven't hit the >:
      if (reading) {
        if (c != '>') {
          // add the new char to the string:
          numberString += c;
        } 
        else {
          char argh[4];
          numberString.toCharArray(argh, 4);
          int theNumber = atoi(argh);
          Serial.println(theNumber); 
          theTime = theNumber;
        }
      }
    }
  }
  
  if (theTime > 0) {
    Serial.println(theTime);
    locateCard();
    removeImage();
  }

 // if it is the time we want, take a picture
// if (theTime == 7 || theTime == 15 || theTime == 19) {
//   takePicture();
// }
// 
 
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("getting hour from server...");
    // send the HTTP PUT request:
    client.println("GET /windowsill/getHour.php HTTP/1.1");
    client.println("Host: courtney-mitchell.com");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void locateCard(){
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  else {
    Serial.println("Card found");
  }
}


void removeImage(){
  Serial.println("Inside Remove Image");
  if (SD.exists("IMAGE.JPG")) {
    //SD.remove("IMAGE.JPG");
    Serial.println("Image removed.");
  }
  else {
    Serial.println("IMAGE.JPG doesn't exist.");
  }
}


void locateCamera(){
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } 
  else {
    Serial.println("No camera found?");
    return;
  }
}


void pictureSettings(){
  //set image size to 640x480
  cam.setImageSize(VC0706_640x480);
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
}


void delayBeforeShooting(){
  Serial.println("Snap in 3 secs...");
  delay(3000);
}

void snapPicture(){
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
}


void savePictureToSD(){
  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE.JPG");
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);
  // Get the size of the image (frame) taken jpglen variable is the size of the photo
  uint16_t jpglen = cam.frameLength();
  Serial.print("Saving... Image size: ");
  Serial.print(jpglen, DEC);
  Serial.print("");
  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();
  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); 
  Serial.println(" ms elapsed");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////


// Function to take picture and save it to SD card
void takePicture() {

  Serial.println("VC0706 Camera snapshot test");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } 
  else {
    Serial.println("No camera found?");
    return;
  }

  // set the picture size
  cam.setImageSize(VC0706_640x480);        // biggest
  // give the ethernet module time to boot up:
  Serial.println("Snap in 3 secs...");
  delay(3000);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");

  // Create an image with the name IMAGExx.JPG
  char filename[13];
  if (SD.exists("IMAGE.JPG")) {
    SD.remove("IMAGE.JPG");
    Serial.println("Image removed.");
  }
  else {
    Serial.println("IMAGE.JPG doesn't exist.");
  }
  strcpy(filename, "IMAGE.JPG");
  // create if does not exist, do not open existing, write, sync after write
  /* if (! SD.exists(filename)) {
   Serial.println("file already there");
   //break;
   }
   */
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    jpglen -= bytesToRead;
  }
  imgFile.close();

  time = millis() - time;
  Serial.println("done!");
  Serial.print(time); 
  Serial.println(" ms elapsed");

}

// function to send picture from SD card to web server (which kicks off email to photo frame)
/*void sendPicture() {
 String fileName = "jasper.jpg";
 String boundary = "--CmDkCe13--";
 
 // if there's a successful connection:
 if (client.connect(server, 80)) {
 Serial.println("connecting...");
 // send the HTTP PUT request:
 client.println("POST /windowsill/fileupload.php HTTP/1.1");
 client.println("Host: courtney-mitchell.com");
 client.println("User-Agent: arduino-ethernet"); //do i need this?
 
 Serial.println("posting image to the server");
 
 // tell the server you're sending the POST in multiple parts
 // send the unique string that delineates the parts
 client.print("Content-Type: multipart/form-data; boundary=");
 client.println(boundary + "\n");
 
 // form the beginning of the request
 String requestHead = "\n--" + boundary + "\n";
 requestHead += "Content-Disposition: form-data; name=\"file\";";
 requestHead += "filename=\"" + fileName + "\"\n";
 requestHead += "Content-Type: image/jpeg\n\n";
 
 // form the end of the request
 String tail = "\n--" + boundary + "--\n\n";
 
 // calculate and send the length of the request
 // including the head, the file, and the tail:
 
 int contentLength = requestHead.length() + thisFile.length + tail.length();
 client.println("Content-Length: " + contentLength + "\n\n");
 
 Serial.println("Sending image " + fileName + "of size " + thisFile.length + "and this boundary " + boundary);
 
 //send the header, the file, and the tail
 client.println(requestHead);
 
 if (dataFile) {
 while (dataFile.available()) {
 client.write(dataFile.read());
 }
 dataFile.close();
 }  
 client.println(tail);
 
 //close the client:
 client.println("Connection: close");
 client.println();
 
 client.stop(); //is this necessary? 
 
 // note the time that the connection was made:
 lastConnectionTime = millis();
 
 } 
 else {
 // if you couldn't make a connection:
 Serial.println("connection failed");
 Serial.println("disconnecting.");
 client.stop();
 }
 
 }*/










