double IntTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - INTERNALCALIB ) * INTERNALGAIN; 
  //t = wADC; //CALIB
  // The returned temperature is in degrees Celsius.
  return (t);
}

double getIntTemp(void)
{
  double temp     = 0.0;
  double temp_avg = 0.0;
  double int_temp_tmp = 0.0;
  
  for (int i=0; i<AVRG; i++)
  {
    delay(10);
    int_temp_tmp = IntTemp();
    int_temp_tmp = (int_temp_tmp + CALIBRATION_OFFSET) * CALIBRATION_GAIN;
    temp_avg += int_temp_tmp; 
  }
  
  temp = temp_avg/((double)AVRG);
  return temp;  
}

float getExtTemp(void)
{
  float temp     = 0.0;
  float temp_avg = 0.0;
  
  for (int i=0; i<AVRG; i++)
  {
    temp = ((float)analogRead(A1) * 3.3 / 1023.0) - 0.5;
    temp /= 0.01;
    delay(100);
    temp_avg += temp;  
  }
  
  temp = temp_avg/((float)AVRG);
  return temp;
}

