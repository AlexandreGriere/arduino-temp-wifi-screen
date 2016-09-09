//WiFi already set on the ESP8826


#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

SoftwareSerial softSerial(9, 10); // RX, TX

void setup(void)
{
  pinMode(7, INPUT_PULLUP);
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initialisation...");
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");

  // Start up the library
  sensors.begin();

  softSerial.begin(115200);
  softSerial.println("AT+RST");

}

bool firstLiveInit = true;
bool firstStatInit = true;
float minTemp = 999;
float maxTemp = -999;

int loopCount = 0;

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  float temp = sensors.getTempCByIndex(0);

  lcd.setCursor(0, 0);
  lcd.print("Loop :");
  lcd.setCursor(7, 0);
  lcd.print(loopCount);

  if (loopCount >= 250)
  {
    sendValue(String(temp));
    loopCount = 0;
  }
  loopCount++;

  Serial.print("Temperature for Device 1 is: ");
  Serial.print(temp); // Why "byIndex"?
  // You can have more than one IC on the same bus.
  // 0 refers to the first IC on the wire

  if (temp > maxTemp)
  {
    maxTemp = temp;
  }
  else if (temp < minTemp)
  {
    minTemp = temp;
  }

  if (digitalRead(7) == LOW)
  {
    if (firstStatInit)
    {
      lcd.begin(16, 2);
      lcd.setCursor(0, 0);
      lcd.print("T min :");
      lcd.setCursor(0, 1);
      lcd.print("T max :");
      firstLiveInit = true;
      firstStatInit = false;
    }
    lcd.setCursor(8, 0);
    lcd.print(minTemp);
    lcd.setCursor(8, 1);
    lcd.print(maxTemp);

    //lcd.print(maxTemp);

  }
  else
  {
    if (firstLiveInit)
    {
      lcd.begin(16, 2);
      //lcd.print("Temperature :");
      firstLiveInit = false;
      firstStatInit = true;
    }

    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(temp);

  }



}


void sendValue(String temp)
{
  softSerial.println("AT+RST");
  delay(3000);

  firstStatInit = true;
  firstLiveInit = true;
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Transmission");
  lcd.setCursor(0, 1);
  lcd.print("des donnees...");

  String host = "51.255.50.96";

  Serial.println("Connected");

  delay(3000);
  // mode "connexions multiples"
  softSerial.println("AT+CIPMUX=1");

  delay(1000);
  // on se connecte à notre serveur en TCP sur le port 80
  softSerial.println("AT+CIPSTART=4,\"TCP\",\"" + host + "\",80");

  delay(1000);

  String h_request = "GET /ArduinoWebservices/trace.php?t=" + temp + " HTTP/1.1\r\nHost: 51.255.50.96\r\n\r\n";
  int request_length = h_request.length();



  Serial.print("Length : ");
  Serial.println(request_length);

  // on donne la taille de la requête qu'on va envoyer
  softSerial.println("AT+CIPSEND=4," + String(request_length));

  delay(100);

  // on envoie la requete
  softSerial.print(h_request);

  Serial.print(h_request);

  delay(3000);
  // on ferme la connexion
  softSerial.println("AT+CIPCLOSE=4");
}
