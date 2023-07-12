#ifndef PWM_H
#define PWM_H

int PWM_init(int _channel = 1);
int PWM_set(int freq, int duty_cycle);

#endif
