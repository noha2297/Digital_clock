#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char seconds=0  , minutes =0 , hours  =0;

void Timer1_init();
void INT0_init ();
void INT1_init ();
void INT2_init ();
void Enable7Segment (int pinNum , int data );

ISR (TIMER1_COMPA_vect)
{
	seconds ++;
	if (seconds == 60)
	{
		minutes ++ ;
		seconds =0;
	}
	if (minutes == 60)
	{
		hours ++;
		minutes =0;
	}
	if (hours == 24)
		seconds = minutes= hours =0;
}

ISR (INT0_vect)
{
	seconds = minutes = hours =0; // reset the clock
}
ISR (INT1_vect)
{
	//TIMSK &= ~(1<<OCIE1A); // disable Timer interrupt
	TCCR1B &= ~(1<<CS11) & ~(1<<CS10); // no clock
}
ISR (INT2_vect)
{
	TCCR1B |= (1<<CS11) |(1<<CS10);
	//TIMSK |=(1<<OCIE1A); // enable timer interrupt
}

int main (void)
{
	DDRC =0x0f; // PC0:PC3 --> OUTPUT
	DDRA= 0xff; // OUTPUT , this control of the enable/disable the 7 segments

	INT1_init ();
	INT0_init ();
	INT2_init ();
	Timer1_init();



	while (1)
	{
		Enable7Segment (5 ,seconds );
		Enable7Segment (3 ,minutes );
		Enable7Segment (1 ,hours );


	}
	return 0;
}
/*
 * ************ Calculations ******************
 *
 * F timer = FCPU /(n * (1+ TOP))
 * 1/ T timer = FCPU /(n * (1+ TOP))
 * Assume that FCPU = 1MHZ = 1^6 HZ , prescaler = 64 " we tried
 * We want T timer = 1 second to make 1 interrupt per second , then
 * 10^6 /( 64 (1+top))
 * then top = ((10^6) / 64 ) -1 = 154624
 *
 * *****************************
 */
void Timer1_init()
{
	TCCR1A =(1<<FOC1A) ; // Non PWM
	TCCR1B |= (1<<CS11) |(1<<CS10) |(1<<WGM12); // CTC Mode with prescaler 64
	TCNT1 =0; // initial value
	OCR1A =15624; // top
	TIMSK =(1<<OCIE1A); // enable timer1 module interrupt at compare match at channel A
	SREG |=(1<<7); // I BIT Enable

}
void INT0_init ()
{
	DDRD &= ~(1<<PD2); // PD2 --> input
	PORTD |= (1<<PD2); // Activate internal pull up

	GICR |= (1<<INT0); // Enable module interrupt "INT0"
	MCUCR |= (1<<ISC01); // Falling edge
	SREG |=(1<<7); // enable I bit  " Global interrupt"

}
void INT2_init ()
{
	DDRB &= ~(1<<PB2); // PB2 --> input
	PORTB |= (1<<PB2); // Activate internal pull up

	GICR |= (1<<INT2); // Enable module interrupt "INT2"
	MCUCSR &=~(1<<ISC2); // Falling --> 0
	SREG |=(1<<7); // enable I bit  " Global interrupt"


}
void INT1_init ()
{
	DDRD &= ~(1<<PD3); // PD3 --> input

	GICR |= (1<<INT1); // Enable module interrupt "INT1"
	MCUCR |= (1<<ISC10) |(1<<ISC11); // rising edge "pull down push button "
	SREG |=(1<<7); // enable I bit  " Global interrupt"
}
void Enable7Segment (int pinNum , int data )
{
	/*
	 * Enable 7 segment and disable the others
	 */
	PORTA = (1<< pinNum); // ENABLE THE PIN
	PORTC = (PORTC & 0xf0 ) |( data%10 & 0x0f) ;
	data = data /10;
	_delay_us(5);

	pinNum -- ;
	PORTA = (1<< pinNum); // ENABLE THE PIN
	PORTC = (PORTC & 0xf0 ) |( (data%10) & 0x0f) ;
	_delay_us(5);


}
