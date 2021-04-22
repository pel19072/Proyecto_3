//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1

#define PUSHS PE_3
#define PUSHC PF_1
#define PUSHJ1 PE_5
#define PUSHC1 PA_6
#define PUSHJ2 PA_7
#define PUSHC2 PE_2

String text1 = "Bienvenido a";
String text2 = "Crazy Cars!";
String text3 = "Seleccione la";
String text3_1 = "Cantidad de";
String text3_2 = "Jugadores:";
String text3_3 = "1    2";
String text4 = "Comenzar!!";

int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};
uint8_t arrow_x = 50;
uint8_t arrow_y = 55;
uint8_t FLAGC = 0;
uint8_t FLAGC1 = 0;
uint8_t FLAGC2 = 0;
uint8_t FLAG = 0;
uint8_t FLAGJ1 = 0;
uint8_t FLAGJ2 = 0;
uint8_t arrow = 0;
uint8_t confirmation = 0;
uint8_t jump = 0;
uint8_t player = 0;
uint8_t choque = 0;
uint8_t xpos = 0;
uint32_t appear = 0;
uint8_t ypos1 = 0;
uint8_t ypos2 = 0;
uint8_t carriles[2][2] = {{0, 0}, {0, 0}};
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

void Limpieza_Unitaria(void);
void Seleccion_de_Jugadores(void);
void Pantalla_de_Inicio(void);
void seleccion_de_carro(void);
void Generar_Carretera(void);
void Generar_Color(uint8_t x);
void eleccion (uint8_t pl, uint8_t x, uint8_t y, int index, char flip, char offset);

