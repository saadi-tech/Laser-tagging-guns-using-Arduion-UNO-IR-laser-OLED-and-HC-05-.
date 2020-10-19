int gap = 200;  //usecs

//fire codes ... [0,1,0} ===> Friendly Fire.
//               [1,0,0} ===> Enemy Fire.


int fire_code[] = {1,0,0};

void setup() {
  // put your setup code here, to run once:
pinMode(2,OUTPUT);
 
}

bool get_parity(){
  int ones = 0;
  for (int i = 0; i < 3 ; i++){
    if (fire_code[i] == 1){
      ones++;
      
    }
    
  }

  if (ones % 2 == 0){
    return false;
  }
  else{
    return true;
  }
  
}
void send_one(){
  int count = 0;
  while(count < 40){
    digitalWrite(2,HIGH);
    delayMicroseconds(11);
    digitalWrite(2,LOW);
    delayMicroseconds(11);
    count++;
  }
}
void send_zero(){
  int count = 0;
  while(count < 15){
    digitalWrite(2,HIGH);
    delayMicroseconds(11);
    digitalWrite(2,LOW);
    delayMicroseconds(11);
    count++;
  }
}

void send_header(){
   int count = 0;
  while(count < 50){
    digitalWrite(2,HIGH);
    delayMicroseconds(11);
    digitalWrite(2,LOW);
    delayMicroseconds(11);
    count++;
  }
}
void send_pulse(){

  send_header();
  delayMicroseconds(gap);

  for (int i = 0; i < 3 ; i++){
    if (fire_code[i] == 1){
      send_one();
    }
    else{
      send_zero();
    }

    delayMicroseconds(gap);
  }

  if (get_parity()){
    send_one();
    delayMicroseconds(gap);
  }
  else{
    send_zero();
    delayMicroseconds(gap);
  }
}
void loop() {
  delay(1000);
  send_pulse();
  
  // put your main code here, to run repeatedly:

}
