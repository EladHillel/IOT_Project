#define MOTOR1_PIN 18
#define MOTOR2_PIN 19
#define MOTOR3_PIN 21

void setup() {
  // Initialize motor control pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  
  // Start motors off
  digitalWrite(MOTOR1_PIN, LOW);
  digitalWrite(MOTOR2_PIN, LOW);
  digitalWrite(MOTOR3_PIN, LOW);
  
  // Wait for a while before testing
  delay(1000);
}

void loop() {
  // Test Motor 1 (forward)
  digitalWrite(MOTOR1_PIN, HIGH);  
  delay(1000);  // Run for 1 second
  digitalWrite(MOTOR1_PIN, LOW);  
  delay(1000);  // Stop for 1 second
  
  // Test Motor 2 (forward)
  digitalWrite(MOTOR2_PIN, HIGH);  
  delay(1000);  // Run for 1 second
  digitalWrite(MOTOR2_PIN, LOW);  
  delay(1000);  // Stop for 1 second
  
  // Test Motor 3 (forward)
  digitalWrite(MOTOR3_PIN, HIGH);  
  delay(1000);  // Run for 1 second
  digitalWrite(MOTOR3_PIN, LOW);  
  delay(1000);  // Stop for 1 second
}