extern uint8_t cover[];
extern uint8_t flecha[];
extern uint8_t player1L[];
extern uint8_t player2L[];
extern uint8_t gcar[];
extern uint8_t rcar[];
extern uint8_t bcar[];
extern uint8_t ycar[];
extern uint8_t street[];
//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  Serial.begin(9600);
  pinMode(PUSHS, INPUT_PULLUP);
  pinMode(PUSHC, INPUT_PULLUP);
  pinMode(PUSHJ1, INPUT_PULLUP);
  pinMode(PUSHC1, INPUT_PULLUP);
  pinMode(PUSHJ2, INPUT_PULLUP);
  pinMode(PUSHC2, INPUT_PULLUP);
  pinMode(PF_4,OUTPUT);
  pinMode(PF_2,OUTPUT);

  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  LCD_Clear(0x00);

  FillRect(0, 0, 319, 206, 0xdf5f);

  //LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
  LCD_Bitmap(0, 120, 320, 120, cover);
  LCD_Print(text1, 60, 0, 2, 0x018a, 0xdf5f);
  LCD_Print(text2, 70, 20, 2, 0x018a, 0xdf5f);
  LCD_Print(text4, 80, 100, 2, 0x018a, 0xdf5f);

}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  if (confirmation == 0) {
    Pantalla_de_Inicio();
  }
  else if (confirmation == 1) {
    Seleccion_de_Jugadores();
  }
  else if (confirmation == 2) {
    Limpieza_Unitaria();
    seleccion_de_carro();
  }
  else if (confirmation == 3) {
    Limpieza_Unitaria();
    LCD_Print("Escoja su vehiculo", 20, 20, 2, 0x018a, 0xdf5f);
    LCD_Print("Jugador 1 Jugador 2", 10, 60, 2, 0x018a, 0xdf5f);
  }
  else if (confirmation == 4) {
    FillRect(0, 0, 320, 240, 0xdf5f);
    LCD_Print("comienza el juego", 20, 120, 2, 0x018a, 0xdf5f);
    delay(1000);
    Generar_Carretera();
    eleccion (player, 15, 201, 0, 0, 0);
    while (choque == 0) {
      int obstacle = rand() % 7;

      if (appear % 50000 == 0) {
        switch (obstacle) {
          case 0:
            break;
          case 1:
            Generar_Color(15);
            break;
          case 2:
            Generar_Color(65);
            break;
          case 3:
            Generar_Color(115);
            break;
          case 4:
            Generar_Color(165);
            break;
          case 5:
            Generar_Color(215);
            break;
          case 6:
            Generar_Color(265);
            break;
        }

        switch (carriles[1][0]) {
          case 0:
            LCD_Bitmap(carriles[0][0], ypos1, 40, 40, ycar);
            break;
          case 1:
            LCD_Bitmap(carriles[0][0], ypos1, 40, 40, gcar);
            break;
          case 2:
            LCD_Bitmap(carriles[0][0], ypos1, 40, 40, bcar);
            break;
          case 3:
            LCD_Bitmap(carriles[0][0], ypos1, 40, 40, rcar);
            break;
        }
        FillRect(carriles[0][0], ypos1 - 10, 40, 10, 0x9492);
        if (ypos1 == 240) {
          carriles[0][0] = 0;
          ypos1 = 0;
        }
        switch (carriles[1][1]) {
          case 0:
            LCD_Bitmap(carriles[0][1], ypos2, 40, 40, ycar);
            break;
          case 1:
            LCD_Bitmap(carriles[0][1], ypos2, 40, 40, gcar);
            break;
          case 2:
            LCD_Bitmap(carriles[0][1], ypos2, 40, 40, bcar);
            break;
          case 3:
            LCD_Bitmap(carriles[0][1], ypos2, 40, 40, rcar);
            break;
        }
        FillRect(carriles[0][1], ypos2 - 10, 40, 10, 0x9492);
        if (ypos2 == 240) {
          carriles[0][1] = 0;
          ypos2 = 0;
        }
      }
      if (appear % 5000 == 0) {
        ypos1++;
        ypos2++;
      }
      appear++;
      if (digitalRead(PUSHC1) == 0) {
        FLAGC1 = 1;
      } else {
        if (FLAGC1 == 1) {
          //choque = 1;
          digitalWrite(PF_4, LOW);
          digitalWrite(PF_2, HIGH);
          FLAGC1 = 0;
          if (xpos <= 200) {
            for (int x = 0; x < 5; x++) {
              eleccion (player, 15 + xpos, 201, x, 0, 0);
            }
            FillRect(15 + xpos, 201, 41, 39, 0x9492);
            xpos = xpos + 50;
            eleccion (player, 15 + xpos, 201, 0, 0, 0);
          } else {
            FillRect(15 + xpos, 201, 41, 39,    0x9492);
            xpos = 0;
            eleccion (player, 15, 201, 0, 0, 0);
          }
        }
      }
      if (digitalRead(PUSHJ1) == 0) {
        FLAGJ1 = 1;
      } else {
        if (FLAGJ1 == 1) {
          digitalWrite(PF_2, LOW);
          digitalWrite(PF_4, HIGH);
          FLAGJ1 = 0;
          if (xpos > 0) {
            for (int x = 0; x < 5; x++) {
              eleccion (player, 15 + xpos, 201, x, 1, 0);
            }
            FillRect(15 + xpos, 201, 41, 39,    0x9492);
            xpos = xpos - 50;
            eleccion (player, 15 + xpos, 201, 0, 0, 0);
          } else {
            FillRect(15, 201, 41, 39,    0x9492);
            xpos = 250;
            eleccion (player, 265, 201, 0, 0, 0);
          }
        }
      }
    }
  }
}
//***************************************************************************************************************************************
// Funciones de Juego
//***************************************************************************************************************************************
void Limpieza_Unitaria(void) {
  switch (jump) {
    case 0:
      FillRect(0, 0, 320, 240, 0xdf5f);
      jump++;
      break;
    case 1:
      break;
  }
}

void Pantalla_de_Inicio(void) {
  if (digitalRead(PUSHC) == 1) {
    FLAGC = 1;
    delay(50);
  }
  else {
    if (FLAGC == 1) {
      FLAGC = 0;
      confirmation = 1;
    }
  }
  LCD_Bitmap(40, 90, 40, 30, flecha);
  delay(150);
  FillRect(40, 90, 40, 30, 0xdf5f);
  delay(150);
}

