void parpadeoLed()  {
  // Cambiar de estado el LED
  byte estado = digitalRead(pinD);
  digitalWrite(pinD, !estado);
}
