int ledPin = 13;
int blinkDelay = 1000;

void setup() {                
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(ledPin, OUTPUT);    
  Serial.begin(115200); //115200
  Serial.println("hello");
  //digitalWrite(ledPin, HIGH);   // set the LED on
  //delay(3000);
}

void loop() {
  //digitalWrite(ledPin, HIGH);   // set the LED on
  //delay(blinkDelay);              // wait for a second
  //digitalWrite(ledPin, LOW);    // set the LED off
  //delay(blinkDelay);              // wait for a second
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    int newDelay = Serial.parseInt(); 
    if(newDelay > 0) {
      Serial.print("ok ");
      //Serial.print("newDelay: ");
      Serial.println(newDelay);
      //Serial.println("ok");
      blinkDelay = newDelay;
    }
  }
}
