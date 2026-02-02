#include <LiquidCrystal.h>
#include <Servo.h>

// LCD 20x4
LiquidCrystal lcd(31, 35, 45, 47, 49, 51);
Servo ventana;

#define POT_PIN A0
#define FAN_PIN 5
#define SERVO_PIN 6

void setup() {
  pinMode(FAN_PIN, OUTPUT);

  ventana.attach(SERVO_PIN);

  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.print("Invernadero");
  delay(1500);
  lcd.clear();
}

void loop() {
  int valor = analogRead(POT_PIN);

  // Simulamos temperatura (0–50 °C)
  float temperatura = valor * 50.0 / 1023.0;

  // -------- LCD --------
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1);
  lcd.print(" C     ");

  // -------- LOGICA --------
  lcd.setCursor(0, 1);

  if (temperatura < 25.0) {
    digitalWrite(FAN_PIN, LOW);
    ventana.write(0);

    lcd.print("Ventilador: OFF ");
    lcd.setCursor(0, 2);
    lcd.print("Ventana: Cerrada ");

  } 
  else if (temperatura < 30.0) {
    digitalWrite(FAN_PIN, HIGH);
    ventana.write(90);

    lcd.print("Ventilador: ON  ");
    lcd.setCursor(0, 2);
    lcd.print("Ventana: Semi   ");

  } 
  else {
    digitalWrite(FAN_PIN, HIGH);
    ventana.write(180);

    lcd.print("Ventilador: ON  ");
    lcd.setCursor(0, 2);
    lcd.print("Ventana: Abierta");
  }

  delay(300);
}
