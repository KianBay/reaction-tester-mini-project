/*
* servo_timer_mchpstdio_win.c
*
* Created: 14-10-2022 19:25:24
* Author : Kianb
*/

#define F_CPU 16000000
#define SERVO_ANGLE_MAX 180
#define SERVO_ANGLE_MIN 0
#define SCORE_TIME_MAX 500
#define SCORE_TIME_MIN 150
#define SERVO_PWM_MAX 2010
#define SERVO_PWM_MIN 460
#define FOSC 16000000
#define BAUD 9600
#define MYUBBR FOSC/16/BAUD-1



#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>


void init_timers(void);
void init(void);
void reset_timer(void);
unsigned long millis(void);

volatile unsigned long millis_value;

ISR(TIMER0_COMPA_vect) {
	millis_value++;
	// PORTD ^= (1 << PD2 ); Used to verify interval using scope
}

void init_timers(void) {

	//Timer 1 for PWM control since all numbers are >8bit
	TCCR1B |= (1 << WGM13) | (1 << CS11); //PWM, Phase and Frequency Correct, TOP = ICR1, prescaler @ 8
	TCCR1A |= (1 << COM1A1); //Compare Output Mode
	TCCR0A |= (1 << WGM01); //Set CTC mode
	TCCR0B |= (1 << CS00) | (1 << CS01); //prescaler 64 for millis()
	ICR1 = 19999; //Since it counts up and down, ICR1 = ((clk/prescaler)/wave_freq)/2 - 1, ie ((16MHz/8)/50Hz)/2 - 1 = 19999
	OCR1A = 0; // SERVO OFF???
	TCNT1 = 0;


	/*Timer 0 is used to create a bootleg millis() function.
	The timer is operated in CTC mode and OCR0A is found by OCRx = ((T x F_CPU) / N) - 1,
	where T is the desired compare match value in SECONDS, F_CPU is clock speed and N is the prescaler.
	ie. OCRx = (0.001s * 16MHz) / 64 - 1 =  249
	*/

	TIMSK0 = (1 << OCIE0A); //Output Compare A MAtch Interrupt Enable
	OCR0A = 249;
	TCNT0 = 0;
}

void init(void) {
	DDRD |= (1 << DDRD5) | (1 << DDRD3) |(1 << DDRD2); //Config servo data pin and LEDs
	DDRA = 0x00; //Set all PA as input cuz idc
	PORTA |= (1 << PA0); //Pull PA0 high
	

}
// Praise the data sheet!
void USART_Init( unsigned int ubrr )
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit; 2nd stop bit is ignored for the receiver */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}
void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) ) //
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}


unsigned long millis(void) {
	unsigned long m;

	//This runs cli() before the statement, then sei() after, (allegedly) allowing us to safely read the variable manipulated by the interrupt
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		m = millis_value;
	}

	return m;
}

int map(unsigned long x, int minIN, int maxIN, int minOUT, int maxOUT) {
	// Mapping input from one domain to an output of another

	return (((x - minIN) * (maxOUT - minOUT)) / (maxIN - minIN)) +minOUT;
}

int generateSeed(void) {
	int seed;
	seed = 17; //Read ADC L values somehow

	return seed;
}

int generateRandomNumber(int seed) {
	srand(seed);
	int randomNumber = rand() % 2001; //Generate number between 0-2000
	randomNumber += 1000; //Now the number is 1000-3000, i.e. we want the random delay to be 1-3s

	return randomNumber;
}

void delay(int delayMs) {
	unsigned long timestamp = millis();

	while (millis() < (delayMs + timestamp)) {
		;
	}
}

int play_game(void) {
	unsigned long score;
	unsigned long start_millis, end_millis;
	int seed = generateSeed();
	int random_number = generateRandomNumber(seed);
	PORTD |= (1 << PD3); //Turn ON the LED to indicate game begins
	OCR1A = SERVO_PWM_MAX;
	delay(random_number);
	//reset_timer(); Dont need to reset timer since the millis() can measure 100+ years before ovf
	PORTD &= (0 << PD3); //Turn OFF the LED to indicate WE HAVE TO REACT NOW LETS GO
	start_millis = millis();
	while ((PINA & (1 << PA0)) == 1) {
		;
	}
	end_millis = millis();
	score = end_millis - start_millis;
	if (score > SCORE_TIME_MAX) {
		score = SCORE_TIME_MAX;
		} else if (score < SCORE_TIME_MIN) {
		score = SCORE_TIME_MIN;
	}
	OCR1A = scoreToPulseWidth(score);
	return score;
}

void play_games_and_transmit(int amount){
	int i;
	unsigned char data[2];
	
	if(amount<=9){
		unsigned long score[amount]; // I wonder what happens if you make a really big array of unsigned longs
		delay(200);
		for(i = 0; i < amount; i++){
			score[i] = play_game();
			delay(500);
		}

		for(i = 0; i < amount; i++){
			USART_Transmit('D');
			data[0] = (score[i] >> 8) & 0xFF;
			data[1] = (score[i] >> 0) & 0xFF;
			USART_Transmit(data[0]);
			USART_Transmit(data[1]);
			delay(100);
		}
	}
	else{
		USART_Transmit('E');
	}
	USART_Transmit('R');
	
}
void awaitSerial(unsigned char c){ // Add a timeout?
	int i = 0;
	while(i == 0){
		if (USART_Receive() == c){
			i = 1;
			
		}
		
	}
}

int convertScoreToDeg(unsigned long score) {
	return map(score, SCORE_TIME_MIN, SCORE_TIME_MAX, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
}

int setServo(int deg) {
	return map(deg, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX, SERVO_PWM_MIN, SERVO_PWM_MAX);
}
int scoreToPulseWidth(unsigned long score){
	return map(score, SCORE_TIME_MIN, SCORE_TIME_MAX, SERVO_PWM_MIN, SERVO_PWM_MAX);
}



void displayScore(unsigned long score) { //Convert this function to display the score in a more fancy way; slowly sort of sliding up to the correct angle
	int deg, servo_pwm;
	deg = convertScoreToDeg(score);
	servo_pwm = setServo(deg);
	OCR1A = servo_pwm;
}

void testServo(void){


	delay(300);
	
	while ((PINA & (1 << PA0)) == 1) {
		OCR1A = (SERVO_PWM_MIN + SERVO_PWM_MAX) / 2;
		delay(500);
		OCR1A = SERVO_PWM_MAX;
		delay(500);
		OCR1A = (SERVO_PWM_MIN + SERVO_PWM_MAX) / 2;
		delay(500);
		OCR1A = SERVO_PWM_MIN;
		delay(500);
		OCR1A = (SERVO_PWM_MIN + SERVO_PWM_MAX) / 2;
		delay(500);
		OCR1A = SERVO_PWM_MIN;
		delay(1000);
	}

}


void main(void) {
	init_timers();
	init();
	USART_Init(MYUBBR);
	unsigned long gameScore; //This was the culprit all along ...
	int angle, deg, servo_pwm;
	unsigned char c = 'g';
	int test = 0;
	sei();
	delay(2000); //Waiting on boot to make sure we are ready to play.. for now..
		if(test==1){
			testServo();
		}
	while (1) {

		delay(500);
		//Play the game
		//awaitSerial('g');
		play_games_and_transmit(4);
		delay(1500); //Little more waiting..
	}
	return;
}
