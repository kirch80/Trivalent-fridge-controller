// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Paginas de interes 
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//  Explicacion voltaje interno CPU       https://code.google.com/archive/p/tinkerit/wikis/SecretVoltmeter.wiki
//  Explicacion sensor acs712             http://www.naylampmechatronics.com/blog/48_tutorial-sensor-de-corriente-acs712.html
//  Explicacion frecuencias PWM arduino   https://arduino-info.wikispaces.com/Arduino-PWM-Frequency
//  Calculadora divisor de tension        https://www.luisllamas.es/calculadora-divisor-de-tension/
//  PCD8544 Glyph Editor                  http://carlos-rodrigues.com/pcd8544/
//  Coversor imagen a HEX                 https://www.riyas.org/2017/01/online-tool-to-convert-bitmap-to-hex-nokia-arduino.html
//  Conversor de imagen a hex             http://www.digole.com/tools/PicturetoC_Hex_converter.php
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de librerias
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <avr/wdt.h>                                                              // Libreria para gestion del watchdog
#include <Fridge_GFX.h>                                                           // Libreria para graficos en GLCD
#include <Fridge_PCD8544.h>                                                       // Libreria PCD8544
#include <Fridge_ds18b20.h>                                                       // Libreria DS18b20
#include <EEPROM.h>                                                               // Libreria EEPROM
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de pines arduino
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define ENC1             2                                                        // (INPUT)Encoder canal 1
#define ENC2             3                                                        // (INPUT)Encoder canal 2
#define EBTN             4                                                        // (INPUT)Encoder switch
#define LCD_LED          5                                                        // (OUTPUT)LCD backlight
#define STEMP0           6                                                        // (INPUT)Sensor temperatura 0, ds18b20 Nevera
#define STEMP1           7                                                        // (INPUT)Sensor temperatura 1, ds18b20 Congelador
#define STEMP2           8                                                        // (INPUT)Sensor temperatura 2, ds18b20 Ambiente
#define RES_OUT          9                                                        // (OUTPUT)Rele resistencia
#define FAN_AMB_OUT     10                                                        // (OUTPUT)Ventilador ambiente
#define FAN_NEV_OUT     11                                                        // (OUTPUT)Ventilador nevera
#define STEMP3          12                                                        // (INPUT)Sensor temperatura 3, ds18b20 Hervidor
#define BUZZER          13                                                        // (OUTPUT)Buzzer
#define LCD_RST         14                                                        // (IN/OUT)LCD señal RST
#define LCD_DC          15                                                        // (IN/OUT)LCD señal DC
#define LCD_DIN         16                                                        // (IN/OUT)LCD señal DIN
#define LCD_SCLK        17                                                        // (IN/OUT)LCD señal SCLK
#define AMPS_PIN        18                                                        // (ADC)Entrada analogica amperios
#define DOOR_SWITCH     19                                                        // (ADC)Entrada interruptor puerta
//#define FREE          20                                                        // (ADC)Disponible
#define VOLT_PIN        21                                                        // (ADC)Entrada analogica voltage DC leido
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables globales numericas
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define NONE             0                                                        // Ninguna pulsacion
#define PULSE            1                                                        // Pulsacion pulso
#define SHORT            2                                                        // Pulsacion corta
#define LONG             3                                                        // Pulsacion larga
#define L                0                                                        // Centrado texto a la izquierda
#define C                1                                                        // Centrado texto al centro
#define R                2                                                        // Centrado texto a la derecha
#define SHORTPRESS     300                                                        // Duracion minima pulsacion corta en ms
#define LONGPRESS     1000                                                        // Duracion minima pulsacion larga en ms
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables modificables por usuario
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define R1            9980                                                        // Valor resistencia R1 divisor de tension VBATT(Valor+ grande, entre VCC y pin arduino)
#define R2            3850                                                        // Valor resistencia R2 divisor de tension VBATT(Valor+ pequeño, entre GND y pin arduino)
#define ACDC_SENSOR     66                                                        // Define sensivilidad en mV/A sensor ACS712 30A
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de opciones instaladas y compiladas
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define CONGELADOR    false                                                       // Controlador con sensor congelador instalado
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables especiales de librerias sensores temperatura y display
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  Fridge_PCD8544  lcd(LCD_SCLK, LCD_DIN, LCD_DC, LCD_RST);                        // LCD pin configuration: SCLK, DIN, D/C, RST. CS to GND  
  Fridge_ds18b20  ds0(STEMP0);                                                    // Configuracion sensor temperatura nevera
  Fridge_ds18b20  ds1(STEMP1);                                                    // Configuracion sensor temperatura congelador
  Fridge_ds18b20  ds2(STEMP2);                                                    // Configuracion sensor temperatura ambiente
  Fridge_ds18b20  ds3(STEMP3);                                                    // Configuracion sensor temperatura hervidor

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de imagenes fijas en memoria FLASH
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const unsigned char TERM[]    PROGMEM = {
  0x0f, 0x00, 0x19, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80,
  0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80,
  0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x10, 0x80, 0xd0, 0x80, 0x1f, 0x80,
  0x3f, 0xc0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x3f, 0xc0, 0x1f, 0x80,
};
const unsigned char DOOR[]    PROGMEM = {
  0x0e, 0x00, 0x0f, 0xc0, 0xff, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0,
  0x8b, 0xf0, 0x8b, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0x8f, 0xf0, 0xff, 0xf0,
  0x0f, 0xc0, 0x0e, 0x00,
  };
const unsigned char SPEAKER[] PROGMEM = {
  0x04, 0x20, 0x0c, 0x90, 0x1c, 0x50, 0xfd, 0x50, 0xfd, 0x50, 0xfd, 0x50, 0xfd, 0x50, 0xfd, 0x50,
  0xfd, 0x50, 0x1c, 0x50, 0x0c, 0x90, 0x04, 0x20,
  };
const unsigned char FLECHA[]  PROGMEM = {
    B11000000
  , B11110000
  , B11111100
  , B11110000
  , B11000000
};
const unsigned char GRABAR[]  PROGMEM = {
    B00001111, B11110000
  , B00001000, B00010000
  , B00001011, B11010000
  , B00001000, B00010000
  , B00001111, B11110000
  , B00001101, B00110000
  , B00001101, B00110000
  , B00000111, B11110000
};
const unsigned char ALARMA[]  PROGMEM = {
    B00000001, B10000000
  , B00000010, B01000000
  , B00000010, B01000000
  , B00000101, B10100000
  , B00000101, B10100000
  , B00001001, B10010000
  , B00001001, B10010000
  , B00010001, B10001000
  , B00010000, B00001000
  , B00100001, B10000100
  , B00100001, B10000100
  , B01000000, B00000010
  , B01111111, B11111110
};

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de imagenes animadas en memoria FLASH
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct StructBitmap {                                                             // Struct basica para 32 bytes por bitmap
  unsigned char bitmap32 [32];                                                    // Tamaño maximo de 32 bytes
};
struct StructBitmapOffOn {                                                        // Struct para bitmaps OFF/ON
  StructBitmap BitmapOffOn[2];                                                    // Tamaño de 2 bitmaps
};

const StructBitmapOffOn FAN PROGMEM = {                                           // Inicializa struct FAN para animacion
  {                                                                     
    {
     0x03, 0x00, 0x04, 0x80, 0x08, 0x40, 0x08, 0x40, 0x04, 0x4c, 0x02, 0x52, 0x3f, 0x61, 0x41, 0xc1,
     0x83, 0x82, 0x86, 0xfc, 0x4a, 0x40, 0x32, 0x20, 0x02, 0x10, 0x02, 0x10, 0x01, 0x20, 0x00, 0xc0
    },
    {
     0x00, 0x30, 0x00, 0x48, 0x30, 0x84, 0x48, 0x84, 0x84, 0x88, 0x82, 0x90, 0x41, 0xa0, 0x3f, 0xc0,
     0x03, 0xfc, 0x05, 0x82, 0x09, 0x41, 0x11, 0x21, 0x21, 0x12, 0x21, 0x0c, 0x12, 0x00, 0x0c, 0x00
    }
  }    
};

const StructBitmapOffOn SNOW PROGMEM = {                                          // Inicializa struct SNOW para animacion
  {                                                                     
    {
     0x00, 0x00, 0x05, 0x40, 0x52, 0x94, 0x31, 0x18, 0x71, 0x1c, 0x09, 0x20, 0x85, 0x42, 0x43, 0x84,
     0xbf, 0xfa, 0x43, 0x84, 0x85, 0x42, 0x09, 0x20, 0x71, 0x1c, 0x31, 0x18, 0x52, 0x94, 0x05, 0x40
    },
    {
     0x00, 0x00, 0xa5, 0x4a, 0x62, 0x8c, 0xe1, 0x0e, 0x15, 0x50, 0x0d, 0x60, 0x9d, 0x72, 0x43, 0x84,
     0xbf, 0xfa, 0x43, 0x84, 0x9d, 0x72, 0x0d, 0x60, 0x15, 0x50, 0xe1, 0x0e, 0x62, 0x8c, 0xa5, 0x4a
    }
  }    
};

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables formato struct de parametros menus para LCD y EEPROM
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct StructMem {
  int         Mode;                                                               // Variable modo de trabajo
  int         TempSp[5];                                                          // Setpoint temperaturas
  int         RpmVentAmb;                                                         // Setpoint rpm ventilador ambiente
  int         RpmVentNev;                                                         // Setpoint rpm ventilador nevera
  int         RpmVentNevOn;                                                       // Setpoint minutos ventilador nevera on
  int         RpmVentNevOff;                                                      // Setpoint minutos ventilador nevera off
  int         EnableAlarms;                                                       // Variable alarmas habilitadas/deshabilitadas
  int         TimeDoorOpen;                                                       // Setpoint alarma tiempo puerta abierta
  int         VoltAlarmHighDC;                                                    // Setpoint alarma voltaje alto DC
  int         VoltAlarmLowDC;                                                     // Setpoint alarma voltaje bajo DC
  int         AmpsAlarmHighDC;                                                    // Setpoint alarma consumo alto DC
  int         AmpsAlarmLowDC;                                                     // Setpoint alarma consumo bajo DC
  int         AmpsAlarmHighAC;                                                    // Setpoint alarma consumo alto AC
  int         AmpsAlarmLowAC;                                                     // Setpoint alarma consumo bajo AC
  int         Brillo;                                                             // Setpoint brillo
  int         Contrast;                                                           // Setpoint contraste
  int         BackLightTime;                                                      // Setpoint tiempo backlight
  int         BackMenuTime;                                                       // Setpoint tiempo backmenu
};
union EEpromMem {                                                                 // Bloque Union para guardar datos en EEPROM
  StructMem Data;                                                                 // Struct con parametros de programa
  byte  b[sizeof(StructMem)];                                                     // Tamaño calculado datos
}
Mem;

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables parametros menus LCD
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
byte          PopUpType;                                                          // Variable pantalla popup a mostrar
byte          ScreenNum;                                                          // Variable pantalla actual
byte          LastScreenNum = 1;                                                  // Variable ultima pantalla
byte          MaxNum;                                                             // Variable numero maximo para AuxMenu
byte          TypePress = NONE;                                                   // Variable tipo de pulsacion

