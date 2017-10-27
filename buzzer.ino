void buzzer(void)
{
  for(int i=800;i<1500;i+=100)
  {
    tone(BUZZ,i);
    delay(100); 
  }
  delay(100);
  for(int i=1500;i>200;i-=100)
  {
    tone(BUZZ,i);
    delay(100); 
  }


// delay(1000);
  noTone(BUZZ);
  delay(100);  
}

void buzzerTest()
{
    tone(BUZZ,440);
    delay(2000);   
    noTone(BUZZ);
}

