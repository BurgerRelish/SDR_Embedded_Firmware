#include <Arduino.h>
#include "Lexer.h"

void setup() {
  // put your setup code here, to run once:
  delay(100);
  ps_string str = "x+y";
  Lexer tokenizer(str);
  tokenizer.tokenize();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(250);
}
