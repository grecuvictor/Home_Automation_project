/**
 * Intel Galileo 2 | Server + PIR Sensor + Light Control
 * -----------------------------------------------------------------------------------------
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 * DIGITAL     Room 2 Relay                2
 * DIGITAL     Room 3 Relay                3
 * DIGITAL     Room 4 Relay                4
 * DIGITAL     Room 5 Relay                5
 * DIGITAL     Room 6 Relay                6
 * DIGITAL     PIR Sensor                  7
 * ANALOG      Light Sensor                2
 */
 
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 
  0x98, 0x4F, 0xEE, 0x03, 0x1A, 0xC0 };    //MAC Adress of Intel Galileo board
IPAddress ip(192,168,0,103);               //Ip Adress: 192.168.0.103:8080

#define	Room_2		2
#define	Room_3		3		
#define	Room_4		4
#define	Room_5		5
#define	Room_6		6		
#define PIR_Sensor      7      //DIGITAL PIR  sensor pin
#define Light_Sensor    2      //ANALOG Light sensor pin

boolean    LED_status = 0;             //Stores the value of the Led for the server
byte       LightValue = 0;             //ANALOG -> Stores the value sent from the Light sensor 1
byte       PirValue   = 0;             //ANALOG -> Stores the value sent from the Water sensor
String     HTTP_req;  

EthernetServer server(8080); // Initialize the Ethernet server library (default:8080)

void setup() {
  Serial.begin(9600);
pinMode(Room_2, OUTPUT);
digitalWrite(Room_2, 1);        // Initialize Room 2 light as Output Pin (Default OFF)
pinMode(Room_3, OUTPUT);
digitalWrite(Room_3, 1);        // Initialize Room 3 light as Output Pin (Default OFF)
pinMode(Room_4, OUTPUT);
digitalWrite(Room_4, 1);        // Initialize Room 4 light as Output Pin (Default OFF)
pinMode(Room_5, OUTPUT);
digitalWrite(Room_5, 1);        // Initialize Room 5 light as Output Pin (Default OFF)
pinMode(Room_6, OUTPUT);
digitalWrite(Room_6, 1);        // Initialize Room 6 light as Output Pin (Default OFF)
pinMode(Light_Sensor, INPUT);   //Light sensor on pin 2
pinMode(13, OUTPUT);            // LED on pin 13
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip); //Initialize server with the right ip & mac adress
  server.begin();         //Server ready
  Serial.print("Server is ON at the folowing IP: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup Ready!");   //Let the user know that the setup is ready (No errors)
}


void loop() {
  delay(2000);   //Wait for sensor to calibrate
  /*************READING SENSORS VALUES****************/

  LightValue = analogRead(Light_Sensor);
  PirValue   = digitalRead(PIR_Sensor);
  PIR_Detection(LightValue,PirValue);                 //Presence Detection Function

  /****************SERVER PROCESSING*******************/
  EthernetClient client = server.available(); // Listen for incoming clients
  if (client) {
    Serial.println("new client"); //println because an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        HTTP_req += c; 
        //    Serial.write(c);
        if (c == '\n' && currentLineIsBlank)
        {
          /*Send a standard http response header*/
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<meta http-equiv=\"refresh\" content=\"5\">"); //Meta refresh tag (Browser refresh the page every 5 sec)
          client.println("<head>");
          client.println("<title>Intel Galileo Server : Light Control</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>LED</h1>");
          client.println("<p>Click to switch LED on and off.</p>");
          client.println("<form method=\"get\">");
          ProcessCheckbox(client);
          client.println("</form>");
          client.println("</body>");
          HTTP_req="";
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          currentLineIsBlank = true;   //New line
        } 
        else if (c != '\r')
        {
          currentLineIsBlank = false; //Character detected on the urent line
        }
      }
    }
    delay(5);                         //Delay for the Browser to receive & process the data
    client.stop();                    // close the connection:
    Serial.println("Client disonnected");
  }
}
/**********************************************
 ******Flip LED value & HTML LED Checkbox****** 
 *********************************************/
 
void ProcessCheckbox(EthernetClient cl)
{
    Serial.println( HTTP_req);
    if (LED_status==1 && HTTP_req.indexOf("control13=off") > -1)
    {
       LED_status = 0;
     }
       if (HTTP_req.indexOf("control13=on") > -1 && LED_status==0)
       {
         Serial.println( HTTP_req.indexOf("control13=on") > -1 && LED_status==0);
         LED_status = 1;       // Checkbox clicked -> toggle the led
       } 
    if (LED_status==1) 
    {    // switch LED on && Checkbox checked
        digitalWrite(13, HIGH);
        cl.println("<input type=\"hidden\"  \ value=\"off\" name=\"control13\" >");
        cl.println("<div style=\"margin-bottom: 15px; background: #ffff00;  width: 100px; height: 100px; border-radius: 50%;\"></div>");     
        cl.println("<button type=\"button\" \ onclick=\"submit();\">Turn OFF</button>");
    }
    else
    {    // switch LED off && checkbox unchecked
       digitalWrite(13, LOW);
       cl.println("<input type=\"hidden\" \  value=\"on\" name=\"control13\" >");
       cl.println("<div style=\" margin-bottom: 15px; background: #ffff80; width: 100px; height: 100px; border-radius: 50%;\"></div>"); 
       cl.println("<button type=\"button\" \ onclick=\"submit();\">Turn ON</button>");
    }
    HTTP_req="";
}

/***********************************************
 ***********Entrance Light Control**************
 ***********************************************/
 
void PIR_Detection(byte LightValue,byte PirValue)
{
  if((LightValue < 195) && PirValue)
  {
    digitalWrite(Room_3, 0);    // Night && Motion Detected -> Light ON
  }
  else
  {
    digitalWrite(Room_3, 1);    // Else Light remains off
  }
}
