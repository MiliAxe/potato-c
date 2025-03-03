#ifndef TIMER_H__
#define TIMER_H__
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define SECONDS_IN_MINUTES 60
#define MINUTES_IN_HOUR    60
#define SECONDS_IN_HOUR    SECONDS_IN_MINUTES*MINUTES_IN_HOUR

typedef enum { 
  NULL_TYPE = -1,
  POMODORO_TYPE = 0,
  SHORT_BREAK_TYPE,
  LONG_BREAK_TYPE,
} TimerType;

typedef struct {
  _Bool paused;
  unsigned int seconds;
  TimerType type;
  unsigned int pomodoro_count;
} Timer;

void Timer_sleep_reduce_second(Timer *timer);
void Timer_set_seconds_based_on_type(Timer * timer);
void Timer_cycle_type(Timer * timer);
void Timer_initialize(Timer *timer);
void Timer_print(Timer *timer);
char * Timer_time_left(Timer *timer);
void Timer_pause(Timer * timer);
void Timer_unpause(Timer * timer);
void Timer_toggle_pause(Timer * timer);
void Timer_print_before_time(Timer timer);
#endif
