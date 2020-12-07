
#include "MessageStack.hpp"

TaskHandle_t Task0;
TaskHandle_t Task1;

void setup() {
  Serial.begin(115200);
  //delay(2000);
  Serial.printf("Start setup\n");

  MessageStack::MessagesStack.begin();
  randomSeed(analogRead(0));
  
  //create a task that will be executed in the Task0code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task0code,   /* Task function. */
    "Task0",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task0,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */
  Serial.printf("Task0 created\n");

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
  
  Serial.printf("Task1 created : end of setup\n");
}

//Task0code
void Task0code( void * pvParameters ) {
  Serial.printf("Task0 running on core : %d\n", xPortGetCoreID());

  char mess[40];
  char value[10];
  int counter = 0;

  for (;;) {
    long qty = random(1, 10);
    for (int i = 0; i < qty; i++)
    {
      strcpy(mess, "mess ");
      itoa(counter++, value, 10);
      strcat(mess, value);

      if (MessageStack::MessagesStack.PushMessage(mess) == false)
        Serial.println(F("Error : a message has been lost ! Stack is full !"));
    }
    
    long delayV = random(10, 500);
    delay(delayV);
  }
}

//Task1code
void Task1code( void * pvParameters ) {
  Serial.printf("Task1 running on core : %d\n", xPortGetCoreID());

  char buffer[MESSAGE_MAXSIZE];
  int pending = 255;

  for (;;) {
    pending = MessageStack::MessagesStack.GetPendingMessageIndex();

    if (pending != 255)
    {
      MessageStack::MessagesStack.GetMessage(pending, buffer);
      Serial.printf("%d : %s\n", MessageStack::MessagesStack.GetCount(), buffer);
    
      long delayV = random(10, 100);
      delay(delayV);
    }
  }
}

bool started = false;

void loop() 
{
  if (!started)
    Serial.printf("loop\n");
  started = true;
}
