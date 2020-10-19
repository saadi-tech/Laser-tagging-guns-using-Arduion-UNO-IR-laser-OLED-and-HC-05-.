#include <SoftwareSerial.h>
unsigned long start = 0;
SoftwareSerial BTSerial(10, 11);

int team_no = 0;
int receiver = 5;
int game_type = 0;
//this array is used to store 4 bits number which is received through IR, (3 BITS are data, 4th is PARITY BIT) for error checking
int times[] = {5,5,5,5};
bool is_connected = 0;

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

int decode_pulse(){

  //just converting a binary code to a number
  int sum = 0;
  for (int i = 0; i<3; i++){
    
    sum = sum + times[i]*pow(2,(2-i));
    
  }

  return sum;
}

void reset_string(){
  for (int i = 0; i<4 ; i++){
   times[i] = 5;
  }
}

void time_connect(){
  bool m = 0;
  while(!m){
    if (BTSerial.available() >0){
      byte in = BTSerial.read();

      if (in == 'M'){
        Serial.println("GOT MASTER");
        m = 1;
        break;
      }
    }
  }

  start = millis();
 Serial.println("Sending response");
  while(millis() - start < 4000){
    BTSerial.write('S');
  }
  BTSerial.flush();
  start = millis();
  while(millis() - start < 3000){
    if (BTSerial.available() > 0){
      byte x = char(BTSerial.read());
      byte result = 'a';
      Serial.println("================");
      Serial.print("TRUE: ");
      Serial.println(result);
      Serial.print("rec: ");
     
      Serial.println(x);
      if (x == 'a'){
        Serial.println("ACK received");
        break;
      }
    }
  }


  byte x = 0;
  start = millis();
  BTSerial.flush();
  while(millis() - start < 2000){
    if (BTSerial.available() > 0 ){

       x = BTSerial.read();

      if (x == 1 || x==2 || x == 3 || x == 0){
        Serial.println(x);
        break;
      }
    }
  }
  Serial.println("REceived game type:");
  Serial.println(x);
  game_type = x;


  start = millis();
  while(millis() - start <2000){
   BTSerial.write('a');
 }
delay(700);
while(BTSerial.available()>0 ){
  char garbage = BTSerial.read();
}
x = 0;
BTSerial.flush();
Serial.println("TEAM INFO");
  start = millis();
  while(millis() - start < 3000){
    if (BTSerial.available() > 0 ){

       x = BTSerial.read();
Serial.println(x);
      if (x == 1 || x==2 || x == 3){
        Serial.println(x);
        break;
      }
    }
  }

Serial.println("TEAM RECEIVED: ");
Serial.println(x);
team_no = x;
 if ( team_no != 0){
  is_connected = 1;
 }
 Serial.print("Is_connected: ");
 Serial.println(is_connected);
 if(is_connected){
  Serial.println("is_connected");
 }
 
}



//this function receives and process the IR input signal
void sense_IR(){
  //bool for error checking
  bool fault = 0;

  //if receiver pin is LOW, that means there is a shot being received.. (active LOW)
  if (! digitalRead(receiver)){
Serial.println("LOW");
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



  BTSerial.write('H');

   
  Serial.println("======");
  }

  
  else{
    Serial.println("Faulty signal, ignored!!");
  }
  
}

//resetting the string, so that one fire should not be tracked twice...
reset_string();
}
}
//======================================
//fuction to reset hte obtained sequence from the shot





void setup() 
{
  Serial.begin(9600);
  BTSerial.begin(38400);
pinMode(receiver,INPUT);
}

void loop(){
  time_connect();


  if(is_connected){
   while(1){ 
    sense_IR();}
  }
  
}