void Seleccion_de_Jugadores(void) {
  switch (jump) {
    case 0:
      FillRect(0, 0, 320, 120, 0xdf5f);
      LCD_Print(text3, 50, 0, 2, 0x018a, 0xdf5f);
      LCD_Print(text3_1, 60, 20, 2, 0x018a, 0xdf5f);
      LCD_Print(text3_2, 70, 40, 2, 0x018a, 0xdf5f);
      LCD_Print(text3_3, 90, 60, 2, 0x018a, 0xdf5f);
      jump++;
      break;
    case 1:
      break;
  }
  //ANTIREBOTE DEL BOTON DE INICIO
  if (digitalRead(PUSHS) == 0) {
    FLAG = 1;
    delay(50);
  }
  else {
    if (FLAG == 1) {
      FLAG = 0;
      switch (arrow) {
        case 0:
          arrow_x = 130;
          arrow_y = 55;
          arrow++;
          break;
        case 1:
          arrow_x = 50;
          arrow_y = 55;
          arrow = 0;
          break;
      }
    }
  }
  LCD_Bitmap(arrow_x, arrow_y, 40, 30, flecha);
  delay(150);
  FillRect(arrow_x, arrow_y, 40, 30, 0xdf5f);
  delay(150);

  if (digitalRead(PUSHC) == 1) {
    FLAGC = 1;
    delay(50);
  }
  else {
    if (FLAGC == 1) {
      FLAGC = 0;
      jump = 0;
      if (arrow_x == 50) {
        confirmation = 2;
      }
      else {
        confirmation = 3;
      }
    }
  }
}
void seleccion_de_carro(void) {
  LCD_Print("Escoja su vehiculo", 20, 20, 2, 0x018a, 0xdf5f);
  if (player == 0) {
    LCD_Sprite(110, 110, 41, 39, player1L, 5, 0, 0, 0);
    if (digitalRead(PUSHJ1) == 0) {
      FLAGJ1 = 1;
    } else {
      if (FLAGJ1 == 1) {
        FLAGJ1 = 0;
        player = 1;
      }
    }
    if (digitalRead(PUSHC1) == 0) {
      FLAGC1 = 1;
    } else {
      if (FLAGC1 == 1) {
        FLAGC1 = 0;
        jump = 0;
        player = 2;
        confirmation = 4;
      }
    }
  } else if (player == 1) {
    LCD_Sprite(110, 110, 41, 39, player2L, 5, 0, 0, 0);
    if (digitalRead(PUSHJ1) == 0) {
      FLAGJ1 = 1;
    } else {
      if (FLAGJ1 == 1) {
        FLAGJ1 = 0;
        player = 0;
      }
    }
    if (digitalRead(PUSHC1) == 0) {
      FLAGC1 = 1;
    } else {
      if (FLAGC1 == 1) {
        FLAGC1 = 0;
        jump = 0;
        player = 3;
        confirmation = 4;
      }
    }
  }

}
void Generar_Carretera(void) {
  for (int x = 0; x < 240 / 10; x++) {
    LCD_Bitmap(0, x * 10, 10, 10, grama);
    LCD_Bitmap(310, x * 10, 10, 10, grama);
  }
  for (int y = 0; y < 240 / 10; y++) {
    LCD_Bitmap(10, y * 20, 100, 20, street);
    LCD_Bitmap(110, y * 20, 100, 20, street);
    LCD_Bitmap(210, y * 20, 100, 20, street);
  }
}

void Generar_Color(uint8_t x) {
  int color_obs = rand() % 3;
  for (int i = 0; i < 2; i++)
  {
    if (carriles[0][i] == 0) {
      carriles[0][i] = x;
      switch (color_obs) {
        case 0:
          carriles[1][i] = 0;
          break;
        case 1:
          carriles[1][i] = 1;
          break;
        case 2:
          carriles[1][i] = 2;
          break;
        case 3:
          carriles[1][i] = 3;
          break;
      }
      break;
    }
  }
}
void eleccion (uint8_t pl, int x2, int y2, int index1, char flip1, char offset1) {
  if (pl == 2) {
    LCD_Sprite(x2, y2, 41, 39, player1L, 5, index1, flip1, offset1);
  } else if (pl == 3) {
    LCD_Sprite(x2, y2, 41, 39, player2L, 5, index1, flip1, offset1);
  }
}
//****************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
  }
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);

      //LCD_DATA(bitmap[k]);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}
