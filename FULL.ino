//importing libraries for working of I2C lcd...
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//i2c pins
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // 
#include <SoftwareSerial.h>

unsigned long start = 0;
bool con = 0;
SoftwareSerial BTSerial(10, 11);
//arrays for selection...
String accessories[3] = {"Yes", "NO"};
String teams[10] = {"TEAM-1" , "TEAM-2" , "FREE-PLAY" };
String types[6] = {"TIME" , "LIVES","TARGET"};
int durations[] = {10,20,30};

int acc;//                 //0 = yes, 1 = no

bool vest_to_be_attached = 0;
int gap = 200;  //usecs gap between the pulses.

//transmitter pin..
int laser = 2;
//receiver pin
int receiver = 5;

//this array is used to store 4 bits number which is received through IR, (3 BITS are data, 4th is PARITY BIT) for error checking
int times[] = {5,5,5,5};

//game duration.
int game_time = 0;
//lives/
int lives = 0;
//health
int health = 100;
//total bullets.
//AMMO
int total_bullets  = 50;
int mag_size  = 10;
//bullets in magzine
int bullets_in_mag = 10;

//trigger pin = 3
int trigger = 3;
//Reload pin = 4
int reload = 4;

//this will save game type..
int game_type = 0; //0 FOR TIME, 1 LIVES, 2 = TARGET

//team numbers team-1 = 1 , team2 = 2, free-play = 3
int team_no = 0;
//team names:
// A = team-1
// B = team-2
// C = team-3
//fire codes ... [0,1,0} ===> Friendly Fire.
//               [1,0,0} ===> Enemy Fire.

//fire code (3-bits ) which will save the data (team numebr of shooter)
int fire_code[3] ;

void setup() {
  // put your setup code here, to run once:
BTSerial.begin(38400);
 

Serial.begin(9600);
lcd.begin(16,2);
lcd.backlight();//Power on the back light
pinMode(laser,OUTPUT);
pinMode(receiver, INPUT);
pinMode(trigger,INPUT);
pinMode(reload,INPUT);
 
}

// ================================
//this is the major selection tree function
void selection_tree(){
  bool team_select = 0;
  bool type_select = 0;
  int team_index = 0;
  int type_index = 0;
  int tp_selection = 0;
  int tm_selection = 0;

  //first asking for game type selection
  lcd.clear();
  lcd.print("Select game type:");
  lcd.setCursor(0,1);
  lcd.print(types[type_index]);


//while no option selected.
//loop through the options using trigger button
  while(!type_select){
    if (digitalRead(reload)){
    type_index +=1;

    if(type_index == 3){
      type_index = 0;
    }

    lcd.clear();
    lcd.print("Select game type:");
    lcd.setCursor(0,1);
    lcd.print(types[type_index]);
    delay(400);
    }

    if(digitalRead(trigger)){
      tp_selection = type_index;
      type_select = 1;
      game_type = tp_selection;
      delay(400);
    }
    
  }

//showing the selected/ game type..

lcd.clear();
lcd.print("Selected!");
lcd.setCursor(0,1);
lcd.print(types[type_index]);
delay(1000);
lcd.clear();


//now checking if the type selected is TIME
if (type_index == 0){
  
  
  
  //time is selected,..
  
  //now asking for time duration...
  bool time_selected = 0;
  int time_index = 0;
  lcd.clear();
  lcd.print("Game duration:");
  lcd.setCursor(0,1);
  lcd.print(durations[time_index]);

while(!time_selected){
    if (digitalRead(reload)){
    time_index +=1;

    if(time_index == 3){
      time_index = 0;
    }

    lcd.clear();
    lcd.print("Game duration:");
    lcd.setCursor(0,1);
    lcd.print(durations[time_index]);
    delay(400);
    }

    if(digitalRead(trigger)){
      game_time = durations[time_index];
      time_selected = 1;
      
      delay(400);
    }
    
  }
 lcd.clear();
lcd.print("Selected!");
lcd.setCursor(0,1);
lcd.print(durations[time_index]);
delay(1000);
lcd.clear(); 
  
}



//if type was selected 'LIVES'
if(type_index = 1){
  //LIVES chosen....
  //SELECT LIVES HERE 3,
  lives = 3;

  
}


//IF TYPE WAS TARGET

if(type_index == 2){
  total_bullets = 10;
  //TOTAL BULLETS ARE 10
}



//IF THE SELECTION WAS NOT TARGET.... THEN ASKING FOR TEAM SIDE
if (type_index!=2){
  lcd.clear();
    lcd.print("Select your TEAM:");
    lcd.setCursor(0,1);
    lcd.print(teams[team_index]);

while(!team_select){
    if (digitalRead(reload)){
    team_index +=1;

    if(team_index == 3){
      team_index = 0;
    }

    lcd.clear();
    lcd.print("Select your TEAM:");
    lcd.setCursor(0,1);
    lcd.print(teams[team_index]);
    delay(400);
    }

    if(digitalRead(trigger)){
      tm_selection = team_index;
      team_select = 1;
      delay(400);
    }
    
  }



team_no = tm_selection+1;
Serial.println(team_no);
team_code(team_no);
show_code();
lcd.clear();
lcd.print("Selected!");
lcd.setCursor(0,1);
lcd.print(teams[team_index]);
delay(1000);
lcd.clear();
lcd.print("Attaching vest");
lcd.setCursor(1,1);
lcd.print("Wait..plz");

attach_vest();
lcd.clear();
lcd.print("Done");
delay(800);
lcd.clear();
Serial.println("Selected type:");
Serial.println(types[type_index]);
Serial.println("Selected team:");
 Serial.println( teams[team_index]);
}
}
//=================================
//finction to convert binary string pulse into a number

