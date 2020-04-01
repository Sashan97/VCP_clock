#include <Adafruit_NeoPixel.h>
#include <iarduino_RTC.h>

int lastMinute;
int lastMode;
bool isOn = true;
bool isActive = true;

//settings
int bpower = 1;       // 0 - off, 1 - on
int bbright = 50;      // brightness 0-250
int bmodes = 1;       // 1 - clock, 2 - illumination, 3 - lamp
int bcolor = 1;       // 1 - original, 2 - red, 3 - green, 4 - blue
int bauto = 0;        // automatic start and stop 0 - off, 1 - on
int bautostart = 0;   //start time, if auto is on
int bautostop = 0;    //stop time, if auto is off

//bluetooth data input
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
bool newData = false;

String mode;            // set time (time) or adjust settings (set)

int arg1 = 0;
int arg2 = 0;
int arg3 = 0;
int arg4 = 0;
int arg5 = 0;
int arg6 = 0;
int arg7 = 0;

//LED strips
Adafruit_NeoPixel hourStrip = Adafruit_NeoPixel(36, 13, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel minuteStrip = Adafruit_NeoPixel(72, 12, NEO_GRB + NEO_KHZ800);

uint32_t orig = minuteStrip.Color(252,165,3);
uint32_t currentColor;

//Clock
iarduino_RTC time(RTC_DS3231);

void setup(){
  lastMinute=61;
  currentColor = orig;
  hourStrip.begin();
  minuteStrip.begin();
  minuteStrip.setBrightness(bbright);    // яркость, от 0 до 255
  hourStrip.setBrightness(bbright);
  minuteStrip.clear();                          // очистить
  minuteStrip.show();                           // отправить на ленту
  hourStrip.clear();
  hourStrip.show();
  Selftest();
  time.begin();
  time.settime(0,0,12,1,1,15,1);  // 0  сек, 51 мин, 21 час, 27, октября, 2015 года, вторник
  Serial.begin(9600);
}

void indicate(int hrStrip, int minStrip){
  //часы
  byte hourIndex;
  if(hrStrip < 12) hourIndex = hrStrip * 3;
  else hourIndex = hrStrip * 3 - 36; 
  hourStrip.clear();
  for(int i = hourIndex; i < (hourIndex + 3); i++){
    hourStrip.setPixelColor(i, currentColor);
  }
  hourStrip.show();
  lastMinute = minStrip;
  
  // минуты

  if(minStrip == 0){
    minuteStrip.clear();
    minuteStrip.show();
  }
  byte minToTile = 5;
  int tiles = minStrip + minStrip/5; //кол-во клеток периметра
  for(int i = 0; i < tiles; i++){
    if(minToTile > 0){
      minuteStrip.setPixelColor(i, currentColor);
      minToTile--;
    }
    else{
      minuteStrip.setPixelColor(i, 0x000000);
      minToTile = 5;
    }
  }
  minuteStrip.show();
}

void updateTime(){
  if(millis()%1000==0){ // если прошла 1 секунда
      time.gettime();
      //проверяем изменилось ли время
      indicate(time.Hours, time.minutes);
      //Serial.println(time.gettime("d-m-Y, H:i:s, D")); // выводим время
  }
}

void updateBluetooth(){
  static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void applySettings(){
  bpower = arg1;
  
  bbright = arg2;
  hourStrip.setBrightness(bbright);
  minuteStrip.setBrightness(bbright);

  bmodes = arg3;

  bcolor = arg4;
  switch(bcolor){
    case 1:
      currentColor = orig;
      break;

    case 2:
      Serial.println(currentColor);
      currentColor = minuteStrip.Color(256,1,1);
      Serial.println(currentColor);
      break;

    case 3:
      currentColor = orig;
      break;
      
    case 4:
      currentColor = minuteStrip.Color(0,0,256);
      break;
  }

  bauto = arg5;
  bautostart = arg6;
  bautostop = arg7;
}

void checkNewData(){
  if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        if(mode == "set") applySettings();
        else{
          minuteStrip.clear();
          minuteStrip.show();
          time.settime(arg3, arg2, arg1, arg5, arg4, arg6, arg7);
        }
        newData = false;
  }
}

void parseData(){
  char * strtokIndx; // this is used by strtok() as an index
  
    strtokIndx = strtok(tempChars, ",");
    mode = strtokIndx;
    strtokIndx = strtok(NULL, ",");
    arg1 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg2 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg3 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg4 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg5 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg6 = atoi(strtokIndx);
    strtokIndx = strtok(NULL, ",");
    arg7 = atoi(strtokIndx);
}

void loop(){
  updateBluetooth();
  checkNewData();

  if(bpower == 1){
    isOn = true;
    
    if(bmodes == 1){ 
      if(lastMode == 2){     //if mode1 selected after mode2
        minuteStrip.clear();
        minuteStrip.show();
        lastMode = 1;
      }
      if(bauto == 0) {
        updateTime();
      }
      else if(bauto == 1){
        autoTime();
      }
    }
    else if(bmodes == 2){ // lantern mode
      hourStrip.fill(currentColor);
      hourStrip.show();
      minuteStrip.fill(currentColor);
      minuteStrip.show();
      lastMode = 2;
    }
  }
  else{ //if turned off
    turnoff();
  }
}

void autoTime(){
  if (time.Hours>=bautostart && time.Hours<bautostop){
      updateTime();
  }
  else{
    hourStrip.clear();
    hourStrip.show();
    minuteStrip.clear();
    minuteStrip.show();
  }
}

void turnoff(){
  if(isOn){
    hourStrip.clear();
    hourStrip.show();
    minuteStrip.clear();
    minuteStrip.show();
    isOn = false;
  }
}

void Selftest(){
  hourStrip.setPixelColor(33, 0xff0000);
  hourStrip.setPixelColor(34, 0xff0000);
  hourStrip.setPixelColor(35, 0xff0000);
  hourStrip.setPixelColor(0, 0xff0000);
  hourStrip.setPixelColor(1, 0xff0000);
  hourStrip.setPixelColor(2, 0xff0000);
  minuteStrip.setPixelColor(70, 0xff0000);
  minuteStrip.setPixelColor(0, 0xff0000);
  hourStrip.show();
  minuteStrip.show();
  delay(500);
  hourStrip.setPixelColor(33, 0x008000);
  hourStrip.setPixelColor(34, 0x008000);
  hourStrip.setPixelColor(35, 0x008000);
  hourStrip.setPixelColor(0, 0x008000);
  hourStrip.setPixelColor(1, 0x008000);
  hourStrip.setPixelColor(2, 0x008000);
  minuteStrip.setPixelColor(70, 0x008000);
  minuteStrip.setPixelColor(0, 0x008000);
  hourStrip.show();
  minuteStrip.show();
  delay(500);
  hourStrip.setPixelColor(33, 0x0000ff);
  hourStrip.setPixelColor(34, 0x0000ff);
  hourStrip.setPixelColor(35, 0x0000ff);
  hourStrip.setPixelColor(0, 0x0000ff);
  hourStrip.setPixelColor(1, 0x0000ff);
  hourStrip.setPixelColor(2, 0x0000ff);
  minuteStrip.setPixelColor(70, 0x0000ff);
  minuteStrip.setPixelColor(0, 0x0000ff);
  hourStrip.show();
  minuteStrip.show();
  delay(500);
  hourStrip.clear();
  minuteStrip.clear();
  hourStrip.show();
  minuteStrip.show();
  delay(1000);
}
