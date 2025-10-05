/*
    Weird (one-way) almost half duplex system.
    With 5 bits burst + SYN/ACK.

    Connect:
        Slave   to  Master

        TX (0)      RX (1)
        RX (1)      TX (0)
        GND         GND

    For Master set SENDER=true.
    For Slave set SENDER=false.

*/


#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

const bool SENDER = true;

const int TX_PIN = 1;
const int RX_PIN = 0;
const int BIT_DELAY = 104;


const char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\\.'-()";

void sendChar(char c) {
    int code = -1;
    for (int i = 0; i < sizeof(charSet) -1; i++) {
        if (charSet[i] == c) {
            code = i;
            break;
        }
    }
    if (code == -1) {
        return;
    }
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(BIT_DELAY);
    for (int i = 0; i < 5; i++) {
        digitalWrite(TX_PIN, (code >> i) & 0x01);
        delayMicroseconds(BIT_DELAY);
    }
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(BIT_DELAY);
}

char receiveChar() {
    char received = 0;
    while (digitalRead(RX_PIN) == HIGH);
    delayMicroseconds(BIT_DELAY / 2);
    delayMicroseconds(BIT_DELAY);
    for (int i = 0; i < 5; i++) {
        if (digitalRead(RX_PIN) == HIGH) {
            received |= (1 << i);
        }
    delayMicroseconds(BIT_DELAY);
    }
    delayMicroseconds(BIT_DELAY);
    return charSet[received];
}



void sendString(String s) {
    int string_size = s.length();
    for (int i = 0; i < string_size; i++) {
        char c = s.charAt(i);
        sendChar(c);
        delay(BIT_DELAY);
    }
}

String receiveString() {
    String result = "";
    while (true) {
        char c = receiveChar();
        result += c;
        int len = result.length();
        if (len >= 2 && result[len - 2] == '\\' && result[len - 1] == 'E') {
            break;
        }
    }
    return result;
}



bool syn() {
    digitalWrite(TX_PIN, HIGH);
    digitalWrite(TX_PIN, LOW);

    lcd.setCursor(0, 0);
    lcd.print("S W");
    lcd.setCursor(0, 0);

    while (digitalRead(RX_PIN) == HIGH);
    lcd.print("A C");
    digitalWrite(TX_PIN, HIGH);
    delay(BIT_DELAY);

    return true;
}

bool ack() {
    lcd.setCursor(0, 0);
    lcd.print("L W");
    lcd.setCursor(0, 0);

    while (digitalRead(RX_PIN) == HIGH);
    digitalWrite(TX_PIN, LOW);
    delay(BIT_DELAY);    
    lcd.print("A C");

    return true;
}



void sender() {
    if (syn()) {
        delay(1000);
        sendString("HELLO-WORLD\\E");

        lcd.setCursor(0, 0);
        lcd.print("C C");
        delay(1000);
    }    

}

void receiver() {
    if (ack()) {
        String s = receiveString();
        lcd.setCursor(0, 1);
        lcd.print("                ");  // Clear the line
        lcd.setCursor(0, 1);
        lcd.print(s);
    }
    lcd.setCursor(0, 0);
    lcd.print("C C");
}


void setup() {
    pinMode(TX_PIN, OUTPUT);
    pinMode(RX_PIN, INPUT);
    digitalWrite(TX_PIN, HIGH);

    lcd.begin(16, 2);
}

void loop() {
    if (SENDER == true){
        sender();
        delay(1000); // simulate unsync initial time.
    } else {
        receiver();
    }
}
