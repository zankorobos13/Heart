class LED{
  private:
    uint8_t pin;
    int usual_blink_period_ms;
    int usual_blink_duration_ms;
    int prev_ms;
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

    void Blink(){
      Blink(usual_blink_period_ms);
    }

    void Blink(int blink_period_ms){
      Blink(blink_period_ms, usual_blink_duration_ms);
    }

    void Blink(int blink_period_ms, int blink_duration_ms){
      int ms = millis();
      if (ms - prev_ms >= blink_period_ms - blink_duration_ms){
        On();
      }
      if (ms - prev_ms >= blink_period_ms){
        Off();
        prev_ms = ms;
      }
    }

    void On(){
      digitalWrite(pin, HIGH);
    }

    void Off(){
      digitalWrite(pin, LOW);
    }
};

