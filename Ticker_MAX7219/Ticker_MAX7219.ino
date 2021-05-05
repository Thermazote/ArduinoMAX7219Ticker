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
unsigned char btSpeed;          //Скорость прокрутки {0 - 15}
unsigned char btPixels[32];     //Массив состояний пикселей матрицы
String btString = "";           //Строка с текстом

//Изменились данные для вывода
bool displayChanged = false;     

//Шрифт для вывода символов
const PROGMEM unsigned char font[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, //   //32
  {0x00, 0x00, 0x6f, 0x00, 0x00}, // !
  {0x00, 0x07, 0x00, 0x07, 0x00}, // "
  {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
  {0x46, 0x49, 0x7f, 0x49, 0x31}, // $
  {0x23, 0x13, 0x08, 0x64, 0x62}, // %
  {0x36, 0x49, 0x56, 0x20, 0x50}, // &
  {0x00, 0x00, 0x07, 0x00, 0x00}, // '
  {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
  {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
  {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
  {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
  {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
  {0x08, 0x08, 0x08, 0x08, 0x08}, // -
  {0x00, 0x60, 0x60, 0x00, 0x00}, // .
  {0x20, 0x10, 0x08, 0x04, 0x02}, // /
  {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
  {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
  {0x00, 0x36, 0x36, 0x00, 0x00}, // :
  {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
  {0x08, 0x14, 0x22, 0x41, 0x00}, // <
  {0x14, 0x14, 0x14, 0x14, 0x14}, // =
  {0x00, 0x41, 0x22, 0x14, 0x08}, // >
  {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
  {0x3e, 0x41, 0x49, 0x55, 0x5e}, // @
  {0x7e, 0x09, 0x09, 0x09, 0x7e}, // A
  {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7f, 0x41, 0x41, 0x41, 0x3e}, // D
  {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
  {0x3e, 0x41, 0x49, 0x49, 0x3a}, // G
  {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
  {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
  {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
  {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
  {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
  {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
  {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
  {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
  {0x0f, 0x30, 0x40, 0x30, 0x0f}, // V
  {0x3f, 0x40, 0x30, 0x40, 0x3f}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
  {0x00, 0x7f, 0x41, 0x41, 0x00}, // [
  {0xff, 0xff, 0xff, 0xff, 0xff}, // Сущность в виде гномика (в ASCII тут ничего нет, но без него не заработает)
  {0x02, 0x04, 0x08, 0x10, 0x20}, // \
  {0x00, 0x41, 0x41, 0x7f, 0x00}, // ]
  {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
  {0x40, 0x40, 0x40, 0x40, 0x40}, // _
  {0x00, 0x00, 0x03, 0x04, 0x00}, // `
  {0x20, 0x54, 0x54, 0x54, 0x78}, // a
  {0x7f, 0x48, 0x44, 0x44, 0x38}, // b
  {0x38, 0x44, 0x44, 0x44, 0x20}, // c
  {0x38, 0x44, 0x44, 0x48, 0x7f}, // d
  {0x38, 0x54, 0x54, 0x54, 0x18}, // e
  {0x08, 0x7e, 0x09, 0x01, 0x02}, // f
  {0x0c, 0x52, 0x52, 0x52, 0x3e}, // g
  {0x7f, 0x08, 0x04, 0x04, 0x78}, // h
  {0x00, 0x44, 0x7d, 0x40, 0x00}, // i
  {0x20, 0x40, 0x44, 0x3d, 0x00}, // j
  {0x00, 0x7f, 0x10, 0x28, 0x44}, // k
  {0x00, 0x41, 0x7f, 0x40, 0x00}, // l
  {0x7c, 0x04, 0x18, 0x04, 0x78}, // m
  {0x7c, 0x08, 0x04, 0x04, 0x78}, // n
  {0x38, 0x44, 0x44, 0x44, 0x38}, // o
  {0x7c, 0x14, 0x14, 0x14, 0x08}, // p
  {0x08, 0x14, 0x14, 0x18, 0x7c}, // q
  {0x7c, 0x08, 0x04, 0x04, 0x08}, // r
  {0x48, 0x54, 0x54, 0x54, 0x20}, // s
  {0x04, 0x3f, 0x44, 0x40, 0x20}, // t
  {0x3c, 0x40, 0x40, 0x20, 0x7c}, // u
  {0x1c, 0x20, 0x40, 0x20, 0x1c}, // v
  {0x3c, 0x40, 0x30, 0x40, 0x3c}, // w
  {0x44, 0x28, 0x10, 0x28, 0x44}, // x
  {0x0c, 0x50, 0x50, 0x50, 0x3c}, // y
  {0x44, 0x64, 0x54, 0x4c, 0x44}, // z
  {0x00, 0x08, 0x36, 0x41, 0x41}, // {
  {0x00, 0x00, 0x7f, 0x00, 0x00}, // |
  {0x41, 0x41, 0x36, 0x08, 0x00}, // }
  {0x04, 0x02, 0x04, 0x08, 0x04}, // ~  //126 
  {0x7E, 0x11, 0x11, 0x11, 0x7E}, // А  //192
  {0x7F, 0x49, 0x49, 0x49, 0x33}, // Б
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // В
  {0x7F, 0x01, 0x01, 0x01, 0x03}, // Г
  {0xE0, 0x51, 0x4F, 0x41, 0xFF}, // Д
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // Е
  {0x77, 0x08, 0x7F, 0x08, 0x77}, // Ж
  {0x41, 0x49, 0x49, 0x49, 0x36}, // З
  {0x7F, 0x10, 0x08, 0x04, 0x7F}, // И
  {0x7C, 0x21, 0x12, 0x09, 0x7C}, // Й
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // К
  {0x20, 0x41, 0x3F, 0x01, 0x7F}, // Л
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // М
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // Н
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // О
  {0x7F, 0x01, 0x01, 0x01, 0x7F}, // П
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // Р
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // С
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // Т
  {0x47, 0x28, 0x10, 0x08, 0x07}, // У
  {0x1C, 0x22, 0x7F, 0x22, 0x1C}, // Ф
  {0x63, 0x14, 0x08, 0x14, 0x63}, // Х
  {0x7F, 0x40, 0x40, 0x40, 0xFF}, // Ц
  {0x07, 0x08, 0x08, 0x08, 0x7F}, // Ч
  {0x7F, 0x40, 0x7F, 0x40, 0x7F}, // Ш
  {0x7F, 0x40, 0x7F, 0x40, 0xFF}, // Щ
  {0x01, 0x7F, 0x48, 0x48, 0x30}, // Ъ
  {0x7F, 0x48, 0x30, 0x00, 0x7F}, // Ы
  {0x00, 0x7F, 0x48, 0x48, 0x30}, // Ь
  {0x22, 0x41, 0x49, 0x49, 0x3E}, // Э
  {0x7F, 0x08, 0x3E, 0x41, 0x3E}, // Ю
  {0x46, 0x29, 0x19, 0x09, 0x7F}, // Я                                   
  {0x20, 0x54, 0x54, 0x54, 0x78}, // а
  {0x3C, 0x4A, 0x4A, 0x49, 0x31}, // б
  {0x7C, 0x54, 0x54, 0x28, 0x00}, // в
  {0x7C, 0x04, 0x04, 0x0C, 0x00}, // г
  {0xE0, 0x54, 0x4C, 0x44, 0xFC}, // д
  {0x38, 0x54, 0x54, 0x54, 0x18}, // е
  {0x6C, 0x10, 0x7C, 0x10, 0x6C}, // ж
  {0x44, 0x54, 0x54, 0x28, 0x00}, // з
  {0x7C, 0x20, 0x10, 0x08, 0x7C}, // и
  {0x78, 0x42, 0x24, 0x12, 0x78}, // й
  {0x7C, 0x10, 0x28, 0x44, 0x00}, // к
  {0x20, 0x44, 0x3C, 0x04, 0x7C}, // л
  {0x7C, 0x08, 0x10, 0x08, 0x7C}, // м
  {0x7C, 0x10, 0x10, 0x10, 0x7C}, // н
  {0x38, 0x44, 0x44, 0x44, 0x38}, // о
  {0x7C, 0x04, 0x04, 0x04, 0x7C}, // п
  {0x7C, 0x14, 0x14, 0x14, 0x08}, // р
  {0x38, 0x44, 0x44, 0x44, 0x00}, // с
  {0x04, 0x04, 0x7C, 0x04, 0x04}, // т
  {0x0C, 0x50, 0x50, 0x50, 0x3C}, // у
  {0x30, 0x48, 0xFE, 0x48, 0x30}, // ф
  {0x44, 0x28, 0x10, 0x28, 0x44}, // х
  {0x7C, 0x40, 0x40, 0x7C, 0xC0}, // ц
  {0x0C, 0x10, 0x10, 0x10, 0x7C}, // ч
  {0x7C, 0x40, 0x7C, 0x40, 0x7C}, // ш
  {0x7C, 0x40, 0x7C, 0x40, 0xFC}, // щ
  {0x04, 0x7C, 0x50, 0x50, 0x20}, // ъ
  {0x7C, 0x50, 0x50, 0x20, 0x7C}, // ы
  {0x7C, 0x50, 0x50, 0x20, 0x00}, // ь
  {0x28, 0x44, 0x54, 0x54, 0x38}, // э
  {0x7C, 0x10, 0x38, 0x44, 0x38}, // ю
  {0x08, 0x54, 0x34, 0x14, 0x7C}, // я  //255
  {0xff, 0x81, 0x81, 0x81, 0xff}  //непечатаемые символы 
};                                     

/*------------- Отладка -------------*/
//Время ожидания прихода в буфер полного пакета (в милисекундах)
#define WAITING_PACKAGE_TIMEOUT 50

//Перенаправлять полученные по блютуз данные в порт
#define REDIRECT_SERIAL_MONITOR 0

//После каждого приёма данных выводить сводку в порт
#define VAR_STATUS 0

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
    //Если включено перенаправление в порт
    if (REDIRECT_SERIAL_MONITOR)
    {
      //Пишем в порт байт из буфера и выходим
      Serial.println(btSerial.read());
      return 0; 
    }

    //Кладём первый байт буфера в контрольный байт
    btByte = btSerial.read();
    
    //Проверка что поймали именно управляющую последовательность
    if (btByte == START_CHAR)
    {
      //Ожидание прихода в буфер полного пакета
      delay(WAITING_PACKAGE_TIMEOUT);
    
      //Считываем байты настроек
      btActivateMatrix = btSerial.read();
      btMode = btSerial.read();       
      btStatic = btSerial.read();     
      btBrightness = btSerial.read();
      btSpeed = btSerial.read();

      //Проверяем содержит ли пакет изменения по строке или массиву состояний
      if (btSerial.available() == 1)
      {
        //Контрольный байт выставляем на закрытие пакета
        btByte = btSerial.read();
      }

      //Если пакет все еще открыт
      if (btByte == START_CHAR)
      {
        //Выставляем флаг чтобы выполнить переотрисовку на матрице
        displayChanged = true;
      
        /* В зависимости от режима работы: 
         * либо заполняем массив состояний пикселей,
         * либо формируем строку текста.
        */
        if (btMode == 0)
        {
          /* Всего 32 столбца по 8 пикселей,
           * тогда сохраним один столбик как в char переменную (снизу вверх). 
           * Каждый бит внутри char'а - состояние пикселя.
           * Вся матрица будет представлена массивом из 32 значений char (0 - 255)
          */
          for (int col = 0; col < 32; col++)
            btPixels[col] = btSerial.read();
  
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
            btString += (char)btByte;
            btByte = btSerial.read();
          }
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
            for (int col = 0; col < 32; col++)
            {
              if ((btPixels[col] & (1 << (row))))
                Serial.print("* ");
              else
                Serial.print("  ");
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
      //Слив буфера от пришедшего мусора 
      while(btSerial.available()) 
        btSerial.read();
    }
  }
  
  
  /* Обновляем состояние матрицы по одному из 4 алгоритмов:
   * 1. Бегущее изображение
   * 2. Статичное изображение
   * 3. Бегущая строка
   * 4. Сатичная строка
  */
  if (btMode == 0)
  {
    if (btStatic == 0)
    {
      //Бегущее изображение
      
    }
    else if (btStatic == 1 && displayChanged)
    {
      //Статичное изображение
  
      //Отрисовываем иозображение
      for (int col = 0; col < 32; col++)
      {
        for (int row = 0; row < 8; row++)
        {
          if (btPixels[col] & (1 << row))
            matrix.drawPixel(col, row, HIGH); 
          else
            matrix.drawPixel(col, row, LOW); 
        }
      }

      //Выводим всё на матрицу
      matrix.write();

      //Выставляем флаг что переотрисовка выполнена
      displayChanged = false;
    }
  }
  else if (btMode == 1)
  {
    if (btStatic == 0)
    {
      //Бегущая строка
      
    }
    else if (btStatic == 1 && displayChanged)
    {
      //Статичная строка
      char symbNumber = 0;//Номер текущего символа в строке
      char col = 0;       //Колонка матрицы
      char fontIndex;     //Индекс символа из массива шрифта
      char factStart = 0; //Фактическое начало символа
      char factEnd = 4;   //Фактический конец символа
      
      //Гасим все пиксели матрицы
      matrix.fillScreen(LOW);

      //Пока не дошли до правого края матрицы
      while (col < 32)
      {
        //Если символы в строке не кончились отрисовываем, иначе прекращаем
        if (symbNumber < btString.length())
        {
          //Находим номер отрисовываемого символа в массиве
          if (btString[symbNumber] > 31 && btString[symbNumber] < 256)
            fontIndex = btString[symbNumber] - 32;
          //else
            //...

          //Если символ это не пробел отрисовываем, иначе делаем обычный офсет
          if (fontIndex != 0)
          {
            /* Некоторые символы по типу "!.,|" имеют пустые боковые столбцы.
             * Чтобы при использовании таких символов строка текста казалась более плотной,
             * будем делать проверку крайних (боковых) столбоцов как целого байта на равенство нулю.
             * Если они равны нулю то удалим из печати этот столбец (просто пропускаем итерацию печати столбца).
            */
            //Сначала найдём фактическое начало символа
            for (int i = 0; i < 5; i++)
            {
              if (pgm_read_byte(&font[fontIndex][i]) == 0)
                factStart++;
              else
                break;
            }
            
            //Затем найдём фактический конец символа
            for (int i = 4; i >= 0; i--)
            {
              if (pgm_read_byte(&font[fontIndex][i]) == 0)
                factEnd--;
              else
                break;
            }

            //После потенциальной обрезки уже отрисовываем символ
            for (int symbCol = factStart; symbCol <= factEnd ; symbCol++)
            {     
              //Если следующий столбец символа не за границей матрицы, то отрисовываем, иначе прекращаем
              if ((col + symbCol - factStart) < 32)
              {
                //Отрисовываем стоблик симовла
                for (int symbBit = 0; symbBit < 8; symbBit++)
                {
                  if (pgm_read_byte(&font[fontIndex][symbCol]) & (1 << symbBit))
                    matrix.drawPixel(col + symbCol - factStart, symbBit, HIGH);
                }
              }
              else 
                break;
            }
    
            /*Высчитываем офсет для следующего символа 
             * col (текущая позиция на матрице)
             * + 6 (+5 для символа по умолчанию + 1 для пустого столбика между символами)
             * - factStart (отнимаем количество удалёных пустых столбцов с начала символа)
             * 5 - factEnd (отнимаем количество удалёных пустых столбцов с конца символа)
            */
            col = col + 6 - factStart - (4 - factEnd);

            //Сбрасываем счётчик стёртых столбцов для следующего символа
            factStart = 0;
            factEnd = 4;

            //Переходим к следующему символу в строке
            symbNumber++;
          }
          else
          {
            //Офсет для следующего символа (+4 пробел)
            col+=3;

            //Переходим к следующему символу в строке
            symbNumber++;
          }
        }
        else
          break;
      }

      //Выводим всё на матрицу
      matrix.write();

      //Выставляем флаг что переотрисовка выполнена
      displayChanged = false;
    }
  }
}
