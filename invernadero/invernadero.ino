#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(31, 35, 45, 47, 49, 51);

#define POT_PIN A0
#define FAN_PIN 5

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Invernadero");
  delay(1500);
  lcd.clear();
}

void loop() {
  int valor = analogRead(POT_PIN);

  // Convertimos a temperatura (0–50 °C)
  float temperatura = valor * 50.0 / 1023.0;

  // Mostrar en LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1);
  lcd.print(" C   ");

  lcd.setCursor(0, 1);
  if (temperatura >= 30.0) {
    lcd.print("Ventilador ON ");
    digitalWrite(FAN_PIN, HIGH);
  } else {
    lcd.print("Ventilador OFF");
    digitalWrite(FAN_PIN, LOW);
  }

  delay(300);
}
