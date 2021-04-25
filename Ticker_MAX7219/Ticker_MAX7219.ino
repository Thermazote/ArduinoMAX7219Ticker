#include <SPI.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

/*------------- Настройки -------------*/
//Пины блютуз модуля
#define BT_RX 3
#define BT_TX 2

//Пин CS матрицы
#define MATRIX_CS 10

//Количество матриц по горизонтали
#define NUM_HORIZONTAL_MATRIX 4

//Количество матриц по вертикали
#define NUM_VERTICAL_MATRIX 1

//Признаки начала и конца передаваемой последовательности
#define START_CHAR 2  //непечатаемый символ ASCII: STX (02) "start of text"
#define END_CHAR 3    //непечатаемый символ ASCII: ETX (03) "end of text"

/*------------- Переменные -------------*/
//Создаём экземпляр класса матрицы
Max72xxPanel matrix = Max72xxPanel(MATRIX_CS, NUM_VERTICAL_MATRIX, NUM_HORIZONTAL_MATRIX);

//Создаём экземпляр класса последовательного порта
SoftwareSerial btSerial(BT_TX, BT_RX);

//Последовательность байт принятых по блютуз
unsigned char btByte;           //Контрольный байт
unsigned char btActivateMatrix; //Матрица: откл.(0), вкл.(1) 
unsigned char btMode;           //Режим работы: рисование(0), текст(1) 
unsigned char btStatic;         //Статичность: откл.(0) вкл.(1)
unsigned char btBrightness;     //Яркость матрицы {0 - 15}
unsigned char btSpeed;          //Скорость прокрутки {0 -15}
unsigned char btPixels[8][4];   //Массив состояний пикселей матрицы
String btString = "";           //Строка с текстом

/*------------- Отладка -------------*/
//Время ожидания прихода в буфер полного пакета (в милисекундах)
#define WAITING_PACKAGE_TIMEOUT 900

//Перенаправлять полученные по блютуз данные в порт
#define REDIRECT_SERIAL_MONITOR 0

//После каждого приёма данных выводить сводку в порт
#define VAR_STATUS 1

/*-------------------------- Сетап --------------------------*/
void setup()
{
  //Открываем последовательный блютуз порт 
  btSerial.begin(9600);

  //Слив буфера
  while(btSerial.available()) 
    btSerial.read();

  //Если включен вывод в порт
  if (REDIRECT_SERIAL_MONITOR || VAR_STATUS)
  {
    //Открываем последовательный порт
    Serial.begin(9600);
  }
  
  //Задаём яркость матрицы {0 - 15}
  matrix.setIntensity(0);

  //Очищаем всю матрицу
  matrix.fillScreen(LOW);
  matrix.write();

  //Задаём систему координат матрицы {1 - 4}
  matrix.setRotation(1);
}

/*-------------------------- Основной цикл --------------------------*/
void loop()
{
  //Проверяем буфер на наличие новых данных
  if (btSerial.available() > 0)
  {
    //Ожидание прихода в буфер полного пакета
    delay(WAITING_PACKAGE_TIMEOUT);
    
    //Если включено перенаправление в порт
    if (REDIRECT_SERIAL_MONITOR)
    {
      //Пишем в порт байт из буфера и выходим
      Serial.println(btSerial.read());
      delay(10);
      return 0; 
    }

    //Кладём первый байт буфера в контрольный байт
    btByte = btSerial.read();
    
    //Проверка что поймали именно управляющую последовательность
    if (btByte == START_CHAR)
    {
      //Считываем байты настроек
      btActivateMatrix = btSerial.read();
      btMode = btSerial.read();       
      btStatic = btSerial.read();     
      btBrightness = btSerial.read();
      btSpeed = btSerial.read();

      /*В зависимости от режима работы: 
       * либо заполняем массив состояний пикселей,
       * либо формируем строку текста.
      */
      if (btMode == 0)
      {
        /*Всего 8 строк по 32 пикселя,
         *тогда сохраним одну строку как 4 char переменных. 
         *Каждый бит внутри char'а - состояние пикселя.
        */
        for (int row = 0; row < 8; row++)
          for (int disp = 0; disp < 4; disp++)
            btPixels[row][disp] = btSerial.read();

        //Последний байт в буфере кладём в контрольный байт
        btByte = btSerial.read();
      }
      else if (btMode == 1)
      {
        //Посимвольный парсинг строки
        btString = "";
        btByte = btSerial.read();
        while(btByte != END_CHAR)
        {
          btString += btByte;
          btByte = btSerial.read();
        }
      }

      //Если включен вывод переменных в порт
      if (VAR_STATUS)
      {
        //Выводим основные переменные в порт
        //Контрольный байт
        Serial.print("Control byte: ");
        if (btByte == END_CHAR)
          Serial.println("CORRECT");
        else
          Serial.println("INCORRECT");
  
        //Состояние матрицы
        Serial.print("Matrix: ");
        if (btActivateMatrix == 0)
          Serial.println("OFF");
        else if (btActivateMatrix == 1)
          Serial.println("ON");
        else
          Serial.println("UNDEFINED");
  
        //Режим работы
        Serial.print("Mode: ");
        if (btMode == 0)
          Serial.println("DRAWING");
        else if (btMode == 1)
          Serial.println("TEXT");
        else 
          Serial.println("UNDEFINED");
  
        //Статичность
        Serial.print("Static: ");
        if (btStatic == 0)
          Serial.println("OFF");
        else if (btStatic == 1)
          Serial.println("ON");
        else 
          Serial.println("UNDEFINED");
  
        //Яркость
        Serial.print("Brightness: ");
        Serial.println(btBrightness, DEC);
  
        //Скорость
        Serial.print("Speed: ");
        Serial.println(btSpeed, DEC);

        //Массив или строка в зависимости от режима работы
        if (btMode == 0)
        {
          //Массив состояний пикселей матрицы
          Serial.println("Pixels: ");
          Serial.print(" ");
          for (int i = 0; i < 4; i++)
            for (int j = 0; j < 8; j++)
            {
              Serial.print(j);
              Serial.print(" ");
            }
          Serial.println();
          for (int row = 0; row < 8; row++)
          {
            Serial.print(row);
            for (int disp = 0; disp < 4; disp++)
            {
              for (int col = 0; col < 8; col++)
              {
                if ((btPixels[row][disp] & (1 << (7 - col))))
                  Serial.print("* ");
                else if (!(btPixels[row][disp] & (1 << (7 - col))))
                  Serial.print("  ");
                else
                  Serial.print("# "); 
              }
            }
            Serial.println();
          }
        }
        else if (btMode == 1)
        {
          //Строка текста
          Serial.print("String: ");
          Serial.println(btString); 
        }
      }
    }
    else
    {
      //Слив буфера 
      while(btSerial.available()) 
        btSerial.read();
    }
  }

  //Обновляем состояние матрицы
}