int decode_pulse(){

  //just converting a binary code to a number
  int sum = 0;
  for (int i = 0; i<3; i++){
    
    sum = sum + times[i]*pow(2,(2-i));
    
  }

  return sum;
}
//=================================

int count_ones(){

  //this function counts one's (for parity checking)
  int temp = 0;
  for (int i = 0 ; i < 4 ; i++){
    if (times[i] == 1){
      temp++;
    }
  }
  return temp;
  
}

//=================================

//this function receives and process the IR input signal
void sense_IR(){
  //bool for error checking
  bool fault = 0;

  //if receiver pin is LOW, that means there is a shot being received.. (active LOW)
  if (! digitalRead(receiver)){

  while(! digitalRead(receiver)){
    //waiting for the header pulse to pass through...
  }

  //NOW noting the time in usecs for each incoming pulse...
  times[0] = pulseIn(receiver,LOW,2000);
  times[1] = pulseIn(receiver,LOW,2000);
  times[2] = pulseIn(receiver,LOW,2000);
  times[3] = pulseIn(receiver,LOW,2000);

  //looping through all 4 values
  for (int i = 0; i <4 ; i++){

    if (times[i] <300 || ( times[i] > 500 && times[i]<900 ) || times[i] > 1200){
      times[i] = 4;
      //if singal is not in between any of our required range, then it is possibly from any other source. so error flag is HIGH
      fault = 1;
      Serial.println("Time error");
    }
    if (times[i] > 900 && times[i] < 1200){
      //IF TIME OF PULSE WAS IN BETWEEN 900-1200 IT WAS A ONE...
      times[i] = 1;
    }
    if (times[i] > 300 && times[i] < 500){
      //DETECTING A 0
      times[i] = 0;
    }

    
  }
 
  //checking parity,

  //COUNTING ONES TO CHECK PARITY.
  int ones = count_ones();
  
  if (ones%2 != 0){
    //IF THE ONES ARE ODD, its a parity check error..
  fault = 1;
  Serial.println("PARITY ERROR");
  }

  if (!fault){

    //if there was no ERROR raised, then its a valid shot.
  Serial.println("Shot detected");
  Serial.print(times[0]);
  Serial.print("    ");
  Serial.print(times[1]);
  Serial.print("    ");
  Serial.println(times[2]);
  
  
 //getting the decoded team number from the shot.
  int number = decode_pulse();

  //Serial.print("Decoded Number: ");
  //Serial.println(number);



  if (number == team_no && team_no != 3){

    //if both are same teams and (its not FREE-PLAY MODE), then its a friendly fire.
    Serial.println("Friendly Fire!!");
    
  }
  if (number != team_no  || team_no == 3){
    //if they are different or its  a free - play mode, then its enemy shot
    Serial.println("Enemy Fire!!");
    Serial.println(number);
    Serial.println(team_no);

    //reducing health by 10%
    health-=10;
    lcd.clear();
    lcd.print("HIT...");
    lcd.setCursor(0,1);
    lcd.print("HEALTH: ");
    lcd.print(health);
    
    if(health == 0){
//if healtyh is 0, then die/knwocked down for 10 secs.

      if (game_type == 1){
        //if game type is LIVES, then lives reduce by one..
        lives--;


//if lives are 0, game is over.
        if (lives == 0){
          lcd.clear();
          lcd.print("GAME OVER");
          
          lcd.setCursor(0,1);
          lcd.print("YOU ARE DEAD!!");
          while(1){
            //"GAME OVER"
          }
        }
      }

      //temporary dead
      Serial.println("DEAD!!");
      lcd.clear();
      lcd.print("DEAD!..");

//creatign an interactive counter for temporary dead time count down
      
      for (int x = 0; x < 10;x++){
        lcd.clear();
        lcd.print("DEAD!..");
        lcd.setCursor(0,1);
        lcd.print("Wait ");
        lcd.print(10 - x);
        lcd.print("secs");
        delay(1000);
      }

      //restoring health
      health = 100;

      
    }
    delay(500);
    lcd.clear();
  }
  Serial.println("======");
  }

  
  else{
    Serial.println("Faulty signal, ignored!!");
  }
  
}

//resetting the string, so that one fire should not be tracked twice...
reset_string();
}
//======================================
//fuction to reset hte obtained sequence from the shot

