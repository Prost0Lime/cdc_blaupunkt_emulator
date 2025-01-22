// Прототипы функций
void sendResponse(uint16_t data);
void initialization(uint8_t incoming, uint8_t ninthBit);

void setup() {
  Serial.begin(9600);
  Serial1.begin(4800);
  UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 9 бит данных
  UCSR1B |= (1 << UCSZ12);               // Включаем 9-й бит
}

boolean playStatus = false;
boolean initial = false;
boolean diskok = false;
int cycle_init = 1; 

void loop() {
  // Чтение команды от магнитолы
  if (Serial1.available()) {
    uint8_t ninthBit = (UCSR1B & (1 << RXB81)) ? 1 : 0; // Чтение 9-го бита
    uint8_t incoming = Serial1.read(); // Чтение младших 8 бит

    // Лог входящих данных
    Serial.print("Получено от магнитолы: 9th bit: ");
    Serial.print(ninthBit);
    Serial.print(", Data: ");
    Serial.println(incoming, HEX);

    if (!initial) {
      initialization(incoming, ninthBit);
    }
    if (initial && !diskok){
      loadDisk();
    }
  }
}

void initialization(uint8_t incoming, uint8_t ninthBit) {
  if (incoming == 0x80 && ninthBit == 1) { // "0x180" = 0x80 + 9-й бит
    sendResponse(0x180);
  }

  if (cycle_init == 1) {  // Цикл 1
    Serial.println("***Цикл: 1");

    if (incoming == 0xAD && ninthBit == 1) {
      sendResponse(0x1AD);
    } else if (incoming == 0x80 && ninthBit == 0) {
    sendResponse(0x080);
    } else if (incoming == 0x4F && ninthBit == 0) {
      cycle_init = 2;
    }
  } 
  else if (cycle_init == 2) {  // Цикл 2
    Serial.println("***Цикл: 2");

    if (incoming == 0xAD && ninthBit == 1) {
      sendResponse(0x1AD);
    } else if (incoming == 0x80 && ninthBit == 0) {
      sendResponse(0x080);
    } else if (incoming == 0x4F && ninthBit == 0) {
      sendResponse(0x04F);
    } else if (incoming == 0x4F && ninthBit == 1) {
      cycle_init = 3;
    }
  }
  else if (cycle_init == 3) {  // Цикл 3

    Serial.println("***Цикл: 3");

    delay(30);
    sendResponse(0x10F);
    delay(21);
    sendResponse(0x048);
    delay(21);
    sendResponse(0x001);
    delay(21);
    sendResponse(0x14F);

    cycle_init = 4;
  } 
  else if (cycle_init == 4) {  // Цикл 4
    Serial.println("***Цикл: 4");

    if (incoming == 0x48 && ninthBit == 1) {
      sendResponse(0x048); 
    } else if (incoming == 0x4F && ninthBit == 0){
      sendResponse(0x04F); 
    } else if (incoming == 0x02 && ninthBit == 0) {
      sendResponse(0x002);
    } else if (incoming == 0x4F && ninthBit == 1) {
      cycle_init = 5;
      Serial.println("Переключение на 9600 бод");
      delay(10); // Задержка перед переключением скорости
      Serial1.end();
      Serial1.begin(9600);
      UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 9 бит данных
      UCSR1B |= (1 << UCSZ12);
      Serial.println(" ПЕРЕКЛЮЧЕНО ");
      initial = true;
    }
    
  } 
}

void loadDisk(){
  Serial.println("Диск ок");
  sendResponse(0x101);
  sendResponse(0x001);
  sendResponse(0x001);
  sendResponse(0x14F);
  diskok = true;
  playStatus = true;
  
}

// Функция отправки ответа с 9-м битом
void sendResponse(uint16_t data) {
  if (data & 0x100) {
    UCSR1B |= (1 << TXB81); // Устанавливаем 9-й бит
  } else {
    UCSR1B &= ~(1 << TXB81); // Сбрасываем 9-й бит
  }
  Serial1.write(data & 0xFF); // Отправляем младшие 8 бит

  // Лог отправки
  Serial.print("Отправлено магнитоле: 9th bit: ");
  Serial.print((data & 0x100) ? 1 : 0);
  Serial.print(", Data: ");
  Serial.println(data & 0xFF, HEX);
}
