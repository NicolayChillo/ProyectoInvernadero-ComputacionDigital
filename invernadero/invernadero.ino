#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>

// ==================================================
// ================= CONFIGURACION ==================
// ==================================================

// ---------------- LCD ----------------
LiquidCrystal lcd(31, 35, 45, 47, 49, 51);
Servo ventana;

// ---------------- TECLADO ----------------
const byte FILAS = 1;
const byte COLUMNAS = 3;

char teclas[FILAS][COLUMNAS] = {
  {'L', 'C', 'R'}
};

byte pinesFilas[FILAS] = {18};
byte pinesColumnas[COLUMNAS] = {19, 20, 21};

Keypad teclado = Keypad(makeKeymap(teclas),pinesFilas,pinesColumnas,FILAS,COLUMNAS);

// ---------------- PINES ----------------
#define POT_TEMP    A0
#define POT_HUM     A1
#define POT_TANK    A2
#define HORA_POT    A3
#define MIN_POT     A4

#define LUZ_PIN     3
#define RIEGO_PIN   4
#define VENTIL_PIN     5
#define SERVO_PIN   6

// ---------------- RELOJ ----------------
#define INTERVALO_SIM 50   // velocidad simulada

// ==================================================
// ================= VARIABLES =======================
// ==================================================

// Pantallas
int pantallaActual = 0;
int pantallaAnterior = -1;

// Modos
bool modoReloj = false;

// Tiempo
unsigned long tiempoAnterior = 0;
int segundos = 0;
int minutos  = 0;
int horas    = 0;

// Para refresco sin parpadeo
int segAnt  = -1;
int minAnt  = -1;
int horaAnt = -1;

// Potenciometros
int horaPotAnt = -1;
int minPotAnt  = -1;
int potTempAnt  = -1;
int potHumAnt   = -1;
int potTankAnt  = -1;

// Control
bool riegoActivo = false;
bool lucesActivas = false;

unsigned long tiempoParpadeo = 0;
bool estadoRiegoLED = false;


// ==================================================
// =============== PROTOTIPOS =======================
// ==================================================

void leerTeclado();
void actualizarReloj();
void mostrarReloj();
void limpiarPantallaSiCambia();
void leerSensores(float &t, float &h, float &n);
void mostrarPantallas(float t, float h, float n);
void ajustarReloj();
void mostrarAjusteReloj();

// ==================================================
// ================== SETUP ==========================
// ==================================================

void setup() {
  pinMode(LUZ_PIN, OUTPUT);
  pinMode(RIEGO_PIN, OUTPUT);
  pinMode(VENTIL_PIN, OUTPUT);

  ventana.attach(SERVO_PIN);

  lcd.begin(20, 4);
  lcd.clear();
}


// ==================================================
// =================== LOOP ==========================
// ==================================================

void loop() {
  leerTeclado();

  actualizarReloj();
  mostrarReloj();
  limpiarPantallaSiCambia();

  float temperatura, humedad, tanque;
  leerSensores(temperatura, humedad, tanque);
  controlarRiego(humedad, tanque);
  controlarLuces();
  controlarVentilacion(temperatura);


  if (!modoReloj) {
    mostrarPantallas(temperatura, humedad, tanque);
  } else {
    ajustarReloj();
    mostrarAjusteReloj();
  }

  delay(150);
}

// ==================================================
// ============== IMPLEMENTACIÓN =====================
// ==================================================

void leerTeclado() {
  char tecla = teclado.getKey();

  if (!tecla) return;

  if (tecla == 'C') {
    modoReloj = !modoReloj;
    pantallaAnterior = -1;
  }

  if (!modoReloj) {
    if (tecla == 'L') {
      pantallaActual = (pantallaActual + 2) % 3;
      pantallaAnterior = -1;
    }
    if (tecla == 'R') {
      pantallaActual = (pantallaActual + 1) % 3;
      pantallaAnterior = -1;
    }
  }
}

void actualizarReloj() {
  if (modoReloj) return;

  if (millis() - tiempoAnterior >= INTERVALO_SIM) {
    tiempoAnterior = millis();
    segundos++;

    if (segundos >= 60) {
      segundos = 0;
      minutos++;
    }
    if (minutos >= 60) {
      minutos = 0;
      horas++;
    }
    if (horas >= 24) {
      horas = 0;
    }
  }
}

void mostrarReloj() {
  if (segundos == segAnt && minutos == minAnt && horas == horaAnt) return;

  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  if (horas < 10) lcd.print("0");
  lcd.print(horas);
  lcd.print(":");
  if (minutos < 10) lcd.print("0");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) lcd.print("0");
  lcd.print(segundos);

  segAnt = segundos;
  minAnt = minutos;
  horaAnt = horas;
}