void reset_string(){
  for (int i = 0; i<4 ; i++){
   times[i] = 5;
  }
}
//============================
void team_code(int a){

  //setting team code...
  if ( a == 1){
    fire_code[0] = 0;
    fire_code[1] = 0;
    fire_code[2] = 1;
    
  }
    if ( a == 2){
    fire_code[0] = 0;
    fire_code[1] = 1;
    fire_code[2] = 0;
    
  }
    if ( a == 3){
    fire_code[0] = 0;
    fire_code[1] = 1;
    fire_code[2] = 1;
    
  }
}

void show_code(){
  for (int i = 0; i<3 ;i++){
    Serial.print(fire_code[i]);
  }
  Serial.println("=================");
}
//====================================================
bool get_parity(){
  //function to get parity
  int ones = 0;
  for (int i = 0; i < 3 ; i++){
    if (fire_code[i] == 1){
      ones++;
      
    }
    
  }
//counts ones and return 1 if its odd, and returns 0 if its even,,, 
  if (ones % 2 == 0){
    return false;
  }
  else{
    return true;
  }
  
}

//=================================================
void send_one(){

  //iuf sending a ONE, we use 40-times a pulse, having time-period 23usecs (38kHz) (but here using time period of 22usecs to cater for the time taken by code commands executiyon
  int count = 0;
  while(count < 40){
    digitalWrite(laser,HIGH);
    delayMicroseconds(11);
    digitalWrite(laser,LOW);
    delayMicroseconds(11);
    count++;
  }
}
//====================================================
void send_zero(){
  int count = 0;

  //for sending 0, send only 15 iterations of pulse
  while(count < 15){
    digitalWrite(laser,HIGH);
    delayMicroseconds(11);
    digitalWrite(laser,LOW);
    delayMicroseconds(11);
    count++;
  }
}
//=====================================================
void send_header(){

  //for sending header, we send a 50 iterations long pulse
   int count = 0;
  while(count < 50){
    digitalWrite(laser,HIGH);
    delayMicroseconds(11);
    digitalWrite(laser,LOW);
    delayMicroseconds(11);
    count++;
  }
}

//=======================================
void send_pulse(){

  //this function fires the PULSE
Serial.print("BULLETS in MAG :");
  Serial.println(bullets_in_mag);
  Serial.print("BULLETS :");
  Serial.println(total_bullets);

  
  if ( bullets_in_mag > 0){
    //IF BULLETS IN MAGAZIN ARE
  
  lcd.clear();
  lcd.print("Fire...");

  //SENDING HEADER PULSE
  send_header();
  delayMicroseconds(gap);

//LOOPING THROGH THE CODE AND SENDING CORRESPONDING 0s and ONes.
  for (int i = 0; i < 3 ; i++){
    Serial.print(fire_code[i]);
    if (fire_code[i] == 1){
      send_one();
    }
    else{
      send_zero();
    }

    delayMicroseconds(gap);
  }
Serial.println(" ");
  if (get_parity()){
    send_one();
    delayMicroseconds(gap);
  }
  else{
    send_zero();
    delayMicroseconds(gap);
  }

//reducing bullets in magazine.
  bullets_in_mag -=1;
  total_bullets -=1;
  lcd.setCursor(0,1);
  lcd.print("Mag bullets: ");
  lcd.print(bullets_in_mag);
  
  }
  else{

    //if magazing was empty.
lcd.clear();
lcd.print("EMPTY MAG!");

    //SHOW THAT THERE ARE NO BULLETS IN THE MAGAZINE.
  }

delay(400);
lcd.clear();
}


