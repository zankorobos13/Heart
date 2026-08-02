class LED{
  private:
    uint8_t pin;
    int usual_blink_period_ms;
    int usual_blink_duration_ms;
    int prev_ms;
    uint8_t brightness = 255;

    static const uint16_t pwm_period_us = 1000;
  public:
    LED(){}

    LED(uint8_t pin, int usual_blink_period_ms, int usual_blink_duration_ms){
      this->pin = pin;
      this->usual_blink_period_ms = usual_blink_period_ms;
      this->usual_blink_duration_ms = usual_blink_duration_ms;
      this->prev_ms = 0;
      Serial.println(this->pin);
      Serial.println(this->usual_blink_period_ms);
      Serial.println(this->usual_blink_duration_ms);
      Serial.println(this->prev_ms);
      
    }

    void SetBrightness(uint8_t value){
      brightness = value;
    }

    void BlinkSequence(String seq){
      for (char c : seq) {
        if (c == '1')
          On();
        else
          Off();
        delay(500);
      }
    }

    void Blink(){
      Blink(usual_blink_period_ms);
    }

    void Blink(int blink_period_ms){
      Blink(blink_period_ms, usual_blink_duration_ms);
    }

    void Blink(int blink_period_ms, int blink_duration_ms){
      unsigned long ms = millis();

      unsigned long elapsed = ms - prev_ms;

      if (elapsed >= blink_period_ms){
          prev_ms = ms;
          elapsed = 0;
      }

      bool ledEnabled = elapsed >= (blink_period_ms - blink_duration_ms);

      if (!ledEnabled){
          digitalWrite(pin, LOW);
          return;
      }

      unsigned long pwmTime = micros() % pwm_period_us;

      uint32_t onTime = (uint32_t)pwm_period_us * brightness / 255;

      if (pwmTime < onTime)
          digitalWrite(pin, HIGH);
      else
          digitalWrite(pin, LOW);
    }

    void On(){
      digitalWrite(pin, HIGH);
    }

    void Off(){
      digitalWrite(pin, LOW);
    }
};

