#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>


// You can use any (4 or) 5 pins
#define sclk 13
#define mosi 11
#define cs   10
#define rst  9
#define dc   8


// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// Option 1: use any pins but a little slower
//Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);

// Option 2: must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

struct matrix_cell
{
  byte x;
  byte y;
};

int score = 0;
int snake_len = 5;
byte cell_size = 3;

byte x_left = 3;
byte x_right = 90;
byte y_down = 58;
byte y_up = 13;

byte matrix_cell_x = 30; // (90 - 3)/3 + 1 = 29 + 1
byte matrix_cell_y = 16; // (58-13)/3 + 1 = 15 + 1 

matrix_cell snake[480]; // snake array
matrix_cell fruit; // position of fruit

void draw_snake(void);
void clear_disp(void);

void go_right(void);
void go_left(void);
void go_up(void);
void go_down(void);

bool snake_out = false;
void snake_out_of(void); // snake out of display

bool snake_ate = false;     // snake ate fruit
bool fruit_snake = false;   // fruit position is the same as snake position
bool snake_crossed = false; // snake crossed herself

void fruit_gener(void);     // generate new coordinates of fruit
void snake_ate_fruit(void); // check if snake ate fruit
void snake_cross(void);     // check if snake crossed herself

byte s_direction = 3; // direction of snake 0=up; 1=down; 2=left; 3=right;
                      // 5 = game over;

void snake_makes_decision(void); // snake makes decision to solve where to go

void setup(void) { 
  display.begin();
  display.fillScreen(BLACK);

  display.setCursor(15,0);
  display.print("score = ");
  display.setCursor(65, 0);
  display.print(score);

  //score = 100;
  //delay(2000);
  //display.setCursor(65, 0);
  display.fillRect(65, 0, 20, 10, BLACK); // erase previous number
  display.setCursor(65, 0);
  display.print(score);

  display.drawRect(1,11, 94, 52, RED);
  //display.fillRect(65, 0, 20, 10, BLACK); // erase previous number
  //display.setCursor(65, 0);
  //display.print(123);

  for(int len = 0; len < snake_len; len++){
    snake[len].x = 4 - len;
    snake[len].y = 0;
  }

  
  pinMode(A0, INPUT);
  PORTC &= 0b11111110; // turn off pullup resitsor
  randomSeed(analogRead(0)); // to generate random fruit position

  fruit_gener();
  
  draw_snake();
  
}

void loop() {
  switch(s_direction) // direction of snake 0=up; 1=down; 2=left; 3=right
  {
    case 0:
    go_up();
    break;

    case 1:
    go_down();
    break;

    case 2:
    go_left();
    break;

    case 3:
    go_right();
    break;

    case 5:
    display.setCursor(25, 20);
    display.print("GAME OVER");
    break;

    default:
    break;
  }

  if(s_direction!=5) // not game over
  { 
    snake_out_of(); // snake out of display
    snake_cross(); // snake crossed itself
    
    snake_ate_fruit();
    if(snake_ate and s_direction != 5)
    {
       fruit_gener(); // generate new fruit
       snake_ate = false;
    }
    clear_disp();
    draw_snake(); // draw snake and fruit
    snake_makes_decision(); // snakes decides where to go
   }

  delay(100);
  /*
  while( !snake_out ){
    go_right();
    //go_up();
    //go_down();
    clear_disp();
    draw_snake();
    snake_out_of();
    delay(200);
  }
  
  if(snake_out){
    display.setCursor(30, 20);
    display.print("snake out");
  }
  */
}

void draw_snake(void){
  display.fillRect(x_left + snake[0].x*cell_size , y_down - snake[0].y*cell_size , cell_size, cell_size, YELLOW); // head of snake
  
  for(int len = 1; len < snake_len; len++){
    display.fillRect(x_left + snake[len].x*cell_size , y_down - snake[len].y*cell_size , cell_size, cell_size, BLUE);
  }

  // draw fruit
  display.fillRect(x_left + fruit.x*cell_size , y_down - fruit.y*cell_size , cell_size, cell_size, GREEN);
}

void clear_disp(void){
  display.fillRect(2, 12, 91, 50, BLACK);
}

void go_right(void){
  for(int len = snake_len - 1; len > 0; len--){
    snake[len] = snake[len-1];
  }
  
  snake[0].x += 1;
}

void go_left(void){
  for(int len = snake_len - 1; len > 0; len--){
    snake[len] = snake[len-1];
  }
  snake[0].x -= 1;
}

void go_up(void){
  for(int len = snake_len - 1; len > 0; len--){
    snake[len] = snake[len-1];
  }
  snake[0].y += 1;
}

