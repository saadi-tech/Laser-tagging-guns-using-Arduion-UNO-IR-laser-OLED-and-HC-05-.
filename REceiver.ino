unsigned long start = 0;
int times[] = {5,5,5,5};

void reset_string(){
  for (int i = 0; i<4 ; i++){
   times[i] = 5;
  }
}

void setup() {
  // put your setup code here, to run once:
pinMode(2,INPUT);
pinMode(3,OUTPUT);
Serial.begin(9600);
}
int decode_pulse(){
  int sum = 0;
  for (int i = 0; i<3; i++){
    
    sum = sum + times[i]*pow(2,(2-i));
    
  }

  return sum;
}

int count_ones(){
  int temp = 0;
  for (int i = 0 ; i < 4 ; i++){
    if (times[i] == 1){
      temp++;
    }
  }
  return temp;
  
}
void sense_IR(){
  bool fault = 0;
  if (! digitalRead(2)){

  while(! digitalRead(2)){
    //waiting for the header pulse to pass through...
  }
  
  times[0] = pulseIn(2,LOW,2000);
  times[1] = pulseIn(2,LOW,2000);
  times[2] = pulseIn(2,LOW,2000);
  times[3] = pulseIn(2,LOW,2000);
  for (int i = 0; i <4 ; i++){

    if (times[i] <300 || (times[i] > 500 && times[i]<900) || times[i] > 1200){
      times[i] = 4;
      fault = 1;
      Serial.println("Time error");
    }
    if (times[i] > 900 && times[i] < 1200){
      times[i] = 1;
    }
    if (times[i] > 300 && times[i] < 500){
      times[i] = 0;
    }

    
  }

  //checking parity,
  int ones = count_ones();
  if (ones%2 != 0){
  fault = 1;
  Serial.println("PARITY ERROR");
  }

  if (!fault){
  Serial.println("Shot detected");
  Serial.print(times[0]);
  Serial.print("    ");
  Serial.print(times[1]);
  Serial.print("    ");
  Serial.println(times[2]);
  
  

  int number = decode_pulse();

  //Serial.print("Decoded Number: ");
  //Serial.println(number);

  if (number == 2){
    Serial.println("Friendly Fire!!");
    
  }
  if (number == 3){
    Serial.println("Enemy Fire!!");
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
void loop() {
  // put your main code here, to run repeatedly:
sense_IR();
}
