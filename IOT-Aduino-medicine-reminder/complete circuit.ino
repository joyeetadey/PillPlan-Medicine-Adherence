#include <Keypad.h>
#include <LiquidCrystal.h>

// LCD

#define LCD_ROWS 2
#define LCD_COLS 16

#define MSG_ERR_R1   "     Error!     "
#define ERROR_VISIBILITY_TIMEOUT 500

#define MSG_ERR_DAY  "  Invalid day   "
#define MSG_ERR_HOUR "  Invalid hour  "
#define MSG_ERR_MIN  "Invalid minutes "
#define MSG_ERR_DIS  "Invalid dispensr"

/* Alarm menu */

#define MSG_NO_ALRM_R1 "   No alarms    "
#define MSG_NO_ALRM_R2 " #: Create new  "

#define MSG_EDIT_TOOLS "#   A  B  C  D  "
#define MSG_HOUR       " Hour(0-23)    #"
#define MSG_MINUTES    " Minutes(0-59) #"
#define MSG_DISPENSER  " Dispenser     #"
#define MSG_CONFIRM    "Sure? Yes:# No:C"

/* Week menu */

#define MSG_DAY_NUM     "Day number(1-7) "
#define MSG_WEEK_TOGGLE " A - toggle     "

/* Main menu */

#define MSG_MAIN_R1     "  A: Week days  "    
#define MSG_MAIN_R2     "  B: Alarms     "

// Keypad

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// Alarms

#define ALARMS_PER_DAY 10
#define WEEK_LENGTH 7

#define ALARM_PIN     11
#define BTN_PIN       12
#define DISPENSER_PIN 10

int i, j; //variables for loops

/* 
 * --------------------------------------------- 
*/

//
// LCD
//

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

void printToLCD(int rowID, char message[]) 
{
  lcd.setCursor(0, rowID);
      
  for(i = 0; i < LCD_COLS; i++) 
  {
    lcd.write(message[i]);
    delay(1); //TinkerCad optimization
  }
}

void printErrorToLCD(char message[]) 
{
  printToLCD(0, MSG_ERR_R1);
  printToLCD(1, message);

  delay(ERROR_VISIBILITY_TIMEOUT);
}

//
// Keypad
//