void limpiarPantallaSiCambia() {
  if (pantallaActual != pantallaAnterior) {
    lcd.clear();
    pantallaAnterior = pantallaActual;
    segAnt = -1;
  }
}

void leerSensores(float &temperatura, float &humedad, float &tanque) {

  int rawTemp  = analogRead(POT_TEMP);
  int rawHum   = analogRead(POT_HUM);
  int rawTank  = analogRead(POT_TANK);

  temperatura = map(rawTemp, 0, 1023, 10, 50);   // °C
  humedad     = map(rawHum,  0, 1023, 0, 100);   // %
  tanque      = map(rawTank,0, 1023, 0, 100);   // %
}


void mostrarPantallas(float t, float h, float n) {

  if (pantallaActual == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(t, 1);
    lcd.print(" C     ");

    if (t < 25) {
      digitalWrite(VENTIL_PIN, LOW);
      ventana.write(0);
      lcd.setCursor(0, 2); lcd.print("Ventilador: OFF ");
      lcd.setCursor(0, 3); lcd.print("Ventana: Cerrada");
    }
    else if (t < 30) {
      digitalWrite(VENTIL_PIN, HIGH);
      ventana.write(90);
      lcd.setCursor(0, 2); lcd.print("Ventilador: ON  ");
      lcd.setCursor(0, 3); lcd.print("Ventana: Semi   ");
    }
    else {
      digitalWrite(VENTIL_PIN, HIGH);
      ventana.write(180);
      lcd.setCursor(0, 2); lcd.print("Ventilador: ON  ");
      lcd.setCursor(0, 3); lcd.print("Ventana: Abierta");
    }
  }

  if (pantallaActual == 1) {
    lcd.setCursor(0, 1); lcd.print("Humedad Suelo:");
    lcd.setCursor(0, 2); lcd.print(h, 0); lcd.print(" %     ");
    lcd.setCursor(0, 3);
    lcd.print(h < 30 ? "Estado: Seco  " :
              h < 70 ? "Estado: Normal" :
                       "Estado: Humedo");
  }

  if (pantallaActual == 2) {
    lcd.setCursor(0, 1); lcd.print("Nivel Tanque:");
    lcd.setCursor(0, 2); lcd.print(n, 0); lcd.print(" %     ");
    lcd.setCursor(0, 3);
    lcd.print(n < 30 ? "Estado: Bajo " :
              n < 70 ? "Estado: Medio" :
                       "Estado: Lleno");
  }
}

void ajustarReloj() {
  int horaNueva = analogRead(HORA_POT) / 43;
  int minNuevo  = analogRead(MIN_POT) / 17;

  horaNueva = constrain(horaNueva, 0, 23);
  minNuevo  = constrain(minNuevo, 0, 59);

  if (horaNueva != horaPotAnt) {
    horaPotAnt = horaNueva;
    horas = horaNueva;
    segundos = 0;
    horaAnt = -1;
  }

  if (minNuevo != minPotAnt) {
    minPotAnt = minNuevo;
    minutos = minNuevo;
    segundos = 0;
    minAnt = -1;
  }
}

void mostrarAjusteReloj() {
  lcd.setCursor(0, 1);
  lcd.print("== AJUSTE RELOJ ==");
  lcd.setCursor(0, 2);
  lcd.print("Hora: ");
  if (horas < 10) lcd.print("0");
  lcd.print(horas);
  lcd.print(":");
  if (minutos < 10) lcd.print("0");
  lcd.print(minutos);
  lcd.setCursor(0, 3);
  lcd.print("Salir = R");
}

void controlarRiego(float humedad, float tanque) {

  if (humedad < 30 && tanque > 20) {
    riegoActivo = true;
  } else {
    riegoActivo = false;
  }

  if (riegoActivo) {
    digitalWrite(RIEGO_PIN, millis() % 400 < 200);
  } else {
    digitalWrite(RIEGO_PIN, LOW);
  }
}

void controlarLuces() {
  if (horas >= 6 && horas < 18) {
    digitalWrite(LUZ_PIN, HIGH);
  } else {
    digitalWrite(LUZ_PIN, LOW);
  }
}

void controlarVentilacion(float temperatura) {
  if (temperatura < 25) {
    digitalWrite(VENTIL_PIN, LOW);
    ventana.write(0);
  }
  else if (temperatura < 30) {
    digitalWrite(VENTIL_PIN, HIGH);
    ventana.write(90);
  }
  else {
    digitalWrite(VENTIL_PIN, HIGH);
    ventana.write(180);
  }
}
