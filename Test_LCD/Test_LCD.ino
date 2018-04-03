// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de librerias
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Fridge_GFX.h>                                                           // Libreria para graficos en GLCD
#include <Fridge_PCD8544.h>                                                       // Libreria PCD8544

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de pines arduino
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define LCD_LED          5                                                        // (OUTPUT)LCD backlight
#define LCD_RST         14                                                        // (IN/OUT)LCD señal RST
#define LCD_DC          15                                                        // (IN/OUT)LCD señal DC
#define LCD_DIN         16                                                        // (IN/OUT)LCD señal DIN
#define LCD_SCLK        17                                                        // (IN/OUT)LCD señal SCLK

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables especiales de librerias display
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  Fridge_PCD8544  lcd(LCD_SCLK, LCD_DIN, LCD_DC, LCD_RST);                        // LCD pin configuration: SCLK, DIN, D/C, RST. CS to GND  

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables generales
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
bool          Puls;                                                               // Pulso 
bool          Puls_FlUp;                                                          // Flanco positivo 
bool          SumaResta;                                                          // Variable para incremento o decremento
int           Contraste = 0;                                                      // Contraste

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque principal setup
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  pinMode(LCD_LED, OUTPUT);                                                       // Configuracion pin LCD backlight
  digitalWrite(LCD_LED, HIGH);                                                    // Activa pin LCD backlight
  analogWrite(LCD_LED, map(50, 0, 100, 255, 0));                                  // Activa iluminacion LCD
  Serial.begin(19200);                                                            // Inicializa puerto serie
  lcd.begin(0);                                                                   // Inicializa LCD y contraste a 0
  lcd.display();                                                                  // Actualiza bufer a LCD
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque principal
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  Blink();                                                                        // Subrutina cambio animaciones
  lcd.setContrast(map(Contraste, 0, 100, 0, 127));  
  Serial.println(Contraste);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque subrutinas y funciones de programa para gestion de menu
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Blink() {                                                                    // Subrutina pulsos de tiempo
  static unsigned long BlinkLastMillis;                                           // Variable con ultimo valor de millis()

  if (millis() - BlinkLastMillis >= 100) {
    BlinkLastMillis = millis();
    if (Contraste > 100) SumaResta = 1;
    if (Contraste <   0) SumaResta = 0;
    if (!SumaResta){
      Contraste = Contraste +1;    
    } else {
      Contraste = Contraste - 1;
    }
  }
}
