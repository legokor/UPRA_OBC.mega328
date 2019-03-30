void error_buzzer_init(void)
{
  buzzer_init(BUZZ);
}

void error_buzzer_startup(void)
{
  buzzer_beep(BUZZ,5000);
  delay(2000);
}

void error_buzzer_nominal(void)
{
  buzzer_beep(BUZZ,1500);
  delay(1500);
}

void error_buzzer_error(void)
{
  buzzer_beep(BUZZ,500);
  delay(500);
  buzzer_beep(BUZZ,500);
  delay(500);
  buzzer_beep(BUZZ,500);
  delay(500);
}

void error_buzzer_first_gps_fix(void)
{
  buzzer_beep(BUZZ,1500);
  delay(1000);
  buzzer_beep(BUZZ,1500);
  delay(1000);
}

