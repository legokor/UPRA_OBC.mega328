void buzzer(void)
{
  for(int i = 0; i < 2; i++)
  {
//    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, LOW);
    delay(1000); 
    digitalWrite(BUZZ, HIGH);
//    pinMode(BUZZ, INPUT_PULLUP);
    delay(1000);
  }
  delay(100);
//  pinMode(BUZZ, INPUT_PULLUP);
}

int32_t buzzerTest(uint8_t repeat)
{
  if( repeat == 0)
  {
//    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, LOW);
    delay(5000); 
    digitalWrite(BUZZ, HIGH);
//    pinMode(BUZZ, INPUT_PULLUP);
    return 0;
  }
  
  for(int i = 0; i < repeat; i++)
  {
//    pinMode(BUZZ, OUTPUT);
    digitalWrite(BUZZ, LOW);
    delay(1000); 
    digitalWrite(BUZZ, HIGH);
 //   pinMode(BUZZ, INPUT_PULLUP);
    delay(1000);
  }
  delay(100);
//  pinMode(BUZZ, INPUT_PULLUP);
  return 0;
}