//==========================

void reload_mag(){
//this code reloads the mag.
  Serial.println("Reloading..");
  lcd.clear();
  lcd.print("RELOADING..");
  
  if (total_bullets / mag_size > 0){
    bullets_in_mag = mag_size;
  }
  else{
    //last mag
    bullets_in_mag = total_bullets;
  }

  delay(700);
  lcd.clear();
}
//=========================================================
void check_trigger(){

  //chck if trigger ws pressed.
  if (digitalRead(trigger)){
    Serial.println("TRIGGERED");
    send_pulse();
    delay(400);
  }
  
}
//========================================================
void attach_ass(){
  ///dunction to ask for assessory attachement.

  
  lcd.clear();
  bool done_selection = 0;
  lcd.print("Attach gear?");
  int index = 0;
  lcd.setCursor(0,1);
  lcd.print(accessories[index]);
  
  while(!done_selection){
    

    if(digitalRead(reload)){
index+=1;
      if (index == 2){
        index = 0;
      }
      lcd.clear();
    lcd.print("Attach gear?");
  
    lcd.setCursor(0,1);
    lcd.print(accessories[index]);
      delay(400);
    }

    if(digitalRead(trigger)){
      lcd.clear();
      lcd.print("Selected:");
  done_selection = 1;
  lcd.setCursor(0,1);
  lcd.print(accessories[index]);
  acc = index;
  
  if (acc == 0){
    vest_to_be_attached = 1;
  }
  else{
    vest_to_be_attached = 0;
  }
  delay(500);
  lcd.clear();
    }
  }
}

//=========================================================
void start_msg(){
  lcd.clear();

  //sending a start msg
  lcd.print("Welcome...");
  delay(1000);
  lcd.clear();
}

void attach_vest(){
  Serial.print("Type: ");
  Serial.println(game_type);
  Serial.print("Team: ");
  Serial.println(team_no);
  
  con = 0;
  start = millis();

  while(millis() - start < 1000){
    BTSerial.write('M');
    Serial.println("sending msg");
    
    
  }
  start = millis();
  Serial.println("Waiting for response");
  while(millis() - start < 3000){
    
    if ( BTSerial.available() > 0){
      byte in = BTSerial.read();
      Serial.print("MSG: ");
      Serial.println(in);
      if (in == 'S'){
        Serial.println("CONNECTED");
        con = 1;
        break;
      }
    }
  }
  if (con){
Serial.println("Sending ACK");
start = millis();
BTSerial.flush();
while(millis() - start < 4000){
  BTSerial.write('a');
}
Serial.println("Sending game type");

start = millis();
while(millis() - start < 1000){
  byte temp = game_type;
  Serial.println(temp);
  BTSerial.write(temp);
}
bool type_ack = 0;
start = millis();
while(millis() - start < 2000){
  if (BTSerial.available() > 0 ){
    byte in = BTSerial.read();
    Serial.println(in);
    if (in == 'a'){
      Serial.println("GAME TYPE ACK DONE");
      type_ack = 1;
      break;
    }
  }
}
BTSerial.flush();
delay(3500);
if (type_ack == 1){
  Serial.println("Sending team");

start = millis();
while(millis() - start < 4000){
  byte temp = team_no;
  Serial.println(temp);
  BTSerial.write(temp);
}
}
Serial.println("done");

}
}



//=========================================================
void loop() {

//maing LOOP
//start msg.
start_msg();

//checking to atach accessories
attach_ass();

//selection tree.
selection_tree();
lcd.print("GAME STARTED..");
delay(300);


while(1){


  //chcekcing for reload
  if(digitalRead(reload)){
    reload_mag();
  }

  //checking for FIRE
check_trigger();

//CHECKING FOR RECEIVED hit
sense_IR();

if( BTSerial.available() >0){
  
  byte inp = BTSerial.read();
  if(inp == 'H'){
    Serial.println("GOT HIT");
    lcd.clear();
    lcd.write("HIT");
    delay(500);
    lcd.clear();
  }
}
  // put your main code here, to run repeatedly:
}
}