int           MenuIndex;                                                          // Variable MenuIndex
int           AuxMenu;                                                            // Variable AuxMenu
int           AuxMenuIndex[2];                                                    // Variable AuxMenuIndex
int           OffsetMenu;                                                         // Variable para offset textos a mostrar
int           OffsetMenuIndex[2];                                                 // Variable para array offset textos a mostrar

bool          BackLight = 1;                                                      // Indicador estado backlight
bool          EditingFlag;                                                        // Indicador estado edicion
    
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de variables generales
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool          ResetAlarms;                                                        // variable reset alarmas
bool          AlarmActiveFlag;                                                    // Variable alguna alarma activada
bool          GasLowInitFlag;                                                     // Variable temperatura minima hervidor superada
bool          GasLowAlarmFlag;                                                    // Variable alarma gas bajo
bool          VoltsBatInitFlag;                                                   // Variable lectura voltaje bateria iniciada
bool          VoltsBatAlarmFlag;                                                  // Variable alarma voltaje bateria
bool          DoorAlarmFlag;                                                      // Variable alarma puerta abierta
bool          AmpsAlarmDCFlag;                                                    // Variable alarma amperaje DC
bool          AmpsAlarmACFlag;                                                    // Variable alarma amperaje AC
bool          SelectedACDC;                                                       // Variable seleccion ACDC hecha

long          CPUmVolt;                                                           // Voltaje CPU en mV para calculos voltaje/tension
int           VoltsBat;                                                           // Variable voltaje bateria
int           AmpsACDC[2];                                                        // Variable amperios AC y DC

int           FanNevAux;                                                          // Variable auxiliar velocidad ventilador nevera
int           FanAmbAux;                                                          // Variable auxiliar velocidad ventilador ambiente

byte          OutputState;                                                        // Variable con estado salidas

int           dsTemp[] = {999,999,999,999};                                       // Array valor temperatura medida en  ºC
static byte   dsCont[] = {0,0,0,0};                                               // Array contadores de fallos consecutivos
bool          dsConfig[] = {0,0,0,0};                                             // Array indicador sensores configurados

volatile int  *EncValue = &AuxMenu;                                               // Inicializa puntero EncValue con AuxMenu
int           MaxEncValue;                                                        // Valor maximo para EncValue
int           MinEncValue;                                                        // Valor minimo para EncValue
bool          Enc1;                                                               // Variable encoder canar A
bool          Enc2;                                                               // Variable encoder canar B
bool          Units10;                                                            // Variable suma 10 unidades
  
bool          Puls_01s;                                                           // Pulso de 0.1s On y 0.1s Off
bool          Puls_05s;                                                           // Pulso de 0.5s On y 0.5s Off
bool          Puls_1s;                                                            // Pulso de 1s On y 1s Off
bool          Puls_1m;                                                            // Pulso de 1m On y 1m Off
bool          Puls_01s_FlUp;                                                      // Flanco positivo pulso de 0,1s
bool          Puls_05s_FlUp;                                                      // Flanco positivo pulso de 0,5s
bool          Puls_1s_FlUp;                                                       // Flanco positivo pulso de 1s
bool          Puls_1m_FlUp;                                                       // Flanco positivo pulso de 1m

bool          DoorOpen;                                                           // Variable puerta abierta
static int    DoorCounter;                                                        // Variable contador segundos puerta abierta

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Definicion de textos y menus LCD en memoria FLASH
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct  StructItem2 {                                                             // Struct basica para textos de unidades
  char Item [3];                                                                  // Tamaño maximo de 3 caracteres
};
struct StructUnitsItems {                                                         // Struct para unidades
  StructItem2 UnitsItems[9];                                                      // Tamaño de 8 textos
};
const StructUnitsItems Units PROGMEM = {                                          // Inicializa struct unidades
  {"", "V", "A", "C\367", "%", "v", "s", "x", "m"},                               // Extended ASCII characters like \xxx in octal format
};

struct  StructItem12 {                                                            // Struct basica para textos de una linea
  char Item [12];                                                                 // Tamaño de 12 caracteres
};
struct StructTextItems {                                                          // Struct para textos generales
  StructItem12 TextItems[19];                                                     // Tamaño de X textos
};
const StructTextItems Text PROGMEM = {                                            // Inicializa struct textos generales
  {"PARAMETROS", 
   "DE FABRICA", 
   "CARGADOS!", 
   "GUARDADOS!", 
   "ERROR", 
   "PV", 
   "SP", 
   "ALARMA", 
   "ACTIVA!", 
   "REALIZANDO", 
   "RESET", 
   "MODO DE FRIO",
   "MODO OFF", 
   "MODO GAS", 
   "MODO ECO",  
   "MODO NORMAL",
   "INACTIVA",
   "ACTIVA",
   "V.BAT"},
};

struct StructParam {                                                              // Struct para parametros
  char Text [12];                                                                 // Texto de 12 caracteres
  byte Unit;                                                                      // Unidades de 2 caracteres
  int  MaxValue;                                                                  // Valor maximo
  int  MinValue;                                                                  // Valor minimo
  byte ConfigByte;                                                                // Valor en binario para configuracion parametro
  int  *ParamPointer;                                                             // Valor puntero a parametro a controlar
};  
struct StructMenu {                                                               // Struct para configuracion menu display
  char              Version[12];                                                  // Texto para version de software
  char              Titulo[12];                                                   // Texto para titulo menu principal
  StructItem12      SubMenu[4];                                                   // Textos para menu principal
  byte              Lineas[5];                                                    // Lineas usadas por submenu
  StructParam       Param[4][8];                                                  // Textos para parametros
};
// Significado bits ConfigByte:
// Bit 0 = Permiso de escritura para edicion parametro, 0 = sin permiso
// Bit 1 = Mostrar barra grafica
// Bit 2 = Mostrar numero
// Bit 3 = Mostrar punto decimal
// Bit 4 = Mostrar check box habilitacion
// Bit 5 = Disponible
// Bit 6 = Disponible
// Bit 7 = Disponible
//
const StructMenu Menu PROGMEM = {                                                 // Inicializa struct Menu con textos
  {"A1.00v"},                                                                     // Inicializa texto version de software 
  {"MENU CONFIG."},                                                               // Inicializa texto titulo 
  {"TEMPERATURAS","VENTILADORES","ALARMAS","DISPLAY"},                            // Inicializa textos menu
  { 3, 3 + CONGELADOR, 3, 7, 3},                                                  // Inicializa numero de lineas por menu/submenu -1
  {
    {                                                                             // Inicializa submenu "TEMPERATURAS"
      {"NEVERA"      ,  3,  100,    0, B00001111, &Mem.Data.TempSp[0]},           // SP nevera 
      # if CONGELADOR                                                             // Si tengo sensor congelador 
      {"CONGELADOR"  ,  3,    0, -150, B00001111, &Mem.Data.TempSp[1]},           // SP congelador
      #endif
      {"AMBIENTE"    ,  3,  600,  250, B00001111, &Mem.Data.TempSp[2]},           // SP ambiente
      {"HERVIDOR"    ,  3, 1150,  600, B00001111, &Mem.Data.TempSp[3]},           // SP hervidor
      {"STANDBY"     ,  3, 1150,  600, B00001111, &Mem.Data.TempSp[4]},           // SP standby
    },
    {                                                                             // Inicializa submenu "VENTILADORES"
      {"VENT1 MAX."  ,  4,  100,   20, B00000111, &Mem.Data.RpmVentAmb},          // SP % velocidad ventilador ambiente
      {"VENT2 MAX."  ,  4,  100,   20, B00000111, &Mem.Data.RpmVentNev},          // SP % velocidad ventilador nevera
      {"VENT2 T.ON." ,  8,   10,    0, B00000111, &Mem.Data.RpmVentNevOn},        // SP minutos ventilador nevera on
      {"VENT2 T.OFF" ,  8,   10,    0, B00000111, &Mem.Data.RpmVentNevOff},       // SP minutos ventilador nevera off
    },
    {                                                                             // Inicializa submenu "ALARMAS"
      {"PUERTA"      ,  6,   99,    0, B00010111, &Mem.Data.TimeDoorOpen},        // Alarma tiempo puerta abierta
      {"GAS BAJO"    ,  3, 1150,  600, B00011111, &Mem.Data.TempSp[4]},           // Alarma gas bajo por poca temperatura hervidor
      {"VOLTS BAT+"  ,  1,  150,   90, B00011111, &Mem.Data.VoltAlarmHighDC},     // Alarma bateria +
      {"VOLTS BAT-"  ,  1,  150,   90, B00011111, &Mem.Data.VoltAlarmLowDC},      // Alarma bateria -
      {"AMPS DC+"    ,  2,  200,    0, B00011111, &Mem.Data.AmpsAlarmHighDC},     // Alarma DC +
      {"AMPS DC-"    ,  2,  200,    0, B00011111, &Mem.Data.AmpsAlarmLowDC},      // Alarma DC -
      {"AMPS AC+"    ,  2,  100,    0, B00011111, &Mem.Data.AmpsAlarmHighAC},     // Alarma AC +
      {"AMPS AC-"    ,  2,  100,    0, B00011111, &Mem.Data.AmpsAlarmLowAC},      // Alarma AC -
    },
    {                                                                             // Inicializa submenu "DISPLAY"
      {"BRILLO"      ,  4,  100,    0, B00000111, &Mem.Data.Brillo},              // Valor brillo 
      {"CONTRASTE"   ,  4,  100,    0, B00000111, &Mem.Data.Contrast},            // Valor contraste 
      {"BACKLIGHT"   ,  6,   60,    0, B00000111, &Mem.Data.BackLightTime},       // Valor tiempo backlight 
      {"RETROCESO"   ,  6,   60,    0, B00000111, &Mem.Data.BackMenuTime},        // Valor tiempo retroceso menus
    }
  }
};

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque principal setup
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  wdt_disable();                                                                  // Deshabilita watchdog durante configuracion
  pinMode(ENC1, INPUT_PULLUP);                                                    // Turn on pullup resistor
  pinMode(ENC2, INPUT_PULLUP);                                                    // Turn on pullup resistor
  pinMode(EBTN, INPUT_PULLUP);                                                    // Turn on pullup resistor
  pinMode(DOOR_SWITCH, INPUT_PULLUP);                                             // Turn on pullup resistor
  pinMode(LCD_LED, OUTPUT);                                                       // Configuracion pin LCD backlight
  digitalWrite(LCD_LED, HIGH);                                                    // Activa pin LCD backlight
  pinMode(RES_OUT, OUTPUT);                                                       // Configuracion pin rele resistencia
  digitalWrite(RES_OUT, LOW);                                                     // Desactiva salida resistencia
  pinMode(FAN_AMB_OUT, OUTPUT);                                                   // Configuracion pin rele ventilador ambiente
  digitalWrite(FAN_AMB_OUT, LOW);                                                 // Desactiva salida ventilador ambiente
  pinMode(BUZZER, OUTPUT);                                                        // Configuracion pin buzzer
  digitalWrite(BUZZER, LOW);                                                      // Desactiva salida buzzer
  pinMode(FAN_NEV_OUT, OUTPUT);                                                   // Configuracion pin rele ventilador nevera
  digitalWrite(FAN_NEV_OUT, LOW);                                                 // Desactiva salida ventilador nevera
  attachInterrupt(0, intrEncChange1, CHANGE);                                     // Configuracion interrupcion encoder canal A
  attachInterrupt(1, intrEncChange2, CHANGE);                                     // Configuracion interrupcion encoder canal B
  FactoryReset();                                                                 // Subrutina valores de fabrica si encoder esta pulsado al inicio
  analogWrite(LCD_LED, map(Mem.Data.Brillo, 0, 100, 255, 0));                     // Activa iluminacion LCD
  lcd.begin(map(Mem.Data.Contrast, 0, 100, 0, 127));                              // Inicializa LCD y contraste
  lcd.display();                                                                  // Actualiza bufer a LCD
  delay(2000);                                                                    // Retardo de 2s para mostrar version y modelo
  Serial.begin(19200);                                                            // Inicializa puerto serie
  wdt_enable(WDTO_1S);                                                            // Habilita whatchdog a 1s
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque principal interrupciones
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void intrEncChange1() {
  delay(1);                                                                       // Retardo para evitar rebotes
  if (digitalRead(ENC1) == Enc1) return;                                          // Si ENC1 no ha cambiado, fin interrupcion
  Enc1 = !Enc1;                                                                   // Invierte valor Enc1
  if (Enc1 && !Enc2) {                                                            // Si detecta incremento
    if ((Units10) && ((*EncValue >= 1000) || (*EncValue < -100))){                // Si valor mayor o igual a 100.0 o menor o igual a -10.0
      *EncValue += 10;                                                            // Incrementa +10 valor puntero
    } else {
      *EncValue += 1;                                                             // Incrementa +1 valor puntero       
    }
    Units10 = 0;                                                                  // Reset incrementos de 10
    BackLight = 1;                                                                // Activa backlight
  }
    CheckMinMaxEnc();                                                             // Comprueba limites 
}

