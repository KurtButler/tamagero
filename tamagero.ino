/* Tamagero v.1.0
    by Kurt Butler

    Yes, this is a Tamagotchi clone. Made for fun
    on the Arduboy platform.
*/
#include <Arduboy2.h>
Arduboy2 ab;
#include "images.h"
#include "strings.h"

// = = = = = = = = = = Variables = = = = = = = = = = = = =
// Program Control
int MenuSt = 0;   //Menu State Variable
int MenuSel = 0;  //Menu Selector Variable
int MenuWind = 0; //Menu window no.
int MenuNextSt = 0;//Menu state to be transitioned to after half an A-press.

// Timing variables
int Twait = 300; //Time to wait after pressing a button.
int eggagemax = 900; //Max egg age before hatching
int aniTimer = 60;
int aniSt = 0;
int aniDir = 1;
int aniX = 0;
int xpcntr = 0;

// Player variables
int xp = 0;
int txp = 0;
int money = 10;
bool dex[25];
bool enLED = false; //Enable LED indicator for Tamagero health

// Tamagero state
float hungry = 0;
float bored  = 0;
float dirty  = 0;
int level  = 0;
bool alive = true;
bool egg   = true; // make sure this is true before running
int age = 0;
int tamaid = 1;

// Other game variables
float hmax = 40000.0; //Max hunger level before running away
float dmax = 55000.0; //Max dirty level befroe running away
float bmax = 12000.0; //Max boredom level
int L = 10;  //temp
const int8_t sprite[32] PROGMEM = {
0x1C, 0x38, 0x30, 0x0C, 0x38, 0x1C, 0x1E, 0x78,
0x3F, 0xFC, 0x7C, 0x3E, 0xC8, 0x13, 0x89, 0x91,
0x89, 0x91, 0xC8, 0x93, 0xFC, 0xBF, 0xBF, 0xFD,
0x9B, 0xEC, 0x95, 0xA4, 0x04, 0x80, 0x00, 0x80};



// -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int dialogue(String string) {
  ab.setCursor(0,0);
  ab.print("\n");
  ab.print(string);
  //some other stuff to make this look nice
  ab.display();

  ab.pollButtons();
  delay(Twait);
  while (!ab.pressed(A_BUTTON)) { ab.pollButtons(); }
  delay(Twait);
  return 0;
}

void setID(int id) {
  // Wrapper that aligns seen Tamagero with the Tamadex listing
  tamaid = id;
  dex[id] = true;
}

void eggReset() {
    hungry = 0; bored = 0; dirty =0; xp = 0;
    age = 0; egg = true; alive = true;
    eggagemax = 600+(rand()%400);
}


// <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <>
//                    Setup Program Code
// <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <>
void setup() {
  // put your setup code here, to run once:
  ab.begin(); //Arduboy splash
  ab.setFrameRate(30);
  ab.initRandomSeed();
  ab.clear();


  // My splash
  ab.setTextColor(WHITE);
  ab.print("Tamagero Ver. 1.0\nKurt Butler");
  ab.display();

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, HIGH);
 
  // ============= Game Init! ==================
  // Initialize Tamadex
  for (int i=0; i<25; i++) {dex[i] = false;}
  setID(0);
 
  delay(1000);
  ab.clear();
  ab.display();
}


