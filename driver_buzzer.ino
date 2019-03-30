bool is_buzzer_present = false;
bool is_buzzer_on;

int32_t buzzer_init(int pin)
{
    if(is_buzzer_present)
    {
      return -1;
    }
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    is_buzzer_on = false;
    is_buzzer_present = true;
    return 0;
}

void buzzer_on(int pin)
{
    digitalWrite(pin, LOW);
    is_buzzer_on = true;
}

void buzzer_off(int pin)
{
    digitalWrite(pin, HIGH);
    is_buzzer_on = false;
}

void buzzer_toggle(int pin)
{
  if(is_buzzer_on)
  {
    buzzer_off(pin);
  }
  else
  {
    buzzer_on(pin);
  }
}

bool buzzer_is_on(void)
{
  return is_buzzer_on;
}

void buzzer_beep(int pin, int len)
{
    digitalWrite(pin, LOW);
    delay(len); 
    digitalWrite(pin, HIGH);
}

