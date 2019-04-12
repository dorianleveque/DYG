#include <Wire.h>
#include <rgb_lcd.h>

class Clock {
  private:
    int hour_left;
    int minute_left;
    int second_left;
    bool start;
    unsigned long current_time;

  public:
    Clock(int hour, int minute, int second)
    : hour_left {hour}, minute_left {minute}, second_left {second}, start {false}, current_time {millis()} {}

    void startClock(){
      this->start = true;
    }
    void stopClock() {
      this->start = false;
    }
    /**
     *  Permet de décrémenter la clock
     *  selon la fréquence d'appel de cette fonction
     */
    void runClock(int period) {      
      if (this->start && !this->isFinished()) {
        if ((millis() - this->current_time) > period) {
          this->current_time = millis();
          this->second_left--;
          
          if (this->second_left < 0) {
            this -> second_left = 59;
            this -> minute_left--;

            if (this->minute_left < 0){
              this -> minute_left = 59;
              this -> hour_left--;
            }
          }
        }
      }
    }
    int compare(int hour, int minute, int second){
      char times[20]; char current_times[20];
      sprintf(times,"%02d%02d%02d",hour, minute, second);
      sprintf(current_times,"%02d%02d%02d",this->hour_left, this->minute_left, this->second_left);
      return strcmp(times, current_times);
    }

    bool isFinished(){
      return (this->hour_left <= 0 && this->minute_left <= 0 && this->second_left <= 0);
    }
    int hour()  { return this -> hour_left;   }
    int minute(){ return this -> minute_left; }
    int second(){ return this -> second_left; }
    /**
     * Fonction qui retourne la valeur de la clock
     * en chaine de caractère
     */
    String toString() {
      char timer[50];
      sprintf(timer, "%02d:%02d:%02d", this->hour_left, this->minute_left,this->second_left);
      return timer;
    }
};

class PassWord {
  private:
    char *symbolList;   // table des symbole
    String mdp;         // mot de passe
    char mdp_display[50]; // mot de passe affiché
    int cursorIndex;  // position du curseur
    bool appui_btn;
    unsigned long current_time;

  public:
    PassWord(char *lettres, String mdp)
    : symbolList { lettres }, mdp {mdp}, cursorIndex{0}, current_time {millis()}, appui_btn{false} {}

    void selectSymbol(float tensionValue){
      if (this->cursorIndex < this->mdp.length()){
        int symbolIndex = map(tensionValue, 0, 1023, 0, strlen(this->symbolList)-1);
        char symbol = this->symbolList[symbolIndex];
        this->mdp_display[this->cursorIndex] = symbol;
      }
      
    }

    void validSymbol(int tensionValue){
      if (tensionValue) {
        if (this->cursorIndex < this->mdp.length()) {
          if (!this->appui_btn) {
            this->cursorIndex ++;
            this->appui_btn = true;
          }
        }
        else {
          this->cursorIndex = 12;
        }
      }
      else {
        this->appui_btn = false;
      }
    }

    bool isCompleted() {
      return (int(this->cursorIndex) == 12);
    }
    
    bool isCorrect(){
      if (int(this->cursorIndex) == 12){
        /*String mdp_compare = String(this->mdp_display);
        String mdp = String(this->mdp);*/
        return String(this->mdp_display) == this->mdp;
      }
    }

    void reset() {
      memset(this->mdp_display,0,sizeof(this->mdp_display));
      this->cursorIndex = 0;
    }

    void display_(rgb_lcd lcd){
      if ((millis() - this->current_time) > 100) {
          this->current_time = millis();
          
          String mdp_display = String(this->mdp_display);
          while(mdp_display.length() < this->mdp.length()){
            mdp_display += "_";
          }
          
          lcd.setCursor(0, 1);
          String disp = "Code: ";
          disp += String(mdp_display);
          lcd.print(disp);
      }
    }
};

/**************************************/
rgb_lcd lcd;

const int colorR = 100;
const int colorG = 100;
const int colorB = 100;
char alphabet[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
Clock clocky {1, 0, 0};
PassWord pass {alphabet, "CESAR"};
bool passWordFind = false;
unsigned long current_time = 0;
bool lcd_clign = false;

void setup() {
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    lcd.setRGB(colorR, colorG, colorB);
    clocky.startClock();
}

void loop() {

  //------------------------------------------
  if (!passWordFind){
    clocky.runClock(1000);
    lcd.setCursor(4, 0);
    lcd.print(clocky.toString());

    /**
     * Clignotement en rouge lorsque la clock est inférieur à 10 min
     */
    if(clocky.compare(0, 0, 1) < 0 && clocky.compare(0, 10, 0) > 0 ) {
      if ((millis() - current_time) > 1000) {           
        current_time = millis();
        if (lcd_clign) { lcd.setRGB(100, 100, 0); } else { lcd.setRGB(100, 100, 100); }
        lcd_clign = !lcd_clign;
        /**
         * Buzzer lorsque la clock est inférieur à 2 min
         */
        if(clocky.compare(0, 0, 1) < 0 && clocky.compare(0, 0, 5) > 0 ) {
          tone(7, 200, 200); // buzzer
        }
      }
    }
    
    if(clocky.compare(0, 0, 0) == 0){
      lcd.setRGB(100, 0, 0);
      tone(7, 440, 2000); // buzzer
      lcd.clear();
      lcd.print("BOOM !");
      delay(2000);
      while(true){
        lcd.clear();
        lcd.print("Vous avez perdu :(");
        delay(2000);
      }
    }
   //---------------------------------------------------

    pass.selectSymbol(analogRead(A0));
    pass.validSymbol(digitalRead(2));
    pass.display_(lcd);

    if (pass.isCompleted()){
      if (pass.isCorrect()){
        passWordFind = true;
      }
      else {    // password incorect
        lcd.setRGB(100, 0, 0);
        tone(7, 440, 200); // buzzer
        pass.reset();
        delay(500);
        lcd.setRGB(100, 100, 100);
      }
    }
  }
  else {
    if (clocky.compare(0, 0, 0)!= 0){
      clocky.runClock(1);
      lcd.setCursor(4, 0);  
      lcd.print(clocky.toString());
    }
    else { 
      lcd.setRGB(0, 100, 0);
      delay(1000);
      lcd.clear();
      lcd.print("Bravo !");
      delay(2000);
      lcd.setCursor(0, 1);
      lcd.print("Bombe désamors");
      lcd.clear();
      lcd.print("Voici le nouveau");
      lcd.setCursor(0, 1);
      lcd.print("mot de passe");
      delay(2000);
      lcd.clear();
      lcd.print("Liberer l'otage:");
      lcd.setCursor(0, 1);
      lcd.print("326");
      delay(10000);
      }
  }
    
}


// code for morse 


  
