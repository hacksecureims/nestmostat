#include <Wire.h> // to have readString function
#include "rgb_lcd.h"
#include <Ethernet.h>
#include <WString.h>


int tempPin = 1; //Analog A1

rgb_lcd lcd;

int colorR = 0;
int colorG = 200;
int colorB = 100;

byte degree[8] = {
    0b00100,
    0b01010,
    0b00100,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

//For the IP and Webserver
byte mac[] = { 0xA8, 0x61, 0x0A, 0xEF, 0xFE, 0xED };
int conf=20;
String cible="0";
IPAddress ip(10,0,3,7);
IPAddress gateway(10, 0, 3, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsServer(8, 8, 8, 8);
EthernetServer server= EthernetServer(80);

long minute = 60000;

unsigned long times,temp_times;

// To analyse the request
String teststring = String(100);
//////////////////////
int i=0;

void breath(unsigned char color)
{

    for(int i=0; i<255; i++)
    {
        lcd.setPWM(color, i);
        delay(5);
    }

    delay(500);
    for(int i=254; i>=0; i--)
    {
        lcd.setPWM(color, i);
        delay(5);
    }

    delay(500);
}

void init_() {
    Serial.println("Reboot init");
    lcd.clear();
    lcd.setRGB(colorR, colorG, colorB);
    lcd.setCursor(0,0);
    // Print a message to the LCD.
    lcd.print("Boot");
    lcd.setCursor(0,1);
    lcd.print("IP.....");
    times=millis();
    //Au bout d'1 min.
    while (Ethernet.begin(mac)==0 && (((millis()-times)/minute) <1)) {
      Ethernet.begin(mac);
    }
    if (!Ethernet.localIP())
      Ethernet.begin(mac, ip, dnsServer, gateway, subnet);
      
    lcd.clear();
    lcd.setCursor(0,0);
    // Print a message to the LCD.
    lcd.print("Bienvenue");
    lcd.setCursor(0,1);
    lcd.print("IP:");
    lcd.print(Ethernet.localIP());  
    delay(2000);
    server.begin();
    delay(100);
    lcd.clear();
}

void setup() 
{ 
    Serial.begin(9600);
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);   
    lcd.createChar(1, degree);   
    init_();
    times=millis();
}

void loop()   
{  
  String conf_send;
  
  float temp = analogRead(tempPin);//getting the voltage reading from the temperature sensor 
  temp = (float)(1023 - temp)*10000/temp; 
  temp = 1/(log(temp/10000)/3975+1/298.15)-273.15; 

  lcd.setCursor(0,0);
  if (Ethernet.localIP())
    lcd.print("#HackSecuReims");
  else {
    lcd.print("#HackSecuReims !");
    Ethernet.begin(mac);
  }
  
  lcd.setCursor(0,1);
  lcd.print(temp,2);
  lcd.write(1);
  lcd.print("C");

  lcd.print(" Conf:");
  lcd.print(conf);
  lcd.print("  ");

  EthernetClient client = server.available();
  delay(10);
  
  temp_times=(millis()-times)/minute;
  //temp_times=(millis()-times);
  if (temp_times > 1) {
    lcd.setRGB(colorR, colorG, colorB);
  }
   
  if (client) {
    //i=i+1;
    //Serial.println(i);
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        /*
         * The header fields are transmitted after the request or response line, 
         * which is the first line of a message. Header fields are colon-separated 
         * name-value pairs in clear-text string format, terminated by a carriage 
         * return (CR) and line feed (LF) character sequence. The end of the header 
         * fields is indicated by an empty field, resulting in the transmission of 
         * two consecutive CR-LF pairs. Long lines can be folded into multiple lines; 
         * continuation lines are indicated by the presence of a space (SP) or horizontal 
         * tab (HT) as the first character on the next line.[1]
         * 
         */
     teststring = client.readStringUntil('\n');
          
     if (teststring.startsWith("GET ")) {
          //Serial.println("requête :"+teststring);
          String uri=teststring.substring(4,teststring.length());
          if (uri.startsWith("/favicon.ico")) {
            client.stop();
          }
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<head>");
          client.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">");        
          client.println("</head>");
          client.println("<html>");

          if (uri.startsWith("/confort.php?temp=")) {
            String tmp=teststring.substring(22,uri.length());
            // La requête tmp est maintenant du style "XX HTTP/1.1"
            // Il faut donc trouver l'espace pour extraire que la valeur
            int tmp_int=tmp.indexOf(" ");
            conf_send=tmp.substring(0,tmp_int);
            client.println("Votre température de "+conf_send+" °C a été enregistrée<br/>");
            conf=conf_send.toInt();
              if (conf_send==cible) {
                lcd.setPWM(REG_RED,255);
                lcd.setPWM(REG_GREEN,0);
                lcd.setPWM(REG_BLUE,0);                
                times=millis();
               }
          }
          else {
            client.println("What is your confortable wished temperature ?");
          }    

          client.println("</html>");
          teststring="";
      }

        if (teststring.startsWith("\r")) {        
          client.stop();
        }
        
        }

      }
    }
}

 