void go_down(void){
  for(int len = snake_len - 1; len > 0; len--){
    snake[len] = snake[len-1];
  }
  snake[0].y -= 1;
}

void snake_out_of(void){
  if(snake[0].x < matrix_cell_x and snake[0].y < matrix_cell_y){ // if snake[0].x < 0 it is 255 because we are using byte variables
    //snake_out = false;
  }else{
    snake_out = true;
    s_direction = 5; //game over
  }
}

void fruit_gener(void)
{
  fruit_snake = false;
  fruit.x = byte(random(matrix_cell_x));
  fruit.y = byte(random(matrix_cell_y));
  
  for(int i=0; i<snake_len; i++)
  {
    if(snake[i].x == fruit.x and snake[i].y == fruit.y)
    {
      fruit_snake = true;
      break;
    }
  }

  while(fruit_snake)
  {
    fruit_gener();
  }
}


void snake_ate_fruit(void)
{
  if(snake[0].x == fruit.x and snake[0].y == fruit.y)
  {
    snake_len++;
    score++;
    display.fillRect(65, 0, 20, 10, BLACK); // erase previous number
    display.setCursor(65, 0);
    display.print(score);
    snake_ate = true;
  } 
}


void snake_cross(void)
{
  for(int k=0; k<snake_len; k++)
  {
    for(int i=1+k; i<snake_len; i++)
    {
      if(snake[k].x == snake[i].x and snake[k].y == snake[i].y)
      {
        snake_crossed = true;
        s_direction = 5; //game over
        break;
      }
    }
  }
}


void snake_makes_decision(void){
  
  if(snake[0].x == 29 and snake[0].y == 0){
    s_direction = 0; // go up
  }else if(snake[0].x == 29 and snake[0].y == 1){
    s_direction = 2; // go left
  }else if(snake[0].x == 1 and snake[0].y == 1){
    s_direction = 0; // go up
  }else if(snake[0].x == 1 and snake[0].y == 2){
    s_direction = 3; // go right
  }else if(snake[0].x == 29 and snake[0].y == 2){
    s_direction = 0; // go up
  }else if(snake[0].x == 29 and snake[0].y == 3){
    s_direction = 2; // go left
  }else if(snake[0].x == 1 and snake[0].y == 3){
    s_direction = 0; // go up
  }else if(snake[0].x == 1 and snake[0].y == 4){
    s_direction = 3; // go right
  }else if(snake[0].x == 29 and snake[0].y == 4){
    s_direction = 0; // go up
  }else if(snake[0].x == 29 and snake[0].y == 5){
    s_direction = 2; // go left
  }else if(snake[0].x == 1 and snake[0].y == 5){
    s_direction = 0;
  }else if(snake[0].x == 1 and snake[0].y == 6){
    s_direction = 3;
  }else if(snake[0].x == 29 and snake[0].y == 6){
    s_direction = 0;
  }else if(snake[0].x == 29 and snake[0].y == 7){
    s_direction = 2;
  }else if(snake[0].x == 1 and snake[0].y == 7){
    s_direction = 0;
  }else if(snake[0].x == 1 and snake[0].y == 8){
    s_direction = 3;
  }else if(snake[0].x == 29 and snake[0].y == 8){
    s_direction = 0;
  }else if(snake[0].x == 29 and snake[0].y == 9){
    s_direction = 2;
  }else if(snake[0].x == 1 and snake[0].y == 9){
    s_direction = 0;
  }else if(snake[0].x == 1 and snake[0].y == 10){
    s_direction = 3;
  }else if(snake[0].x == 29 and snake[0].y == 10){
    s_direction = 0;
  }else if(snake[0].x == 29 and snake[0].y == 11){
    s_direction = 2;
  }else if(snake[0].x == 1 and snake[0].y == 11){
    s_direction = 0;
  }else if(snake[0].x == 1 and snake[0].y == 12){
    s_direction = 3;
  }else if(snake[0].x == 29 and snake[0].y == 12){
    s_direction = 0;
  }else if(snake[0].x == 29 and snake[0].y == 13){
    s_direction = 2;
  }else if(snake[0].x == 1 and snake[0].y == 13){
    s_direction = 0;
  }else if(snake[0].x == 1 and snake[0].y == 14){
    s_direction = 3;
  }else if(snake[0].x == 29 and snake[0].y == 14){
    s_direction = 0;
  }else if(snake[0].x == 29 and snake[0].y == 15){
    s_direction = 2;
  }else if(snake[0].x == 0 and snake[0].y == 15){
    s_direction = 1; // go down
  }else if(snake[0].x == 0 and snake[0].y == 0){
    s_direction = 3;
  }

 // byte s_direction = 3; // direction of snake 0=up; 1=down; 2=left; 3=right;
                      // 5 = game over;
}