void intrEncChange2() {
  delay(1);                                                                       // Retardo para evitar rebotes
  if (digitalRead(ENC2) == Enc2) return;                                          // Si ENC2 no ha cambiado, fin interrupcion                                              
  Enc2 = !Enc2;                                                                   // Invierte valor Enc2
  if (Enc2 && !Enc1) {                                                            // Si detecta decremento
    if ((Units10) && ((*EncValue > 1000) || (*EncValue <= -100))){                // Si valor menor a 100.0 o menor o igual a -10.0
      *EncValue -= 10;                                                            // Decrementa -10 valor puntero
    } else {
      *EncValue -= 1;                                                             // Decrementa -1 valor puntero       
    }
    Units10 = 0;                                                                  // Reset incrementos de 10 
    BackLight = 1;                                                                // Activa backlight
  }
    CheckMinMaxEnc();                                                             // Comprueba limites
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque principal
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  GetCPUVolt();                                                                   // Subrutina lectura voltaje CPU como ref. analogicas
  GetVoltBat();                                                                   // Subrutina lectura voltaje bateria
  GetAmps();                                                                      // Subrutina lectura amperios 
  GetDS18b20();                                                                   // Subrutina lectura temperatura sensores ds18b20
  Blink();                                                                        // Subrutina cambio animaciones
  BackLightMenuTimer();                                                           // Subrutina control Backlight y retroceso menus
  Pinta();                                                                        // Subrutina pinta LCD
  Alarmas();                                                                      // Subrutina gestion alarmas
  Salidas();                                                                      // Subrutina control salidas
  SerialPlotter();                                                                // Subrutina para graficas
  wdt_reset();                                                                    // Reset watchdog
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque subrutinas y funciones de programa para gestion de menu
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void Pinta() {                                                                    // Subrutina pinta LCD

  lcd.clearDisplay();                                                             // Limpia bufer y pantalla
  switch (ScreenNum) {                                                            // Comprueba pantalla a mostrar
    case 0:                                                                       // Pantalla de pop up
      switch (PopUpType) {                                                        // Selecciona tipo de pantalla pop up a mostrar
        case 0:                                                                   // Pantalla de inicio
          DrawString (C, &Menu.Version, 1, 0, 0, 20, false);                      // Escribe version de software
        break;

        case 1:                                                                   // Tipo pop up "PARAMETROS GUARDADOS"
          lcd.drawBitmap(35, 30, GRABAR, 16, 8, BLACK);                           // Pinta imagen
          lcd.drawRoundRect(0, 0, 83, 48, 5, BLACK);                              // Pinta marco pantalla popup
          DrawString (C, &Text.TextItems[0], 1, 0, 0, 5, false);                  // Escribe linea 1
          DrawString (C, &Text.TextItems[3], 1, 0, 0, 15, false);                 // Escribe linea 2
        break;
      
        case 2:                                                                   // Tipo pop up "PARAMETROS DE FABRICA CARGADOS!"
          lcd.drawBitmap(35, 35, GRABAR, 16, 8, BLACK);                           // Pinta imagen
          lcd.drawRoundRect(0, 0, 83, 48, 5, BLACK);                              // Pinta marco pantalla popup
          DrawString (C, &Text.TextItems[0], 1, 0, 0, 5, false);                  // Escribe linea 1
          DrawString (C, &Text.TextItems[1], 1, 0, 0, 15, false);                 // Escribe linea 2
          DrawString (C, &Text.TextItems[2], 1, 0, 0, 25, false);                 // Escribe linea 3
        break;  

        case 3:                                                                   // Tipo pop up "ALARMA ACTIVA!, CONFIRME RESET"
          lcd.drawBitmap(35, 33, ALARMA, 16, 13, BLACK);                          // Dibuja bitmap simbolo alarma   
          lcd.drawRoundRect(0, 0, 83, 48, 5, BLACK);                              // Pinta marco pantalla popup
          DrawString (L, &Text.TextItems[7] , 1, 3, 0,  5, false);                // Escribe linea 1
          DrawString (R, &Text.TextItems[8], 1, 0, 1,  5, false);                 // Escribe linea 1
          DrawString (C, &Text.TextItems[9], 1, 0, 0, 15, false);                 // Escribe linea 2
          DrawString (C, &Text.TextItems[10], 1, 0, 0, 25, false);                // Escribe linea 3
        break;  
      }
      PopUpTimer(LastScreenNum);                                                  // Cambia a pantalla indicada despues del tiempo seleccionado
      break;
      
    case 1:                                                                       // Pantalla principal
      ScreenMainConfig(2,3);                                                      // Configura pantalla principal
    break;

    case 2:                                                                       // Pantalla menu configuracion
      ScreenMnuConfig(1,3);                                                       // Configura pantalla menus
    break;

    case 3:                                                                       // Pantalla menu seleccion modo
      ScreenModeConfig(1,3);                                                      // Configura pantalla modo
    break;

  }
  lcd.display();                                                                  // Actualiza bufer a LCD
}

bool ScreenControl(int LongScreen, int ShortScreen, int MaxMenuIndex, bool RstAuxMenu){ // Subrutina control movimientos en pantalla

  static unsigned long btnHeld = 0;
  static bool btnPressed = 0;
  long AuxLongValue;                                                              // Variable auxiliar para valores long

  if ((!btnHeld) && (!btnPressed)) {                                              // Si es el primer ciclo de scan una vez pulsado encoder
    if (digitalRead(EBTN) == LOW) {                                               // Si pulso encoder
      btnHeld = millis();                                                         // Guarda milis actuales
      BackLight = 1;                                                              // Activa BackLight
    }
  } 
  
  if ((btnHeld) && (!btnPressed)) {                                               // Si no he detectado tipo de pulsacion todavia
    AuxLongValue = millis() - btnHeld;                                            // Calcula tiempo transcurrido desde pulsacion
    if ((digitalRead(EBTN) == HIGH) || (AuxLongValue > LONGPRESS)) {              // Si he soltado pulsador encoder o tiempo maximo transcurrido
      if (AuxLongValue < LONGPRESS) {                                             // Si tiempo transcurrido es menor que LONGPRESS
        if (AuxLongValue > SHORTPRESS) {                                          // Si tiempo transcurrido es mayor que SHORTPRESS
          TypePress = SHORT;                                                      // pulsacion corta
        } else {
          TypePress = PULSE;                                                      // pulsacion pulso    
        }
      } else {                                                                    // Si tiempo transcurrido es mayor que LONGPRESS
        TypePress = LONG;                                                         // pulsacion larga
      }
      btnHeld = 0;                                                                // Reset variable milis guardados
      btnPressed = 1;                                                             // Set variable btnPressed
    }
  }
  
  if ((btnPressed) && (digitalRead(EBTN) == HIGH)) btnPressed = 0;                // Si ya he detectado tipo de pulsacion
  if ((TypePress == LONG) && (ScreenNum == 1) && (!EditingFlag) && AlarmActiveFlag) { //Si tengo alarmas y estoy en pantalla principal
    TypePress = NONE;                                                             // Reset tipo pulsacion
    LastScreenNum = ScreenNum;                                                    // Memoriza pantalla actual
    ScreenNum = 0;                                                                // Selecciona pantalla tipo popup
    PopUpType = 3;                                                                // Selecciona popup reset alarmas
    ResetAlarms = 1;                                                              // Reset alarmas
    return 1;                                                                     // Devuelve salida subrutina
  }

    if (TypePress == SHORT) {                                                     // En caso de pulsacion corta
    TypePress = NONE;                                                             // Reset tipo pulsacion
    if (ScreenNum == 1) {                                                         // Pantalla principal
      CheckMinMaxToEnc(&Mem.Data.Mode, 0, 3);                                     // Cargo y configuro limites encoder
      LastScreenNum = ScreenNum;                                                  // Memoriza pantalla actual
      ScreenNum = ShortScreen;                                                    // Carga pantalla configurada ShortScreen
      EditingFlag = 1;                                                            // Cambia estado de bit Editing
      return 1;                                                                   // Devuelve salida subrutina   
    } else {
      TypePress = PULSE;                                                          // Pulsacion corta    
    }
  }

  if (TypePress == PULSE) {                                                       // En caso de pulso
    TypePress = NONE;                                                             // Reset tipo pulsacion
    if (MenuIndex < MaxMenuIndex) {                                               // Si MenuIndex < MaxMenuIndex
      AuxMenuIndex[MenuIndex] = AuxMenu;                                          // Salvo AuxMenu en la posicion del array AuxMenuIndex
      OffsetMenuIndex[MenuIndex] = OffsetMenu;                                    // Salvo OffsetMenu en la posicion del array OffsetMenuIndex
    }
    MenuIndex++;                                                                  // Incrementa MenuIndex
    if (MenuIndex > MaxMenuIndex) {                                               // Si MenuIndex > MaxMenuIndex
      MenuIndex = MaxMenuIndex;                                                   // MenuIndex = MaxMenuIndex
      EditingFlag = !EditingFlag;                                                 // Cambia estado de bit Editing
      if (!EditingFlag) {                                                         // Si hay edicion
        for (int i = 0 ; i < sizeof(StructMem) ; i++) EEPROM.put(i, Mem.b[i]);    // Actualiza valores en EEPROM si hay valores diferentes
        LastScreenNum = ScreenNum;                                                // Memoriza pantalla actual
        PopUpType = 1;                                                            // Configura pantalla parametros guardados
        ScreenNum = 0;                                                            // Selecciona pantalla popup
        return 1;                                                                 // Devuelve salida subrutina
      }
    }
    if (RstAuxMenu) {                                                             // Si reset AuxMenu
      if (!EditingFlag) AuxMenu = 0;                                              // Inicializa AuxMenu si no tengo EditingFlag
      OffsetMenu = 0;                                                             // Inicializa OffsetMenu
    }
  }
 
  if (TypePress == LONG) {                                                        // En caso de pulsacion larga
    TypePress = NONE;                                                             // Reset tipo pulsacion
    MenuIndex--;                                                                  // Decremento MenuIndex
    if (MenuIndex < 0) {                                                          // Si MenuIdex < 0, sale a pantalla principal
      MenuIndex = 0;                                                              // Inicializa MenuIndex
      ScreenNum = LongScreen;                                                     // Carga pantalla configurada LongScreen
      AuxMenu = 0;                                                                // Inicializa AuxMenu
      OffsetMenu = 0;                                                             // Inicializa OffsetMenu
      return 1;                                                                   // Devuelve salida subrutina
    }
    if (RstAuxMenu) {                                                             // Si reset AuxMenu
      AuxMenu = AuxMenuIndex[MenuIndex];                                          // Recupero AuxMenu de la posicion del array AuxMenuIndex
      OffsetMenu = OffsetMenuIndex[MenuIndex];                                    // Recupero OffsetMenu de la posicion del array OffsetMenuIndex
    }
    AuxMenuIndex[MenuIndex] = 0;                                                  // Resetea posicion actual del array AuxMenuIndex
    OffsetMenuIndex[MenuIndex] = 0;                                               // Resetea posicion actual del array OffsetMenuIndex
    EditingFlag = 0;                                                              // Reset edicion
  }
  return 0;                                                                       // Devuelve continuar con subrutina
}                                              

void ScreenMainConfig(int LongScreen, int ShortScreen) {                          // Subrutina configuracion pantalla menu
  struct StructParam AuxSens;                                                     // Struct auxiliar para lectura rapida
  int dsTempAux;                                                                  // Variable auxiliar numero sensor a mostrar
  int AuxMap;                                                                     // Variable auxiliar para max-min termometro
  int i;                                                                          // Variable contador generico
  
  if (ScreenControl(LongScreen, ShortScreen, 1, false)) return;                   // Subrutina control movimientos en pantalla

  #if CONGELADOR                                                                  // Si tengo sensor congelador
    dsTempAux = AuxMenu;                                                          // Copio temperatura sensor a auxMap
  #else
    if (AuxMenu > 0) {                                                            // Si AuxMenu > 0
      dsTempAux = AuxMenu + 1;                                                    // Copio temperatura sensor + 1 a auxMap        
    } else {
      dsTempAux = AuxMenu;                                                        // Copio temperatura sensor a auxMap    
    }
  #endif

  if (!AlarmActiveFlag && !DoorOpen) {                                            // Si no tengo ni alarmas ni puerta abierta
    lcd.drawBitmap(71, 1, TERM, 12, 30, BLACK);                                   // Dibuja bitmap termometro
    memcpy_P (&AuxSens, &Menu.Param[0][AuxMenu], sizeof AuxSens);                 // Copia desde memoria FLASH sensor seleccionado  
    AuxMap = dsTemp[dsTempAux];                                                   // Variable auxiliar para termometro
    CheckMinMax(&AuxMap, AuxSens.MinValue, AuxSens.MaxValue);                     // Compruebo limites para visualizar temperatura en termometro
    lcd.fillRect(75, 25, 4, map(AuxMap, AuxSens.MinValue, AuxSens.MaxValue, 0, -22), BLACK);// Relleno termometro dependiendo valores            
    
  } else {
    if ((AlarmActiveFlag || DoorAlarmFlag) && Puls_05s) lcd.drawBitmap(71, 0, SPEAKER, 12, 12, BLACK);// Si tengo alarmas, dibuja bitmap altavoz
    if (!AlarmActiveFlag){                                                        // Si no tengo alarmas
      if (DoorOpen) {                                                             // Si puerta abierta
        if (!DoorAlarmFlag) DrawInt (C, int(Mem.Data.TimeDoorOpen - DoorCounter), &Units.UnitsItems[0], 1, 65, 4, false, false); // Si no tengo alarmas, escribe tiempo restante  
        lcd.drawBitmap(71, 13, DOOR, 12, 18, BLACK);                              // Dibuja bitmap puerta
      }
    } else {                                                                      // Si tengo alarmas
      lcd.drawBitmap(69, 15, ALARMA, 16, 13, BLACK);                              // Dibuja bitmap simbolo alarma 
    }
  }
                                              
  if (((Mem.Data.Mode >= 0) && (Mem.Data.Mode <= 2)) && (MenuIndex == 0) && Puls_1s){ // Si tengo modo OFF, GAS o ECO y pulso 1s solo en pantalla principal
    DrawString (C, &Text.TextItems[12 + Mem.Data.Mode], 1, 0, 12, 1, true);       // Pinta texto OFF, GAS o ECO invertido
  } else {
    DrawString (C, &Menu.Param[0][AuxMenu].Text, 1, 0, 12, 1, false);             // Pinta texto sensor seleccionado
  }  
  
  if (!dsConfig[dsTempAux]) {                                                     // Si no tengo error de lectura sensor de temperatura
    DrawString (C, &Text.TextItems[4], 2, 0, 12, 14, Puls_1s);                    // Escribe texto "Error!" 
  } else {                                                                        // Si tengo error de lectura sensor de temperatura
    if (dsTemp[dsTempAux] <= 999) {                                               // Comprueba si temperatura es menor de 4 digitos
      DrawInt (C, dsTemp[dsTempAux], &Units.UnitsItems[0], 3, 0, 10, true, false);// Escribe temperatura con un decimal
    } else {
      DrawInt (C, (dsTemp[dsTempAux] / 10), &Units.UnitsItems[0], 3, 0, 10, false, false); // Escribe temperatura sin decimales      
    }
  }
  CheckMinMaxToEnc(&AuxMenu, 0, 2 + CONGELADOR);                                  // Cargo y configuro limites encoder

  if (MenuIndex == 0) {                                                           // Si MenuIdex = 0, muestra pantalla normal
    DrawInt (C, AmpsACDC[SelectedACDC], &Units.UnitsItems[2], 1, 1, 32, true, ((AmpsAlarmDCFlag || AmpsAlarmACFlag) && Puls_05s)); // Dibuja consumo en DC       
    DrawInt (C, VoltsBat, &Units.UnitsItems[1], 1, 1, 40, true, (VoltsBatAlarmFlag && Puls_05s)); // Dibuja voltaje bateria 
    
    lcd.drawBitmap( 33, 32,  FAN.BitmapOffOn[(FanNevAux > 0 && Puls_05s)].bitmap32, 16, 16, BLACK);        // Pinta bitmap
    lcd.drawBitmap( 51, 32,  SNOW.BitmapOffOn[(digitalRead(RES_OUT) && Puls_05s)].bitmap32, 16, 16, BLACK);// Pinta bitmap 
    lcd.drawBitmap( 68, 32,  FAN.BitmapOffOn[(FanAmbAux > 0 && Puls_05s)].bitmap32, 16, 16, BLACK);        // Pinta bitmap 
  }

  if (MenuIndex == 1) {                                                           // Si MenuIdex = 1, muestra pantalla edicion

    memcpy_P (&AuxSens, &Menu.Param[0][AuxMenu], sizeof AuxSens);                 // Copia desde memoria FLASH    
    if (!bitRead(AuxSens.ConfigByte, 0)) EditingFlag = 0;                         // Si no tengo permiso de escritura, reset EditingFlag
    if (EditingFlag) {                                                            // Si tengo edicion
      CheckMinMaxToEnc(AuxSens.ParamPointer, AuxSens.MinValue, AuxSens.MaxValue); // Cargo y configuro limites encoder
    }     
    DrawString (L, &Text.TextItems[6], 2, 0, 0, 33, false);                       // Escribe texto "SP"
    DrawInt (R, *AuxSens.ParamPointer, &Units.UnitsItems[AuxSens.Unit], 2, 24, 33, true, EditingFlag); // Escribe consigna
  }  
}

void ScreenMnuConfig(int LongScreen, int ShortScreen) {                           // Subrutina configuracion pantalla menu
  struct      StructParam AuxSens;                                                // Struct auxiliar para lectura rapida
  int         DrawOffset;                                                         // Offset cursor dependiendo campos a dibujar
  int         i;                                                                  // Variable contador generico
  static int  AuxEnable;                                                          // Variable auxiliar 2 para opcion enable

  if (ScreenControl(LongScreen, ShortScreen, 2, true)) return;                    // Subrutina control movimientos en pantalla
  
  lcd.drawFastHLine(3, 8, 74, BLACK);                                             // Escribe linea debajo itulo menu

  if ((MenuIndex == 0) || (MenuIndex == 1)) {                                     // Si MenuIdex = 0 o 1, muestra menu o submenu
    memcpy_P (&MaxNum, &Menu.Lineas[AuxMenuIndex[0] + MenuIndex], 1);             // Lee numero maximo de lineas submenu
    CheckMinMaxToEnc(&AuxMenu, 0, MaxNum);                                        // Cargo y configuro limites encoder

    if (MenuIndex == 0){                                                          // Si MenuIdex = 0, muestra menu
      DrawString (C, &Menu.Titulo, 1, 0, 4, 0, false);                            // Escribe texto titulo menu
      for (i = 0; i <= MaxNum - OffsetMenu; i++) DrawString (L, &Menu.SubMenu[i + OffsetMenu], 1, 7, 0, (i * 10) + 11, false); // Bucle para escribir menu
    }
    if (MenuIndex == 1){                                                          // Si MenuIdex = 1, muestra submenu
      DrawString (C, &Menu.SubMenu[AuxMenuIndex[0]], 1, 0, 0, 0, false);          // Pinta texto titulo submenu
      for (i = 0; i <= MaxNum - OffsetMenu; i++) DrawString (L, &Menu.Param[AuxMenuIndex[0]][i + OffsetMenu].Text, 1, 7, 0, (i * 10) + 11, false);  // Bucle para escribir submenu
    }
    CheckDispBar();                                                               // Subrutina comprueba si muestra barra desplazamiento y offset
  }
 
  if (MenuIndex == 2) {                                                           // Si MenuIdex = 2, muestra parametros
    DrawOffset = 0;                                                               // Inicializa Offset para corregir posicion
    DrawString (C, &Menu.Param[AuxMenuIndex[0]][AuxMenuIndex[1]].Text, 1, 0, 0, 0, false); // Escribe titulo parametro
    memcpy_P (&AuxSens, &Menu.Param[AuxMenuIndex[0]][AuxMenuIndex[1]], sizeof AuxSens); // Copia desde memoria FLASH

    bool BitEnablPar  = bitRead(AuxSens.ConfigByte, 0);                           // Bit 0 = Permiso de escritura para edicion parametro
    bool BitBarGraph  = bitRead(AuxSens.ConfigByte, 1);                           // Bit 1 = Mostrar barra grafica
    bool BitDisplNum  = bitRead(AuxSens.ConfigByte, 2);                           // Bit 2 = Mostrar numero
    bool BitDecPoint  = bitRead(AuxSens.ConfigByte, 3);                           // Bit 3 = Mostrar punto decimal
    bool BitCheckBox  = bitRead(AuxSens.ConfigByte, 4);                           // Bit 4 = Mostrar checkbox

    if (!BitEnablPar) EditingFlag = 0;                                            // Si no tengo permiso de escritura, reset EditingFlag
    if (EditingFlag && ((!BitCheckBox) || (BitCheckBox && AuxMenu == 1))) {       // Si tengo edicion y no checkbox o tengo edicion, checkbox y AuxMenu = 0
      CheckMinMaxToEnc(AuxSens.ParamPointer, AuxSens.MinValue, AuxSens.MaxValue); // Cargo y configuro limites encoder
    }

    if (BitBarGraph) {                                                            // Si BarrGraph on
      DrawOffset = DrawOffset - 3;                                                // Ajusta Offset para corregir posicion
      lcd.drawRect(0, 40, 84, 8, BLACK);                                          // Pinta linea
      lcd.fillRect(0, 40, map(*AuxSens.ParamPointer, AuxSens.MinValue, AuxSens.MaxValue, 0, 84), 8, BLACK);// Pinta barra grafica
    }

    if (BitDisplNum) {                                                            // Si tengo valor numerico a mostrar
      if (BitCheckBox) DrawOffset = DrawOffset + 4;                               // En caso de tener que mostrar checklist, ajusta Offset para corregir posicion
      DrawString (L, &Text.TextItems[5 + BitEnablPar], 2, 0, 0, 20 + DrawOffset, false); // Escribe SP o PV
      DrawInt (R, *AuxSens.ParamPointer, &Units.UnitsItems[AuxSens.Unit], 2, 24, 20 + DrawOffset, BitDecPoint, (EditingFlag && ((!BitCheckBox) || (BitCheckBox && AuxMenu == 1)))); // Si tengo punto decimal   
    }

    if (BitCheckBox) {                                                            // Si tengo checklist a mostrar
      if (BitDisplNum) DrawOffset = DrawOffset - 13;                              // En caso de tener que mostrar numero, ajusta Offset para corregir posicion
      lcd.drawRoundRect(8, 22 + DrawOffset, 9, 9, 1, BLACK);                      // Pinta borde checklist
      DrawString (L, &Units.UnitsItems[7 * bitRead(Mem.Data.EnableAlarms, AuxMenuIndex[1])], 1, 10, 0, 22 + DrawOffset, false); // Escribe x checkbox 
      DrawString (L, &Text.TextItems[16 + (bitRead(Mem.Data.EnableAlarms, AuxMenuIndex[1]))], 1, 20, 0, 23 + DrawOffset, ((AuxMenu == 0) && EditingFlag)); // Escribe enable/disable
      if (!EditingFlag) {                                                         // Si no tengo edicion
        CheckMinMaxToEnc(&AuxMenu, 0, 1);                                         // Cargo y configuro limites encoder
        if (AuxMenu == 0) lcd.drawFastHLine(18, 19, 52, BLACK);                   // Si no tengo edicion y AuxMenu = 0, escribe linea bajo checklist 
        if (AuxMenu == 1) lcd.drawFastHLine(23, 36, 54, BLACK);                   // Si no tengo edicion y AuxMenu = 1, escribe linea bajo parametro                                           //
      }  
      if (EditingFlag && AuxMenu == 0) {                                          // Si tengo edicion de checklist
        CheckMinMaxToEnc(&AuxEnable, 0, 1);                                       // Cargo y configuro limites encoder
        bitWrite(Mem.Data.EnableAlarms, AuxMenuIndex[1], AuxEnable);              // Escribe estado de alarma habilitada/deshabilitada
      }
    }  

    if (EditingFlag && ((((AuxMenuIndex[0]) * 10) + (AuxMenuIndex[1])) == 31)) {  // Si tengo edicion y estoy en menu contraste
      lcd.setContrast(map(*AuxSens.ParamPointer, 0, 100, 0, 127));                // Configura contraste LCD
    }
 
    return;
  }
}

void ScreenModeConfig(int LongScreen, int ShortScreen) {                          // Subrutina configuracion pantalla mode
  int i;                                                                          // Variable contador generico

  if (ScreenControl(LongScreen, ShortScreen, 0, true)) return;                    // Subrutina control movimientos en pantalla
  
  lcd.drawFastHLine(3, 8, 74, BLACK);                                             // Escribe linea
  DrawString (C, &Text.TextItems[11], 1, 0, 0, 0, false);                         // Escribe texto MODO
  for (i = 0; i < 4; i++) DrawString (C, &Text.TextItems[12 + i], 1, 0, 0, 10 + (10 * i), (Mem.Data.Mode == i) && Puls_05s); // Escribe texto;
}

void CheckMinMax(int *AuxInt, int Min, int Max) {                                 // Subrutina control maximos y minimos parametros

  if (*AuxInt < Min) *AuxInt = Min;                                               // Si AuxInt < Min, AuxInt = Min
  if (*AuxInt > Max) *AuxInt = Max;                                               // Si AuxInt > Max, AuxInt = Max  
}

void CheckMinMaxToEnc(int AuxInt, int Min, int Max) {                             // Subrutina control maximos y minimos con parametros

  EncValue = AuxInt;                                                              // Paso valor a encoder
  MinEncValue = Min;                                                              // Valor minimo encoder
  MaxEncValue = Max;                                                              // Valor maximo encoder
  CheckMinMaxEnc();                                                                // Compuebo limites valor encoder
}

void CheckMinMaxEnc() {                                                           // Subrutina control maximos y minimos encoder

  if (*EncValue < MinEncValue) *EncValue = MinEncValue;                           // Si EncValue < MinEncValue, EncValue = MinEncValue
  if (*EncValue > MaxEncValue) *EncValue = MaxEncValue;                           // Si EncValue > MaxEncValue, EncValue = MaxEncValue  
}

void CheckDispBar() {                                                             // Subrutina control barra de desplazamiento

  if (((AuxMenu - OffsetMenu) < 0) && (OffsetMenu > 0)) OffsetMenu--;             // Si cursor fuera de pantalla por abajo, decrementa OffsetMenu
  if ((AuxMenu - OffsetMenu) > 3) OffsetMenu++;                                   // Si cursor fuera de pantalla por arriba, incrementa OffsetMenu
  lcd.drawBitmap(0, ((AuxMenu - OffsetMenu) * 10) + 12, FLECHA, 8, 5, BLACK);     // Pinta flecha de seleccion en el cursor
  if (MaxNum > 3) {                                                               // Si tengo mas de 4 campos a mostrar,
    lcd.drawRect(80, 0, 4, 48, BLACK);                                            // Muestra borde barra de desplazamiento
    lcd.drawRect(81, map(AuxMenu, 0, MaxNum, 1, 24), 2, 24, BLACK);               // Muestra barra de desplazamiento
  }
}

void DrawString (int CenterType, int TextItem, int TextSize, int LeftOffset, int RightOffset, int y_pos, bool Alarm) { // Subrutina para escribir strings en pantalla
char AuxChar[13];                                                                 // Variable auxiliar para textos 12 caracteres mas blank
int  x_pos;                                                                       // Variable con posicion x calculada

  memset(AuxChar, 0, sizeof AuxChar);                                             // Inicializa string auxiliar
  memcpy_P (&AuxChar, TextItem, 12);                                              // Copia desde memoria FLASH
  switch (CenterType) {                                                           // Comprueba tipo de centrado texto
    case 0:                                                                       // Centrado a la izquierda
      x_pos = LeftOffset;
    break;
    
    case 1:                                                                       // Centrado en el medio
      x_pos = (((84 - RightOffset) - (strlen(AuxChar) * (6 * TextSize))) / 2) + LeftOffset;
    break;
    
    case 2:                                                                       // Centrado a la derecha
      x_pos = ((84 - RightOffset) - (strlen(AuxChar) * (6 * TextSize)));
    break;
  }
  if (Alarm) lcd.fillRect(x_pos - 1, y_pos - 1, (strlen(AuxChar) * (6 * TextSize)) + 1, (7 * TextSize) + 2, BLACK); // Pinta rectangulo dependiendo si hay alarma o no

  lcd.setTextSize(TextSize);                                                      // Ajusta tamaño fuente
  lcd.setTextColor(!Alarm);                                                       // Ajusta color fuente dependiendo si hay alarma o no
  lcd.setCursor(x_pos, y_pos);                                                    // Calcula cursor para texto
  lcd.print(AuxChar);                                                             // Escribe texto
}

void DrawInt (int CenterType, int Number, int Units, int TextSize, int x_pos, int y_pos, bool DecPoint, bool Alarm) { // Subrutina para escribir strings en pantalla
  String AuxString;                                                               // Variable auxiliar para valores strings
  char   AuxNumber[7];                                                            // Variable auxiliar para 4 bytes numeros + 2 bytes unidades + blank
  char   AuxUnits[3];                                                             // Variable auxiliar para 2 bytes unidades + blank
  int    x_pos_calc;                                                              // Variable con posicion x calculada
  byte   NumberPixels;                                                            // Variable longitud numero calculado en pixels
  byte   StringLenght;                                                            // Variable longitud String calculada
  byte   MaxPixels;                                                               // Variable numero de campos maximos
  bool   DisplayUnits = 0;                                                        // Variable para mostrar inidades
  float  AuxFloat;
    
  memset(AuxUnits, 0, sizeof AuxUnits);                                           // Inicializa string auxiliar
  memcpy_P (&AuxUnits, Units, sizeof AuxUnits);                                   // Copia desde memoria FLASH unidades
  
  AuxFloat = (float)Number;                                                       // Convierte numero en float para no mostrar decimales 
         
  if (DecPoint)  {
    AuxFloat /= 10.0;                                                             // Divide numero entre 10 para mostrar decimales
    if ((AuxFloat > 99.9) || (AuxFloat < -9.9)) {                                 // Si numero con decimales supera 4 campos
      DecPoint = 0;                                                               // Si me voy a salir fuera de los 4 campos disponibles, quito punto decimal
      if (EditingFlag) Units10 = 1;                                               // Solo si estoy en edicion activo variable suma 10 unidades
    }
  }
  dtostrf(AuxFloat, 1 + (Number >= 10) + (Number >= 100) + (Number >= 1000), DecPoint, AuxNumber); // Convierte numero a string en el formato adecuado    
  AuxString = AuxNumber;                                                          // Mueve AuxNumber a String auxiliar
  NumberPixels = AuxString.length() * (6 * TextSize);                             // Calcula longitud en pixeles del numero a mostrar
  StringLenght = NumberPixels;
  MaxPixels = (24 * TextSize);                                                    // Numero maximo de pixeles 4 campos * 6 pixels * Tamaño del texto
  
  if (AuxUnits != "") {                                                           // Si tengo que mostrar unidades
    DisplayUnits = 1;                                           
    AuxString = AuxUnits;                                                         // Mueve AuxNumber a String auxiliar
    StringLenght += AuxString.length() * 6;                                       // Calcula longitud en pixeles de las unidades a mostrar
    MaxPixels += AuxString.length() * 6;                                          // Numero maximo de pixeles MaxPixel + 6 pixel de unidades * caracteres unidades
  }
  
  switch (CenterType) {                                                           // Comprueba tipo centrado
    case 0:                                                                       // Posicion a la izquierda
      x_pos_calc = x_pos;                                                         // Calcula posicion x
    break;
    
    case 1:                                                                       // Posicion centrada
      x_pos_calc = x_pos +((MaxPixels - StringLenght) / 2);                       // Calcula posicion x
    break;

    case 2:                                                                       // Posicion a la derecha
      x_pos_calc = x_pos + MaxPixels - StringLenght;                              // Calcula posicion x
    break;
  }

  if (Alarm) lcd.fillRect(x_pos - 1, y_pos - 1, MaxPixels + 1, (7 * TextSize) + 2, BLACK); // Pinta rectangulo dependiendo si hay alarma o no
  
  lcd.setTextSize(TextSize);                                                      // Ajusta tamaño fuente
  lcd.setTextColor(!Alarm);                                                       // Ajusta color fuente dependiendo si hay alarma o no
  lcd.setCursor(x_pos_calc, y_pos);                                               // Calcula cursor para texto
  lcd.print(AuxNumber);                                                           // Escribe numeros
  if (DisplayUnits) {                                                             // Si tengo que mostrar unidades
    lcd.setTextSize(1);                                                           // Ajusta tamaño fuente
    lcd.setCursor((x_pos_calc - TextSize + 1) + NumberPixels, (y_pos - TextSize + 1) + (8 * (TextSize - 1)));// Calcula cursor para texto
    lcd.print(AuxUnits);                                                          // Escribe unidades
  }
}
    
void FactoryReset() {                                                             // Subrutina valores de fabrica
  int i;                                                                          // Variable contador generico

  if (digitalRead(EBTN) == LOW) {
    Mem.Data.Mode = 0;                                                            // Valor de fabrica modo de trabajo

    Mem.Data.TempSp[0] =   45;                                                    // Valor de fabrica consigna nevera
    Mem.Data.TempSp[1] = -120;                                                    // Valor de fabrica consigna congelador 
    Mem.Data.TempSp[2] =  380;                                                    // Valor de fabrica consigna ambiente 
    Mem.Data.TempSp[3] = 1020;                                                    // Valor de fabrica consigna hervidor
    Mem.Data.TempSp[4] =  850;                                                    // valor de fabrica consigna standby hervidor

    Mem.Data.RpmVentAmb = 100;                                                    // Valor de fabrica rpm ventilador ambiente   
    Mem.Data.RpmVentNev = 50;                                                     // Valor de fabrica rpm ventilador nevera
    Mem.Data.RpmVentNevOn = 1;                                                    // Valor de fabrica minutos On ventilador nevera  
    Mem.Data.RpmVentNevOff = 5;                                                   // Valor de fabrica minutos Off ventilador nevera  

    Mem.Data.EnableAlarms = 0;                                                    // Variable alarmas habilitadas/deshabilitadas
    Mem.Data.TimeDoorOpen = 30;                                                   // Variable alarma tiempo puerta abierta
    Mem.Data.VoltAlarmHighDC = 145;                                               // Variable alarma voltaje alto
    Mem.Data.VoltAlarmLowDC = 105;                                                // Variable alarma voltaje bajo
    Mem.Data.AmpsAlarmHighDC = 100;                                               // Variable alarma consumo alto DC
    Mem.Data.AmpsAlarmLowDC = 40;                                                 // Variable alarma consumo bajo DC
    Mem.Data.AmpsAlarmHighAC = 10;                                                // Variable alarma consumo alto AC
    Mem.Data.AmpsAlarmLowAC = 02;                                                 // Variable alarma consumo bajo AC
        
    Mem.Data.Brillo = 50;                                                         // Valor de fabrica brillo
    Mem.Data.Contrast = 55;                                                       // Valor de fabrica contraste
    Mem.Data.BackLightTime = 15;                                                  // Valor de fabrica retroiluminacion
    Mem.Data.BackMenuTime = 10;                                                   // Valor de fabrica retroceso pantalla inactiva
    
    PopUpType = 2;                                                                // Configura pantalla parametros guardados
    ScreenNum = 0;                                                                // Selecciona pantalla popup
    for (i = 0 ; i < sizeof(StructMem) ; i++) EEPROM.put(i, Mem.b[i]);            // Actualiza valores en EEPROM
  } else {
    for (i = 0; i < sizeof(StructMem); i++) Mem.b[i] = EEPROM.read(i);            // Lee EEPROM
  }
}

void BackLightMenuTimer() {                                                       // Subrutina control Backlight
  static unsigned long LastMillis;
  static unsigned long SetPointLights;
  static unsigned long SetPointMenu;

  if (!EditingFlag) {                                                             // Si no tengo edicion
    SetPointLights = unsigned(Mem.Data.BackLightTime) * 1000;                     // Calculo ms para que no afecte en caso de editar parametro
    SetPointMenu = unsigned(Mem.Data.BackMenuTime) * 1000;                        // Calculo ms para que no afecte en caso de editar parametro
  }

  if (BackLight) {                                                                // Si BackLight == 1
    analogWrite(LCD_LED, map(Mem.Data.Brillo, 0, 100, 255, 0));                   // Activa iluminacion LCD
    LastMillis = millis();                                                        // Guarda valor actual de millis()
    BackLight = 0;                                                                // Inicializa flag inicio timers
  }
  if (LastMillis) {
    if ((millis() - LastMillis) > SetPointLights) digitalWrite(LCD_LED, HIGH);    //Calcula tiempo restante para TimerLightOn == 0
    if (((millis() - LastMillis) > SetPointMenu) && (SetPointMenu > 0)){          //Calcula tiempo restante para TimerMenuOn == 0
      EncValue = &AuxMenu;                                                        // Carga en puntero EncValue la direccion de la variable AuxMenu
      AuxMenuIndex[0] = 0;                                                        // Variable AuxMenuIndex
      AuxMenuIndex[1] = 0;                                                        // Variable AuxMenuIndex
      OffsetMenuIndex[0] = 0;                                                     // Variable AuxMenuIndex
      OffsetMenuIndex[1] = 0;                                                     // Variable AuxMenuIndex
      LastScreenNum = 0;                                                          // Variable ultima pantalla
      EditingFlag = 0;                                                            // Indicador estado edicion
      ScreenNum = 1;                                                              // Variable pantalla a cargar
      MenuIndex = 0;                                                              // Variable MenuIndex
      AuxMenu = 0;                                                                // Variable AuxMenu
      OffsetMenu = 0;                                                             // Variable OffsetMenu
    }
    if (((millis() - LastMillis) > SetPointMenu) && ((millis() - LastMillis) > SetPointLights)) LastMillis = 0;// Inicializa temporizador
  }
}

void PopUpTimer(int Screen) {                                                     // Subrutina control retroceso menu
  static unsigned long LastMillis;                                                // Variable con ultimo valor de millis

  if (LastMillis == 0) LastMillis = millis();                                     // Salva valor actual de millis si tiempo de espera no ha empezado                                                           
  if (millis() - LastMillis >= 3000){                                             // Si han pasado x ms o pulsacion larga
    if (PopUpType == 1) TypePress = LONG;                                         // Pulsacion larga para retroceso menu
    ScreenNum = Screen;                                                           // Carga pantalla seleccionada
    EditingFlag = 0;                                                              // Reset Editing
    LastMillis = 0;                                                               // Reset LastMillis
    PopUpType = 0;                                                                // Reset tipo de PopUp
  }  
}
  
void Blink() {                                                                    // Subrutina pulsos de tiempo
  static unsigned long BlinkLastMillis01s;                                        // Variable con ultimo valor de millis()
  static unsigned long BlinkLastMillis05s;                                        // Variable con ultimo valor de millis()
  static unsigned long BlinkLastMillis1s;                                         // Variable con ultimo valor de millis()
  static unsigned long BlinkLastMillis1m;                                         // Variable con ultimo valor de millis()

  Puls_01s_FlUp = 0;                                                              // Flanco positivo pulso de 0,1s a 0
  if (millis() - BlinkLastMillis01s >= 100) {
    BlinkLastMillis01s = millis();
    Puls_01s = !Puls_01s;                                                         // Pulso de 0,1s on y 0,1s off
    Puls_01s_FlUp = 1;                                                            // Flanco positivo pulso de 0,1s a 1
  }

  Puls_05s_FlUp = 0;                                                              // Flanco positivo pulso de 0,5s a 0
  if (millis() - BlinkLastMillis05s >= 500) {
    BlinkLastMillis05s = millis();
    Puls_05s = !Puls_05s;                                                         // Pulso de 0,5s on y 0,5s off
    Puls_05s_FlUp = 1;                                                            // Flanco positivo pulso de 0,5s a 1
  }

  Puls_1s_FlUp = 0;                                                               // Flanco positivo pulso de 1s a 0
  if (millis() - BlinkLastMillis1s >= 1000) {
    BlinkLastMillis1s = millis();
    Puls_1s = !Puls_1s;                                                           // Pulso de 1s on y 1s off
    Puls_1s_FlUp = 1;                                                             // Flanco positivo pulso de 1s a 1
  }

  Puls_1m_FlUp = 0;                                                               // Flanco positivo pulso de 1m a 0
  if (millis() - BlinkLastMillis1m >= 60000) {
    BlinkLastMillis1m = millis();
    Puls_1m = !Puls_1m;                                                           // Pulso de 1m on y 1m off
    Puls_1m_FlUp = 1;                                                             // Flanco positivo pulso de 1m a 1
  }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Bloque subrutinas y funciones de programa genericas
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void SerialPlotter() {                                                            // Subrutina pinta grafica

  if (Puls_1m_FlUp) {
    Serial.print(dsTemp[0] / 10.0);                                               // Pinta temperatura nevera
    Serial.print("\t");                                                           // Pinta espacio
    #if CONGELADOR                                                                // Si tengo sensor congelador
      Serial.print(dsTemp[1] / 10.0);                                             // Pinta temperatura congelador
    #endif
    Serial.print("\t");                                                           // Pinta espacio
    Serial.print(dsTemp[2] / 10.0);                                               // Pinta temperatura ambiente
    Serial.print("\t");                                                           // Pinta espacio
    Serial.print(dsTemp[3] / 10.0);                                               // Pinta temperatura hervidor
    Serial.print("\t");                                                           // Pinta espacio
    Serial.print((digitalRead(RES_OUT) * 10.0) - 40.0);                           // Pinta salida resistencias
    Serial.print("\t");                                                           // Pinta espacio
    Serial.print((digitalRead(FAN_NEV_OUT) * 10.0) - 60.0);                       // Pinta salida ventilador nevera
    Serial.print("\t");                                                           // Pinta espacio
    Serial.println((digitalRead(FAN_AMB_OUT) * 10.0) - 80.0);                     // Pinta salida ventilador ambiente
  }  
}

void Alarmas() {                                                                  // Subrutina gestion alarmas
// Significado bits EnableAlarms:
// Bit 0 = Puerta abierta
// Bit 1 = Temperatura gas por debajo histeresis
// Bit 2 = VoltAlarmHighDC
// Bit 3 = VoltAlarmLowDC
// Bit 4 = AmpsAlarmHighDC
// Bit 5 = AmpsAlarmLowDC
// Bit 6 = AmpsAlarmHighAC
// Bit 7 = AmpsAlarmLowAC
//
  if (bitRead(Mem.Data.EnableAlarms, 0) && digitalRead(DOOR_SWITCH)){             // Si tengo interruptor puerta cerrada o no tengo alarma habilitada
    DoorOpen = 1;                                                                 // Variable puerta abierta
    if (DoorCounter < Mem.Data.TimeDoorOpen) {                                    // Si tiempo puerta abierta no transcurrido
      if (Puls_1s_FlUp) DoorCounter++;                                            // Cuenta segundos puerta abierta    
    } else {                                                                      // Si tiempo puerta abierta transcurrido
      DoorAlarmFlag = 1;                                                          // Activa alarma       
      BackLight = 1;                                                              // Activa backlight
    }
  } else {                                                                        // Si tengo interruptor puerta abierta
    DoorOpen = 0;                                                                 // Variable puerta cerrada
    DoorCounter = 0;                                                              // Reset contador segundos puerta abierta 
    DoorAlarmFlag = 0;                                                            // Reset flag alarma puerta
  }
  
  if (Mem.Data.Mode == 1) {                                                       // Si Modo GAS
    if (dsTemp[3] > Mem.Data.TempSp[4]) GasLowInitFlag = 1;                       // Si temperatura minima hervidor superada
    if (GasLowInitFlag) {                                                         // Si temperatura minima hervidor superada
      if (bitRead(Mem.Data.EnableAlarms, 1))  GasLowAlarmFlag = (dsTemp[3] < Mem.Data.TempSp[4]); // Alarma gas bajo por temperatura hervidor
    }
  } else {
    GasLowInitFlag = 0;                                                           // Reset flag temperatora minima hervidor superada  
  }
  if (VoltsBatInitFlag) {                                                         // Si lecturas voltage iniciadas Ok
    if (bitRead(Mem.Data.EnableAlarms, 2))  VoltsBatAlarmFlag = (VoltsBat > Mem.Data.VoltAlarmHighDC); // Alarma Batt+
    if (bitRead(Mem.Data.EnableAlarms, 3))  VoltsBatAlarmFlag = (VoltsBat < Mem.Data.VoltAlarmLowDC);  // Alarma Batt-
  }
 
  if (ResetAlarms){                                                               // Reset alarmas
    GasLowInitFlag = 0;                                                           // Reset flag 
    GasLowAlarmFlag = 0;                                                          // Reset flag   
    VoltsBatAlarmFlag = 0;                                                        // Reset flag  
    AmpsAlarmDCFlag = 0;                                                          // Reset flag 
    AmpsAlarmACFlag = 0;                                                          // Reset flag  
    ResetAlarms = 0;                                                              // Fin reset alarmas
  }
  if (GasLowAlarmFlag || VoltsBatAlarmFlag || AmpsAlarmDCFlag || AmpsAlarmACFlag || !dsConfig[0] || !dsConfig[1] || !dsConfig[2] || !dsConfig[3]) { // Si tengo alarmas 
    AlarmActiveFlag = 1;                                                          // Activa flag ade alarma activa
    Mem.Data.Mode = 0;                                                            // Si alarma activa, mode = 0   
    BackLight = 1;                                                                // Activa backlight
  } else {
    AlarmActiveFlag = 0;                                                          // Desactiva flag de alarma activa
  } 
}

void Salidas() {                                                                  // Subrutina control salidas
  static int  CouterMin;                                                          // Variable contador minutos
  int         i;                                                                  // Variable contador generico

  if (Mem.Data.Mode > 0) {                                                        // Si tengo ciclo activo
    if (int (dsTemp[0]) >= (Mem.Data.TempSp[0] + 1)) bitSet(OutputState, 0);      // Calcula valor activacion salida nevera
    if (int (dsTemp[0]) <= (Mem.Data.TempSp[0] - 1)) bitClear(OutputState, 0);    // Calcula valor desactivacion salida nevera
    #if CONGELADOR                                                                // Si tengo sensor de congelador
      if (int (dsTemp[1]) >= (Mem.Data.TempSp[1] + 1)) bitSet(OutputState, 1);    // Calcula valor activacion salida congelador
      if (int (dsTemp[1]) <= (Mem.Data.TempSp[1] - 1)) bitClear(OutputState, 1);  // Calcula valor desactivacion salida congelador
    #endif
    if (int (dsTemp[2]) >= (Mem.Data.TempSp[2] + 1)) bitSet(OutputState, 2);      // Calcula valor activacion salida ambiente
    if (int (dsTemp[2]) <= (Mem.Data.TempSp[2] - 1)) bitClear(OutputState, 2);    // Calcula valor desactivacion salida ambiente    
    if (int (dsTemp[3]) <= (Mem.Data.TempSp[3] - 1)) bitSet(OutputState, 3);      // Calcula valor activacion salida hervidor
    if (int (dsTemp[3]) >= (Mem.Data.TempSp[3] + 1)) bitClear(OutputState, 3);    // Calcula valor desactivacion salida hervidor
    if (int (dsTemp[3]) <= (Mem.Data.TempSp[4] - 1)) bitSet(OutputState, 4);      // Calcula valor desactivacion salida standby hervidor
    if (int (dsTemp[3]) >= (Mem.Data.TempSp[4] + 1)) bitClear(OutputState, 4);    // Calcula valor desactivacion salida standby hervidor                      
 
    if (bitRead(OutputState, 2) && dsConfig[2]){                                  // Si tengo activada salida ventilador ambiente y sensor ambiente configurado
      FanAmbAux = Mem.Data.RpmVentAmb;                                            // Velocidad programada ventilador ambiente
    } else {
      FanAmbAux = 0;                                                              // Velocidad 0 ventilador ambiente 
    }

    if (Puls_1m_FlUp) {                                                           // Si tengo pulso 1m
      CouterMin++;                                                                // Incrementa contador de minutos
      if (!bitRead(OutputState, 5)) {                                             // Si salida no esta activa
        if (CouterMin >= Mem.Data.RpmVentNevOff) {                                // Si tiempo off transcurrido                                     
          bitWrite(OutputState, 5, true);                                         // Activa salida
          CouterMin = 0;                                                          // Inicializa contador de minutos
          FanNevAux = Mem.Data.RpmVentNev;                                        // Velocidad max ventilador nevera    
        } 
      } else {
        if (CouterMin >= Mem.Data.RpmVentNevOn) {                                 // Si tiempo on transcurrido   
          bitWrite(OutputState, 5, false);                                        // Desactiva salida
          CouterMin = 0;                                                          // Inicializa contador de minutos
          FanNevAux = 0;                                                          // Velocidad 0 ventilador nevera    
        }
      }       
    }     
  
    if(Mem.Data.Mode == 1){                                                       // Modo GAS
      bitClear(OutputState, 0);                                                   // Desactiva salida nevera
      bitClear(OutputState, 1);                                                   // Desactiva salida congelador
      bitClear(OutputState, 3);                                                   // Desactiva salida hervidor
      bitClear(OutputState, 4);                                                   // Desactiva salida standby
    }
        
    if(Mem.Data.Mode == 2){                                                       // Modo ECO
      bitClear(OutputState, 4);                                                   // Desactiva salida standby
    }

  } else {                                                                        // Si tengo alarma
    OutputState = 0;                                                              // Desactivo flags salidas
    FanNevAux = 0;                                                                // Velocidad 0 ventilador nevera    
    FanAmbAux = 0;                                                                // Velocidad 0 ventilador ambiente     
  }
  digitalWrite(RES_OUT, (bitRead(OutputState, 0) || bitRead(OutputState, 1) || bitRead(OutputState, 4)) && bitRead(OutputState, 3)); // Salida resistencia
  analogWrite(FAN_AMB_OUT, map(FanAmbAux, 0, 100, 0, 255));                       // Configura velocidad ventilador ambiente
  if (!DoorOpen){                                                                 // Si tengo puerta cerrada
    analogWrite(FAN_NEV_OUT, map(FanNevAux, 0, 100, 0, 255));                     // Configura velocidad ventilador nevera
  } else {                                                                        // Si tengo interruptor puerta abierta
    analogWrite(FAN_NEV_OUT, 0);                                                  // Para ventilador nevera con puerta abierta 
  }
  digitalWrite(BUZZER, ((AlarmActiveFlag || DoorAlarmFlag) && Puls_05s));         // Si tengo alarmas, salida buzzer intermitente 
}

void GetCPUVolt() {                                                               // Subrutina lectura voltaje CPU

  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);                                                                       // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);                                                            // Convert
  while (bit_is_set(ADCSRA, ADSC));                                               // Whait for conversion complete
  CPUmVolt = ADCL;                                                                // Read 8 low bits
  CPUmVolt |= ADCH << 8;                                                          // Read 8 high bits
  CPUmVolt = 1126400L / CPUmVolt;                                                 // Voltage is returned in milli-V. So 5000 is 5v

}
    
void GetVoltBat() {                                                               // Subrutina lectura voltaje
  static long Result;                                                             // Variable resultado calculado acumulativo
  static byte i;                                                                  // Variable contador generico

  if (!VoltsBatAlarmFlag) {                                                       // Si no tengo alarma voltaje bateria
    Result += (((analogRead(VOLT_PIN) * CPUmVolt) / 1023) * (R1 + R2) / R2) / 100;// Ecuación  para obtener V, VIn, resolucion 1023
    i++;                                                                          // Incrementa contador
    if (i >= 25) {                                                                // Si he tomado 25 muestras
      VoltsBat = (int)(Result / 25);                                              // Hago promedio
      Result = 0;                                                                 // Acumulativo igual a 0
      i = 0;                                                                      // Inicializa contador
      VoltsBatInitFlag = 1;                                                       // Variable lectura voltaje bateria iniciada
    }
  } else {
    VoltsBatInitFlag = 0;                                                         // Reset lectura voltaje bateria iniciada      
  }
}

void GetAmps() {                                                                  // Subrutina lectura amperios
  long         AuxValue;                                                          // Variable auxiliar para valores 
  static long  result;                                                            // Variable resultado calculado acumulativo
  static long  Imax;                                                              // Variable para Imax
  static long  Imin;                                                              // Variable para Imin
  static int   NegCount;                                                          // Variable contador lecturas negativas
  static int   Readings;                                                          // Contador numero ciclos lecturas Ok
  static byte  i;                                                                 // Variable contador generico
  static bool  FirstAlarmFlag;                                                    // Variable para primera alarma detectada
  
  if ((digitalRead(RES_OUT)) && (i < 25)) {                                       // Si termostato ON o 25 muestras no alcanzadas
   AuxValue = ((((analogRead(AMPS_PIN) * CPUmVolt) / 1023) - (CPUmVolt / 2)) * 10) / ACDC_SENSOR; // Ecuación  para obtener A, mVIn, resolucion 1023, sensibilidad en V/A sensor 30A
    if (AuxValue < 0) NegCount++;                                                 // Cuenta las mediciones negativas        
    if(AuxValue>Imax) Imax=AuxValue;                                              // Compara para sacar Imax
    if(AuxValue<Imin) Imin=AuxValue;                                              // Compara para sacar Imin
    result += AuxValue;                                                           // Almacena y suma calculo acumulativo
    i++;                                                                          // Incrementa contador       
    if (i >= 25) {                                                                // Si he tomado 25 muestras
      if (NegCount > 5) {                                                         // Si he tenido mas de 5 mediciones negativas
        if (SelectedACDC) {                                                       // Si antes habia detectado DC
          SelectedACDC = 0;                                                       // AC detectada
          Readings = 0;                                                           // Inicializa contador lecturas Ok 
          goto Initialize;                                                        // Inicializa lecturas 
        }
        if (!AmpsAlarmACFlag) AmpsACDC[0] = (int)(((Imax-Imin)/2));               // Hago promedio y sumo offset
      } else {
        if (!SelectedACDC) {                                                       // Si antes habia detectado AC
          SelectedACDC = 1;                                                       // DC detectada
          Readings = 0;                                                           // Inicializa contador lecturas Ok 
          goto Initialize;                                                        // Inicializa lecturas 
        }
        if (!AmpsAlarmDCFlag) AmpsACDC[1] = (int)(result / i);                    // Hago promedio y sumo offset      
      } 
      if (Readings >= 3) {                                                        // Si ha hecho mas de 3 mediciones correctas
        if (SelectedACDC) {                                                       // DC detectada
          if ((bitRead(Mem.Data.EnableAlarms, 4) && (AmpsACDC[1] > Mem.Data.AmpsAlarmHighDC)) || (bitRead(Mem.Data.EnableAlarms, 5) && (AmpsACDC[1] < Mem.Data.AmpsAlarmLowDC))){  
            if (FirstAlarmFlag){
              AmpsAlarmDCFlag = 1;                                                // Activo flag de alarma AmpsDC
            } else {
              FirstAlarmFlag  = 1;                                                // Activo flag de primera alarma detectada
            }
          } else {
              FirstAlarmFlag  = 0;                                                // Reset flag de primera alarma detectada 
          }
        } else {                                                                  // AC detectada
          if ((bitRead(Mem.Data.EnableAlarms, 6) && (AmpsACDC[0] > Mem.Data.AmpsAlarmHighAC)) || (bitRead(Mem.Data.EnableAlarms, 7) && (AmpsACDC[0] < Mem.Data.AmpsAlarmLowAC))){
            if (FirstAlarmFlag){
              AmpsAlarmACFlag = 1;                                                // Activo flag de alarma AmpsAC 
            } else {
               FirstAlarmFlag  = 1;                                               // Activo flag de primera alarma detectada    
            }
          } else {
              FirstAlarmFlag  = 0;                                                // Reset flag de primera alarma detectada 
          }
        }  
     } else {
        Readings++;                                                               // Incrementa contador lecturas Ok 
      }
    }
  } else {                                                                        // Si termostato OFF          
    if ((!digitalRead(RES_OUT)) && (!AmpsAlarmDCFlag) && (!AmpsAlarmACFlag)) {    // Si no tengo alarmas de consumo
     SelectedACDC = 0;                                                            // Inicializa seleccion ACDC
     Readings = 0;                                                                // Inicializa contador lecturas Ok                 
     AmpsACDC[0] = 0;                                                             // Inicializa AmpsDC leidos
     AmpsACDC[1] = 0;                                                             // Inicializa AmpsAC leidos
    }
    Initialize:                                                                   // Marca de salto GOTO       
    NegCount = 0;                                                                 // Inicializa contador lecturas negativas
    result = 0;                                                                   // Acumulativo igual a 0
    Imax=0;                                                                       // Inicializa Imax
    Imin=0;                                                                       // Inicializa Imin
    i = 0;                                                                        // Inicializa contador    
  }  
}

void GetDS18b20() {                                                               // Subrutina inicializacion y lectura sensores DS18b20

  ds0.begin((int)&dsConfig[0], &dsTemp[0], &dsCont[0]);                           // Inicializacion y lectura sensor 0, nevera
  #if CONGELADOR                                                                  // Si tengo sensor congelador
    ds1.begin((int)&dsConfig[1], &dsTemp[1], &dsCont[1]);                         // Inicializacion y lectura sensor 1, congelador
  #else
    dsConfig[1] = 1;                                                              // Anula alarma sensor al marcrlo como configurado
  #endif
  ds2.begin((int)&dsConfig[2], &dsTemp[2], &dsCont[2]);                           // Inicializacion y lectura sensor 2, ambiente 
  ds3.begin((int)&dsConfig[3], &dsTemp[3], &dsCont[3]);                           // Inicializacion y lectura sensor 3, hervidor 
}