char keys[KEYPAD_ROWS][KEYPAD_COLS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[KEYPAD_ROWS] = {9, 8, 7, 6};
byte colPins[KEYPAD_COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

//
// Days of week
//

char weekLetters[] = {' ', 'M', 'T', 'W', 'H', 'F', 'S', 'U'};

int toggleDayIndex;

void toggleWeekDayEnable() 
{
   weekLetters[toggleDayIndex] = (isupper(weekLetters[toggleDayIndex])) 
     				 ? tolower(weekLetters[toggleDayIndex]) 
     				 : toupper(weekLetters[toggleDayIndex]);
}

//
// Alarms
//

class TAlarm 
{
  public:
    char *day, hour, minutes, dispenserId = -1;
};

TAlarm alarms[WEEK_LENGTH * ALARMS_PER_DAY];

int firstEmptyAlarmIndex() 
{
  for(i = 0; i < sizeof(alarms)/sizeof(*alarms); i++) 
  {
    if (alarms[i].dispenserId == -1) break;
  }
  return (i == sizeof(alarms)/sizeof(*alarms)) ? -1 : i;
}

int nextAlarmIndex(int currIndex) 
{
  for(i = currIndex + 1; i < sizeof(alarms)/sizeof(*alarms); i++) 
  {
    if (alarms[i].dispenserId != -1) break;
  }
  return (i == sizeof(alarms)/sizeof(*alarms)) ? -1 : i;
}

int prevAlarmIndex(int currIndex) 
{
  for(i = currIndex - 1; i >= 0; i--) 
  {
    if (alarms[i].dispenserId != -1) break;
  }
  return i;
}

/* Alarming */

bool buttonIsPressed() {
  return digitalRead(BTN_PIN); 
}

void alarm() {
  while(buttonIsPressed()) {
    digitalWrite(ALARM_PIN, HIGH);
    delay(100);
  }
  digitalWrite(ALARM_PIN, LOW);
}

/* Dispensing */

void dispensePill(char dispenserNumber) {
   digitalWrite(DISPENSER_PIN, HIGH);
   delay(dispenserNumber);
   digitalWrite(DISPENSER_PIN, LOW);
}

/* Tracking time */

char maxDays[] = {-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 30, 30, 31};
int hours = 9;
int minutes = 15;
int seconds = 55;
int day = 9;
int weekdayIndex = 4;
int month = 7;
int year = 2020;
char time[17];

//
// Menus
//

void (*menu)(char);
bool confirm;

int alarmIndex = -1;

/* Alarm edit submenu */

char formatted[LCD_COLS + 1];

enum awaitingValue 
{
  awaitingNothing = -1, awaitingDay, awaitingHour, awaitingMinutes, awaitingDispenser, awaitingDeletion
};

#define AV_CONFIRM  0
#define AV_AWAITING 1
#define AV_VALUE    2

char awaitVal[3] = {false, awaitingNothing, -1};

void awaitValReset() 
{
  awaitVal[AV_CONFIRM] = false, awaitVal[AV_AWAITING] = awaitingNothing, awaitVal[AV_VALUE] = -1; 
}

bool validateValue(int value) 
{
  switch(awaitVal[AV_AWAITING]) 
  {
    case awaitingDay:      return value >= 1 && value <= WEEK_LENGTH;
    case awaitingHour:     return value >= 0 && value <= 23;
    case awaitingMinutes:  return value >= 0 && value <= 59;
    case awaitingDispenser:return value >= 0;
  }
}

void setAVNum(char key) 
{
  if (validateValue((awaitVal[AV_VALUE] * 10) + (key - '0'))) 
  {
    awaitVal[AV_VALUE] *= 10;
    awaitVal[AV_VALUE] += key - '0';
    editingSubMenu('#');
  }
  else if (awaitVal[AV_VALUE] < 0 && validateValue(key - '0')) 
  {
    awaitVal[AV_VALUE] = key - '0';
  }
  else 
  {
    switch(awaitVal[AV_AWAITING]) 
    {
      case awaitingDay: printErrorToLCD(MSG_ERR_DAY); break;
      case awaitingHour: printErrorToLCD(MSG_ERR_HOUR); break;
      case awaitingMinutes: printErrorToLCD(MSG_ERR_MIN); break;
      case awaitingDispenser: printErrorToLCD(MSG_ERR_DIS); break;
    }
    
    awaitValReset();
    alarmMenu('~'); editingSubMenu('~');
  }
}

void editingSubMenu(char key) 
{
  switch(key) 
  {
    case 'A': awaitVal[AV_AWAITING] = awaitingDay; printToLCD(1, MSG_DAY_NUM);
      break;
    case '1': case '2': case '3': case '4': case '5': 
    case '6': case '7': case '8': case '9': case '0':
   	  if (awaitVal[AV_AWAITING] != awaitingNothing) setAVNum(key);
      if (awaitVal[AV_AWAITING] == awaitingDay) editingSubMenu('#');
    
      break;
    case 'B': awaitVal[AV_AWAITING] = awaitingHour; printToLCD(1, MSG_HOUR); break;
    case 'C':
      if (awaitVal[AV_CONFIRM]) 
      {
        editingSubMenu('*');
      } 
      else 
      {
        awaitVal[AV_AWAITING] = awaitingMinutes;
        printToLCD(1, MSG_MINUTES);
      }
    
      break;
    case 'D': awaitVal[AV_AWAITING] = awaitingDispenser; printToLCD(1, MSG_DISPENSER); break;
    case '#':
      if (awaitVal[AV_AWAITING] == awaitingNothing) 
      {
        awaitVal[AV_AWAITING] = awaitingDeletion;
      }
      
      if (awaitVal[AV_CONFIRM]) 
      {
        switch(awaitVal[AV_AWAITING]) 
        {
          case awaitingDay: alarms[alarmIndex].day = &weekLetters[awaitVal[AV_VALUE]]; break;
          case awaitingHour: alarms[alarmIndex].hour = awaitVal[AV_VALUE]; break;
          case awaitingMinutes: alarms[alarmIndex].minutes = awaitVal[AV_VALUE]; break;
          case awaitingDeletion:
          case awaitingDispenser: alarms[alarmIndex].dispenserId = awaitVal[AV_VALUE]; break;
        }
        awaitValReset();
        editingSubMenu('*');
      }
      else 
      {
        awaitVal[AV_CONFIRM] = true;
        printToLCD(1, MSG_CONFIRM);
      }
    
      break;
    case '*': awaitValReset(); menu = alarmMenu; menu('~'); break;
    case '~': printToLCD(1, MSG_EDIT_TOOLS); break;
  }
}

/* Alarm menu */

void printAlarmMenu() 
{
  if (alarmIndex < 0) 
  {
    printToLCD(0, MSG_NO_ALRM_R1);
    printToLCD(1, MSG_NO_ALRM_R2);
  }
  else 
  {
    snprintf(formatted, LCD_COLS + 1, "%02i  %c  %02i:%02i %02i ", alarmIndex, *alarms[alarmIndex].day, alarms[alarmIndex].hour, alarms[alarmIndex].minutes, alarms[alarmIndex].dispenserId);
    printToLCD(0, formatted);
    
    snprintf(formatted, LCD_COLS + 1, "%c        C     %c", (prevAlarmIndex(alarmIndex) >= 0) ? 'A' : 'a' , (alarmIndex < nextAlarmIndex(alarmIndex)) ? 'B' : 'b');
    printToLCD(1, formatted);
  }
}

void alarmMenu(char key) 
{
  switch(key) 
  {
    case 'A':
      if (prevAlarmIndex(alarmIndex) >= 0) 
      {
        alarmIndex--; 
        printAlarmMenu(); 
      } 
    
      break; 
    case 'B': 
      if (alarmIndex < nextAlarmIndex(alarmIndex)) 
      { 
        alarmIndex++; 
        printAlarmMenu(); 
      } 
    
      break;
    case 'C': menu = editingSubMenu; menu('~'); break;
    case '#': 
      alarmIndex = firstEmptyAlarmIndex();
      menu = editingSubMenu; 
      alarmMenu('~'); menu('D');
      break;
    case '*': alarmIndex = 0; menu = alarmSettingsMenu; menu('~'); break;
    case '~': printAlarmMenu(); break;
  }
}

/* Weekday menu */

char selWeekdayIndex;

void printWeekdayMenu() {
  if (confirm) {
   	printToLCD(0, MSG_CONFIRM);
  }
  else {
    printToLCD(0, MSG_DAY_NUM); 
  }
  printToLCD(1, "                ");
}

void weekdayMenu(char key) {
  switch(key) {
    case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7':
      if(!confirm) {
         selWeekdayIndex = key - '0';
         confirm = true;
         weekdayMenu('~');
      }
      break;
    case '#':
      if(confirm) {
         confirm = false;
         weekdayIndex = selWeekdayIndex;
         weekdayMenu('*');
      }
      break;
    case 'C': confirm = false; weekdayMenu('~'); break;
    case '*': menu = mainMenu; menu('~'); break;
    case '~': printWeekdayMenu(); break;
  }
}

/* Week menu */

bool waitForDayNum;

void printWeekMenu() 
{
  sprintf(formatted," %c %c %c %c %c  %c %c ", weekLetters[1], weekLetters[2], weekLetters[3], weekLetters[4], weekLetters[5], weekLetters[6], weekLetters[7]);
  printToLCD(0, formatted);
  
  if (waitForDayNum) printToLCD(1, MSG_DAY_NUM);
  else if (confirm) printToLCD(1, MSG_CONFIRM);
  else printToLCD(1, MSG_WEEK_TOGGLE);
}

void weekMenu(char key) 
{  
  switch(key)
  {
    case 'A': waitForDayNum = true; weekMenu('~'); break;
    case 'C': confirm = false; weekMenu('~'); break;
    case '#': 
      if (confirm) 
      {
        toggleWeekDayEnable();
      	confirm = false;
      }  
      weekMenu('~');
    
      break;
    case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7':
      if (waitForDayNum) 
      {
      	toggleDayIndex = key - '0'; 
        waitForDayNum = false; confirm = true; 
      } 
      weekMenu('~'); 
    
      break;
    case '*': menu = alarmSettingsMenu; menu('~'); break;
    case '~': printWeekMenu();  break;
  }
}

/* Alarm settings menu */

void alarmSettingsMenu(char key) 
{  
  switch(key)
  {
    case 'A': menu = weekMenu; menu('~'); break;
    case 'B': menu = alarmMenu; menu('~'); break;
    case '~': printToLCD(0, MSG_MAIN_R1); printToLCD(1, MSG_MAIN_R2); break;
    case '*': menu = mainMenu; menu('~'); break;
  }
}

/* Main menu */

void mainMenu(char key) {
  switch(key) {
    case 'A': menu = alarmSettingsMenu; menu('~'); break;
    case 'B': menu = weekdayMenu; menu('~'); break;
    case '~': 
      snprintf(time,17,"    %02i:%02i:%02i  |A", hours, minutes, seconds);
 	  printToLCD(0,time);
    
      snprintf(time,17," %c %02i/%02i/%04i |B", weekLetters[weekdayIndex], day, month, year);
      printToLCD(1,time);
  }
}

//
// Main logic
//

int getActiveAlarmIndex() {
  for(i = 0; i < sizeof(alarms)/sizeof(*alarms); i++) {
    if(*alarms[i].day == weekLetters[weekdayIndex] &&
      	alarms[i].hour == hours &&
      	alarms[i].minutes == minutes) {
      return i;
    }
  }
  return -1;
}

void setup()
{
  lcd.begin(LCD_COLS, LCD_ROWS);
  pinMode(13, INPUT);

  menu = mainMenu;
  mainMenu('~');
}

char key;

unsigned long lastTime;

void loop()
{
  if (menu == mainMenu) {
    menu('~');
  }
  
  key = keypad.getKey(); // getKey function is too quick, once you get the key, the second time you get it it will return null
  if (key != '\0') 
  {
    menu(key);
  }
  
  delay(10);
  
  if (millis() - lastTime >= 1000) {
    lastTime = millis();
    seconds++;

  	if(seconds > 59) {
   	  seconds = 0;
      minutes++;
      
      if (isupper(weekLetters[weekdayIndex])) {
        i = getActiveAlarmIndex();
        if(i > -1) {
          dispensePill(alarms[i].dispenserId);
          delay(3000);
          seconds += 3;
          alarm();
        }
      }
    }
    if (minutes > 59) {
      minutes = 0;
      hours++;
    }
    if (hours > 23) {
      hours = 0;
      weekdayIndex++;
      day++;
    }
    if(weekdayIndex > 7) {
      weekdayIndex = 1;
    }
    if (day > maxDays[month]) {
      day = 1;
      month++;
    }
    if (month > 12) {
      month = 1;
      year++;
    }
  }
}
