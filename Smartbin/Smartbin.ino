

#include <FirebaseESP8266.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Define the pins for the ultrasonic sensor and the MQ-2 gas sensor
const int trigPin = D5;
const int echoPin = D6;
const int gasSensorPin = A0;

// Define the variables to hold the sensor data
int distance = 0;
int gasValue = 0;

// Define the GPS module serial pins and variables
const int gpsTxPin = D1;
const int gpsRxPin = D2;
SoftwareSerial gpsSerial(gpsRxPin, gpsTxPin);
TinyGPSPlus gps;

// Define the Firebase variables
#define FIREBASE_HOST "smart-bin-15e98-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "igwrD2ZhOPnav7Al9WcVTA0RXjAmHqpGACo5yhUJ"

// WiFi credentials
#define WIFI_SSID "Hui Hui"
#define WIFI_PASSWORD "huihuihu"

FirebaseData firebaseData;

void setup() {
  // Start the serial communication
  Serial.begin(9600);

   // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Set up the ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Set up the GPS serial communication
  gpsSerial.begin(9600);

  // Set up the Firebase connection
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Measure the distance with the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  distance = pulseIn(echoPin, HIGH) / 58;
  long percentage = (distance*100)/40;

  // Read the gas sensor value
  gasValue = analogRead(gasSensorPin);

  // Read the GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        double latitude = gps.location.lat();
        double longitude = gps.location.lng();
        double altitude = gps.altitude.meters();
        String gpsData = String(latitude) + "," + String(longitude) + "," + String(altitude);
        Firebase.setString(firebaseData, "/gpsData", gpsData);
      }
    }
  }

  // Print the sensor data to the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Fill: ");
  Serial.print(100-percentage);
  Serial.print(" %, Remaining: ");
  Serial.print(percentage);
  Serial.print(" %, Gas Value: ");
  Serial.println(gasValue);

  // Send the sensor data to Firebase
  Firebase.setDouble(firebaseData, "/sensors/Bin fill", 100-percentage);
  Firebase.setDouble(firebaseData, "/sensors/Remaining", percentage);
  Firebase.setDouble(firebaseData, "/sensors/gasValue in ppm", gasValue);

  // Wait for a short time before taking the next measurement
  delay(5000);
}
