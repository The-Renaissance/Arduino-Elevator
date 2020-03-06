#define FLOORS 4
static const uint8_t buttons[FLOORS] = {8,9,10,11};
static const uint8_t leds[FLOORS] = {4,5,6,7};

typedef enum {
  UP = 0,
  DOWN,
  RETURN
} elevator_dir;

static elevator_dir direction;

static volatile bool floor_requested[FLOORS] = {false, false, false, false};
static volatile int up_requests, down_requests;
static int current_floor;

void setup() {
  noInterrupts();
  // Initialize LEDs and buttons
  for (uint8_t i = 0; i < FLOORS; ++i) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
  }

  // initialize interrupt registers
  PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
  PCICR = (1 << PCIE0);
  
  // In the beginning, the elevator is stationary and stays on the first floor
  digitalWrite(leds[0], HIGH);
  direction = RETURN;
  interrupts();
}

void loop() {
  // change elevator direction based on number of up and down requests
  if (up_requests == 0 && down_requests != 0) {
    direction = DOWN;
  } else if (up_requests != 0 && down_requests == 0) {
    direction = UP;
  } else if (up_requests == 0 && down_requests == 0) {
    direction = RETURN;
  }

  // if no request is received and elevator already on the first floor, do nothing.
  if (direction == RETURN && current_floor == 0) return;

  // based on current floor and direction, determines if the elevator reaches a desired floor.
  // If yes, add 5s to the delay to simulate people going in and out of the elevator
  if (floor_requested[current_floor] == true) {
    floor_requested[current_floor] = false;
    if (direction == DOWN) --down_requests;
    else if (direction == UP) --up_requests;
    delay(5000);
    return;
  }
  delay(1500); // simulated 1.5s delay for traversing one floor
  if (direction == DOWN || direction == RETURN) {
    digitalWrite(leds[current_floor--], LOW);
    digitalWrite(leds[current_floor], HIGH);
  } else if (direction == UP) {
    digitalWrite(leds[current_floor++], LOW);
    digitalWrite(leds[current_floor], HIGH);
  }
}

ISR(PCINT0_vect) {
  for (int i = 0; i < FLOORS; ++i) {
    if (digitalRead(buttons[i]) == LOW) {
      if (floor_requested[i] == false) {
          if (current_floor < i) {
            ++up_requests;
            floor_requested[i] = true;
          } else if (current_floor > i) {
            ++down_requests;
            floor_requested[i] = true;
          }
      }
      break;
    }
  }
}
