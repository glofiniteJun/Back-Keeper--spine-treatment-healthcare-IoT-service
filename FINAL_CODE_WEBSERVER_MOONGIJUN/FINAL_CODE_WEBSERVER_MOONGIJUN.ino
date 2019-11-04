#include "WiFiEsp.h"
#include "Wire.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

String receivestring;
String weightptstring;
String xstring;
String ystring; 
char xstring_arr [6]; 
char ystring_arr [6];
float xvalue = 0.00;                
float yvalue = 0.00; 

int first;
int second;
int last;

char ssid[] = "21residences";            // your network SSID (name)
char pass[] = "clementi2020";        // your network password
int status = WL_IDLE_STATUS;

WiFiEspServer server(80);

void setup()
{
  Serial.begin(115200);   // initialize serial for debugging
  Serial1.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop()
{
   WiFiEspClient client = server.available();  // listen for incoming clients
  
  if (client) { 
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the http request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println("Refresh: 3");  // refresh the page automatically every 3 sec
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        receivestring = "";
        weightptstring = "";
        xstring = "";
        ystring = "";
        memset(xstring_arr, '\0', 6);
        memset(ystring_arr, '\0', 6);
        first = 0;
        second = 0;
        last = 0;
        while(Serial.available()){ //Read from Serial
           //delay(10);
           if (Serial.available() >0) {
                char c = Serial.read ();                   // gets one byte from serial buffer
                receivestring += c;                        // construct the recievestring_ch
           }
        }
        Serial.println(receivestring.length());
         for(int i = 0;i<30; i++){
          if(receivestring[i] == 'f')
          {
            first = i; //remember the first devision point of the raw datas
            Serial.print("first devide = ");
            Serial.println(first);
          }
          else if(receivestring[i] == 's')
          {
            second = i;//remember the second devision point of the raw datas
            Serial.print("second devide = ");
            Serial.println(second);
          }
          else if(receivestring[i] == 'l'){
            last = i; //remember the last devision point of the raw datas
            Serial.print("last devide = ");
            Serial.println(last);
            break;
          }
         }
    
        if (receivestring.length() >0 && first ==0) { //without weight point
           Serial.print ("receivestring = ");                // see what was received
           Serial.println (receivestring);                   // see what was received
           // == expect a string like 0.00s1.00l containing the xvalue and yvalue ==  
           weightptstring = "";
           xstring = receivestring.substring (0, second);      // get the characters until 's' devision point
           ystring = receivestring.substring (second+1, last);       // get the next characters until 'l' devision point
    
           receivestring = ""; //  
           Serial.print ("x= ");                       
           Serial.println (xstring);                      // see what was received
           Serial.print ("y= ");                 
           Serial.println (ystring);                       // see what was received
    
           xvalue = 0.00;                             // declare float for temp
           yvalue = 0.00;                              // declare float for humidity
         
       //  magic needed to convert string to float
           xstring.toCharArray(xstring_arr, sizeof(xstring_arr));
           xvalue = atof(xstring_arr);
           ystring.toCharArray(ystring_arr, sizeof(ystring_arr));
           yvalue = atof(ystring_arr);
      }
      else if(receivestring.length() >0 && first != 0) //When There is Weight point included
      {
           Serial.print ("receivestring = ");                // see what was received
           Serial.println (receivestring);                   // see what was received
           // == expect a string like 0.00s1.00l containing the xvalue and yvalue ==   
           weightptstring = receivestring.substring (0, first);      // get the characters until 'f' devision point
           xstring = receivestring.substring (first+1, second);      // get the characters until 's' devision point
           ystring = receivestring.substring (second+1, last);       // get the characters until 'l' devision point
    
           receivestring = ""; //  
           Serial.print ("weightptstring = ");                       
           Serial.println (weightptstring);
           Serial.print ("x= ");                       
           Serial.println (xstring);                      // see what was received
           Serial.print ("y= ");                 
           Serial.println (ystring);                       // see what was received
    
           xvalue = 0.00;                             // declare float for temp
           yvalue = 0.00;                              // declare float for humidity
         
       //  magic needed to convert string to float
           xstring.toCharArray(xstring_arr, sizeof(xstring_arr));
           xvalue = atof(xstring_arr);
           ystring.toCharArray(ystring_arr, sizeof(ystring_arr));
           yvalue = atof(ystring_arr);
      }   

      
        client.println("******************Your BackKeeper*******************");
        client.println("<br />");
        client.print("The current Weight Point is: ");
        client.print(weightptstring);
        client.print("<br />");
        client.print("X Coordinate : ");
        client.print(xvalue);
        client.print("<br />");
        client.print("Y Coordinate : ");
        client.print(yvalue);
        client.println("<br />");
        client.println("</html>");
      break;
    }
    if (c == '\n') {
      // you're starting a new line
      currentLineIsBlank = true;
    } else if (c != '\r') {
      // you've gotten a character on the current line
      currentLineIsBlank = false;
    }
   }
  }
  // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
