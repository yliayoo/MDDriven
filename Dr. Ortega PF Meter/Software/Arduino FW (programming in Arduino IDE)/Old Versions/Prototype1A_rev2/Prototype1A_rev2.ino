int const Ts = 8; 
int i, cal, p,c;
int const L = 738; 
int readVal[L];    
int calSamp = 100;
int ss = L / 6;
int secA[14] = { 
  1, 1, 1, 1, 1, 1, 1, 1, 1, ss * 5, 1, ss * 3, 1 , ss }; //chg
int secB[14] = { 
  1, 1, 1, 1, 1, 1, 1, 1, (L - 1), 1, ss * 4, 1, ss * 2, 1 }; //chg
int trig1 = 8;
int trig2 = 9;
int gkFlag = 0;

void setup()
{
  Serial.begin(57600);          //57600 needed for MINI
  Serial.println("Power On");
  pinMode(13, OUTPUT); //Chg*
  pinMode(12, OUTPUT); //
  delay(510);            //Roving module require 500ms before $$$
  Serial.print("$$$");      //Roving requires no carriage return
  delay(5);                 //Roving requires 5ms after command
}

void loop() { 
  digitalWrite(12,0);
  digitalWrite(13,0);
  Serial.println("");

  //CONFIRM BT CONNECTION WITH PHONE 
  while(1) 
  {
    Serial.flush();
    Serial.println("GK");

    if(Serial.available() > 0) 
      gkFlag = Serial.parseInt();

    delay(10);
    Serial.print("$$$");
    delay(5);

    if( gkFlag == 1) {
      digitalWrite(13,1);
      digitalWrite(12,1);
      delay(100);
      p=1;
      break;
    }
    digitalWrite(12,0);
    delay(300);
    digitalWrite(12,1);
    delay(300);
  }

  //BEGIN TRIGGER MODE
  while(1) 
  {
    while(p==1) {
      delay(1000);
      Serial.print("---");
      Serial.println("");
      delay(1000);
      c =1;
      p =0; 
    }

    while(c==1) {
      delay(1000);
      cal = findCal(calSamp);
      Serial.print("Cal is: ");
      Serial.print(cal);
      c =0; 
    }
    delay(Ts);

    //FIRST CHECK
    readVal[1] = analogRead(A0);
    if (readVal[1] > (cal+trig1))  //fix
    {
      delay(Ts);
      readVal[2] = analogRead(A0);

      //SECOND CHECK
      if (readVal[2] > (cal+trig2))
      {
        int x = 13; //changed
        digitalWrite(12, 0);
        digitalWrite(13, 0);
        delay(Ts);

        //GATHER RAW DATA
        for (i = 3; i < L; i++)
        {
          readVal[i] = analogRead(A0);
          delay(Ts);
          if (i == secA[x]) {        //chg*
            digitalWrite(13, 1);
            digitalWrite(12, 0);
            x = x - 1;
          }
          else if(i == secB[x]) {
            digitalWrite(12, 1);
            digitalWrite(13, 0);
            x = x - 1;
          }
        }

        //Spline
        readVal[0] = cal;
        delay(100);
        blinkTimerOFF();
        delay(500);
        blinkTimerON();
        delay(500);
        blinkTimerOFF();

        //PRINT RAW DATA
        Serial.println("");
        for (i = 0; i < L; i++)
        {
          Serial.print(readVal[i]);
          Serial.print(",");
        }

        //DETERMINE PEF
        float PEF = 0;
        for (i = 0; i < L; i++)
          PEF = max(PEF, readVal[i]);

        //CONVERT PEF
        PEF = bitsToflow(PEF);
        PEF = PEF * 60000;

        //ANALYZE PEF
        if (PEF > 600 || PEF < 400)
          digitalWrite(12, 1);
        else
          digitalWrite(13, 1);

        //PRINT PEF
        Serial.println("");
        Serial.print("PEF in LPM is ");
        Serial.print(PEF, 2);

        //DETERMINE FEV
        float FEV_even = 0;
        float FEV_odd = 0;
        float FEVsimp = 0;
        float FEVtemp = 0;
        float FEV_beg_end[4];
        FEV_beg_end[0] = bitsToflow(readVal[0]);
        FEV_beg_end[1] = bitsToflow(readVal[1]);
        FEV_beg_end[2] = bitsToflow(readVal[L - 2]);
        FEV_beg_end[3] = bitsToflow(readVal[L - 1]);

        //CONVERT FEV
        for (i = 2; i < (L - 3); i = (i + 2))
        {
          FEVtemp = bitsToflow(readVal[i]);
          FEV_even = FEV_even + FEVtemp;
        }
        for (i = 3; i < (L - 2); i = (i + 2))
        {
          FEVtemp = bitsToflow(readVal[i]);
          FEV_odd = FEV_odd + FEVtemp;
        }

        //SIMPSON'S 1/3 COMPOSITE INTEGRATION
        FEVsimp = FEV_beg_end[0] + 4 * (FEV_odd + FEV_beg_end[1]) + 2 * (FEV_even + FEV_beg_end[2]) + FEV_beg_end[3];
        FEVsimp = FEVsimp * 2.7430; //c = (1000L/m^3)*(b-a)/(3n)  = (1000)*6/(3*750) = 1000*TS/3 = 8.229/3
        Serial.println("");
        Serial.print("Simpson's 1/3 rule FEV6 in L is ");
        Serial.print(FEVsimp, 4);
        Serial.println("");
      } //this will be used for the while-trigger loop
    }
  }
}

int findCal(int calSamp)
{
  int cal = 0;
  for (i = 0; i < calSamp; i++)
    cal = cal + analogRead(A0);

  return cal/calSamp;
}

float bitsToflow(int bits)
{
  float a;
  if( (bits>(cal-3)) && (bits<(cal+3)))  //chg*
    bits = cal;
  else
    a = 0.004887585533*(bits - cal); // (5/1023)*bits - [(5/1023)*cal = rest volt]
  if(a < 0)                              
    return -sqrt(-a) * .0079066947;

  return sqrt(a) * .0079066947; //[m^3/s]. Use of alpha constant. 
}

void blinkTimerOFF()
{
  digitalWrite(13, 0);
  digitalWrite(12, 0);
}

void blinkTimerON()
{
  digitalWrite(13, 1);
  digitalWrite(12, 1);
}



