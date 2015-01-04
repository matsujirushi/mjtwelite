////////////////////////////////////////////////////////////////////////////////
// matsujirushi.h
//
// 2014/01/04 0.1 Takashi Matsuoka
//                ・新規作成。
//

#ifndef MATSUJIRUSHI_H_
#define MATSUJIRUSHI_H_

////////////////////////////////////////
// Digital I/O
//

enum pinmode_t
{
	INPUT,
	OUTPUT,
	INPUT_PULLUP,
};

enum digital_t
{
	LOW,
	HIGH,
};

void pinMode(int pin, enum pinmode_t mode);
void digitalWrite(int pin, enum digital_t value);
enum digital_t digitalRead(int pin);

////////////////////////////////////////
// Analog I/O
//

int analogRead(int pin);


#endif /* MATSUJIRUSHI_H_ */

////////////////////////////////////////////////////////////////////////////////
