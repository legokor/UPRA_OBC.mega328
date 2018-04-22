void ecam_init(void)
{
  _Serial.print(F("OBC: INIT EXTERNAL CAMERA..."));

  pinMode(MENU, OUTPUT);
  digitalWrite(MENU, LOW);
  pinMode(SHTR, INPUT);
  digitalWrite(SHTR, LOW);

  _Serial.println(F("OK"));

}

void ecam_pressSHTR(int duration)
{
  _Serial.print(F("OBC: ECAM SHUTTER..."));
  pinMode(SHTR, OUTPUT);
  digitalWrite(SHTR, HIGH);
  delay(duration);
  digitalWrite(SHTR, LOW);
  pinMode(SHTR, INPUT);
  _Serial.println(F("PRESSED"));
}

void ecam_pressMENU(int duration)
{
  _Serial.print(F("OBC: ECAM MENU..."));
  digitalWrite(MENU, HIGH);
  delay(duration);
  digitalWrite(MENU, LOW);
  _Serial.println(F("PRESSED"));
}


void ecam_ON(void)
{
  ecam_pressMENU(100);
  is_ecam_on = true;
  _Serial.println(F("OBC: ECAM ON"));
}

void ecam_OFF(void)
{
  ecam_pressMENU(1000);
  is_ecam_on = false;
  _Serial.println(F("OBC: ECAM OFF"));
}

void ecam_VideoMode(void)
{
  ecam_pressMENU(100);
  delay(500);  
  ecam_pressMENU(100);
  delay(500);  
  ecam_pressMENU(100);
  delay(500);  
}

void ecam_PictureMode(void)
{
  ecam_pressMENU(100);
  delay(500);  
}