// <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <>
//                    Main Program Code
// <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <> <>
void loop() {
  // put your main code here, to run repeatedly:

  //wait and look at buttons
  if (!ab.nextFrame())
    return;
  ab.pollButtons();

  // = = = Update internal variables = = =
  // Menu Update
  if (MenuSt != MenuNextSt) { MenuSel = 0; delay(Twait); }
  MenuSt = MenuNextSt;

  // Tamagero update
  if (alive) {
    if (egg) {
      // Egg Life
      if (age < eggagemax) { age++; }
      else { egg = false; age = 0;  }
    } else {
      //Tamagero life update
      age++;
      if (hungry < hmax) {hungry++;}
      if (dirty  < dmax) {dirty++;}
      if (bored  < bmax) {bored++;}

      //Check if the Tamagero is dead
      if (hungry >= hmax-1 || dirty >= dmax-1) {
        // DEATH
        alive = false;
        hungry = 0; bored = 0; dirty = 0; xp = 0;
        //        #####################
        dialogue(F("Your tamagero has run\naway!"));
      } else {
        // EXP System
        if (xpcntr <= 0) {
          xp++; txp++; xpcntr = 100;
          if (bored <= 0.05*bmax ) {xp++;}
          if (xp > 10 + level*20) {level++; money = money+2 +level;}
        } else {
          xpcntr--;
        }
      }
     
    }
  }

  // = = = Update status LED = = =
  if (!alive || !enLED) { //LED Off
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
  }
  else if (egg) { //LED Blue
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }
  else {
    if (hungry > 0.9 * hmax || dirty > 0.9 * hmax) {//LED Red
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, HIGH);
    } 
    else if (hungry > 0.5 * hmax || dirty > 0.5 * hmax) {//LED Yellow
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
    } 
    else {//LED Green
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
    }
  }


  // = = = = = Menu Control Logic = = = = =
  // Main Menu
  MenuNextSt = MenuSt;
  if (MenuSt == 0) {
    if (ab.pressed(A_BUTTON)) {
      if (MenuSel==0) {MenuNextSt = 2;}
      if (MenuSel==1) {MenuNextSt = 1;}
      if (MenuSel==2) {enLED = !enLED; delay(Twait);}
      }
    else if (ab.pressed(B_BUTTON)) { age = age; }
    else if (ab.pressed(UP_BUTTON) && MenuSel > 0) { MenuSel--; delay(Twait);}
    else if (ab.pressed(DOWN_BUTTON) && MenuSel < 2) { MenuSel++; delay(Twait);}
  }

  // Action Menu
  if (MenuSt == 2) {
    if (ab.pressed(A_BUTTON)) {
      if (MenuSel==0) { // FEED TAMAGERO
        if (bored == bmax) {dialogue(F("Your tamagero refused\nto eat!"));} // Only if not starving
        else {
          if (money>=1) {
            money = money-1;
            hungry = hungry - 0.33*hmax;
            if (hungry < 0) {hungry=0;}
            delay(Twait);
          } else {dialogue(F("You don't have enough\nmoney!"));}
          } 
      }
      if (MenuSel==1) { // CLEAN TAMAGERO
        if (bored == bmax) {dialogue(F("Your tamagero refused\nto bathe!"));}
        else {
            if (money>=1) {
              money = money-1;
              dirty = dirty - 0.33*dmax;
              if (dirty < 0) { dirty = 0; }
              delay(Twait);
            } else {dialogue(F("You don't have enough\nmoney!"));}
          }
      }
      if (MenuSel==2) { // PLAY
        // - - -
          bored = bored - 0.33*bmax;
          if (bored < 0) { bored = 0; }
          delay(Twait);
        // - - -
        }
      if (MenuSel==3) {
        MenuNextSt = 5;
      }
    }
    else if (ab.pressed(B_BUTTON)) { MenuNextSt = 0; delay(Twait); }
    else if (ab.pressed(UP_BUTTON) && MenuSel > 0) { MenuSel = MenuSel - 1; delay(Twait); }
    else if (ab.pressed(DOWN_BUTTON) && MenuSel < 3) {  MenuSel = MenuSel + 1; delay(Twait); }
  }
 
  // Stats Menu
  if (MenuSt == 1) {
    if (ab.pressed(B_BUTTON)) { MenuNextSt = 0; delay(Twait); }
    else if (ab.pressed(A_BUTTON)) {
      if (MenuSel==0) { MenuNextSt = 3;}
      if (MenuSel==1) { MenuNextSt = 4;}
      delay(Twait);
    }
    else if (ab.pressed(UP_BUTTON) && MenuSel > 0) { MenuSel = MenuSel - 1; delay(Twait); }
    else if (ab.pressed(DOWN_BUTTON) && MenuSel < 1) {  MenuSel = MenuSel + 1; delay(Twait); }
  }
 
  // Info Menus
  if (MenuSt == 3 || MenuSt == 4) {
    if (ab.pressed(B_BUTTON)) { MenuNextSt = 1; delay(Twait); }
    else if (ab.pressed(A_BUTTON)) {  MenuNextSt = 0; delay(Twait); }
  }

  // Buy Menu
  if (MenuSt == 5) {
    if (ab.pressed(B_BUTTON)) { MenuNextSt = 2; delay(Twait); }
    else if (ab.pressed(A_BUTTON)) {
        if (MenuSel==0) {//Buy random
          if (money>=10) {
            money = money-10;
            setID(rand()%25);
            eggReset();
            MenuNextSt = 0;
          } else {dialogue(F("You don't have enough\nmoney!"));}
        }
        if (MenuSel==1) {//Buy new
          if (money >= 20) {
            money = money - 20;
            L = rand()%25;
            for (int i=0; i<25; i++) { //Find a new Tamagero!
              if (dex[L]) {L = (L+1)%25;}
              else {break;}
            }
            setID(L);
            eggReset();
            MenuNextSt = 0;
          } else {dialogue(F("You don't have enough\nmoney!"));}
        }
        delay(Twait);
    }
    else if (ab.pressed(UP_BUTTON) && MenuSel > 0) { MenuSel = MenuSel - 1; delay(Twait); }
    else if (ab.pressed(DOWN_BUTTON) && MenuSel < 1) {  MenuSel = MenuSel + 1; delay(Twait); }
}




  // <> <> <> <> <> <> <> <> <>  Update graphics <> <> <> <> <> <> <> <> <>
  ab.clear();
  ab.drawSlowXYBitmap(0, 0, (tamabg), 128,64, WHITE);
  L =22-(22*hungry)/hmax;
  ab.fillRect(23,5,  L,  3,WHITE); //FOOD   (max pix is 22)    // LATER: Make sure these use saturated arithmetic
  L = (22*dirty)/dmax;
  ab.fillRect(23,14, L,  3,WHITE);//DIRT
  L = 22-(22*bored)/bmax;
  ab.fillRect(23,23, L,  3,WHITE);//FUN

  // Draw tamagero sprite
  if (alive) {
    if (egg) {
      ab.drawSlowXYBitmap(75, 15, ntamago, 16, 16, BLACK);
    } else {
      if (aniTimer<=0) { //This is some logic to make the sprite dance
        if (aniSt==0) {aniSt=1; aniTimer=1; aniX++;}
        else if (aniSt==1) {aniSt=2; aniTimer=1; aniX++;}
        else if (aniSt==2) {aniSt=3; aniTimer=1; }
        else if (aniSt==3) {aniSt=4; aniTimer=30 +  (rand() % 30);}
        else if (aniSt==4) {aniSt=5; aniTimer=1;  aniX--;}
        else if (aniSt==5) {aniSt=0; aniTimer=40 +  (rand() % 30);  aniX--; aniDir = 2*(rand()%2)-1;}
      } else {aniTimer--;}
      ab.drawSlowXYBitmap(80+aniDir*aniX, 15-(aniSt%2), (ntamablock + 32*tamaid), 16, 16, BLACK);
    }
  }

 
  // <> <> <> <> <> <> <> <> <> Menu Text <> <> <> <> <> <> <> <> <> <>
  ab.setCursor(0,32);
  if (MenuSt == 0) {// Main Menu
    if (MenuSel==0) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Actions\n"));
    if (MenuSel==1) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Statistics\n"));
    if (MenuSel==2) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Toggle LED\n"));
  }
  if (MenuSt == 2) {// Action Menu
    if (MenuSel==0) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Feed  ($1)    $"));
    ab.print(money);
    ab.print("\n");
    if (MenuSel==1) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Clean ($2)\n"));
    if (MenuSel==2) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Play\n"));
    if (MenuSel==3) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Buy   ($10-20)"));
  }
  if (MenuSt == 1) { // Stats Menu
    if (MenuSel==0) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Tama Stats\n"));
    if (MenuSel==1) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Player Stats\n"));
  }
  if (MenuSt==3) {  // Tamagero Info
  if (!egg) {
    ab.print(tamanames[(tamaid % 25)]);
  } else {ab.print("???");}
    ab.print(" \nAge: ");
    ab.print((((age)/30.0)/3600.0),3);
    ab.print(F(" hr\nXp:  "));
    ab.print(xp);
    ab.print(F("  \nLvl: "));
    ab.print(level);
  }
  if (MenuSt==4) {  //   Player Info
    ab.print(F("Total Xp: "));
    ab.print(txp);
    ab.print(F("\nMoney:    "));
    ab.print(money);
   
    //Tamadex display
    ab.print("\nDex ");
    for (int i=0; i<25; i++) {
      if (dex[i]) {ab.print("o");}
      else {ab.print("-");}
      if (i==12) {ab.print("\n    ");}
    }
  }
  if (MenuSt == 5) { // Buy Tamagero Menu
    if (MenuSel==0) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Buy any ($10)  $"));
    ab.print(money);
    ab.print("\n");
    if (MenuSel==1) {ab.print("*");} else {ab.print(" ");}
    ab.print(F(" Buy new ($20)\n"));
  }
 
  ab.display();
}
