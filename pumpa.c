#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <lcd.h>
#include <string.h>

//adrese registara
#define SEC 0x02
#define MIN 0x03
#define HOU 0x04

#define changeHexToInt(hex) ((((hex)>>4) *10 ) + ((hex)%16))
const char RTC = 0x51; //hardverska adresa RTC-a

//PINOVI koje koristi LCD
const int RS = 3;
const int EN = 14;
const int D0 = 4;
const int D1 = 12;
const int D2 = 13;
const int D3 = 6;
char SENSOR = 29;
char BUTTON = 24;
char PUMP_SIGNAL =0;

int fd;
int hours, minutes, seconds;

char txt_seconds[4];
char txt_minutes[4];
char txt_hours[4];

int main()
{	
	if(wiringPiSetup() == -1) exit(1); //inicijalizacija WiringPi
	pinMode (SENSOR, INPUT); //PIN senzor je u INPUT modu
	pinMode (BUTTON, INPUT); //PIN BUTTON je u INPUT modu
	pinMode(PUMP_SIGNAL,OUTPUT); // definisanje PUMP_SIGNAL kao izlaz
	pullUpDnControl(BUTTON, PUD_UP); //dugme ce da radi u Pull up rezimu

	int sensor_value = 0;
	int button_value = 1;
	int pump_signal_value=0;
	int lcd_h;
	int seconds_tmp = 1;
	int detc_hours;
	int detc_minutes;
	int detc_seconds;
	int enable = 0;
	int enable1 =0;
	FILE *fp;

	lcd_h = lcdInit(2, 16, 4, RS, EN, D0, D1, D2, D3, D0, D1, D2, D3); //Inicijalizacija LCD-a 2 - broj kolona,16-broj redova -4 bitni interfejs, pinovi koje koristi lcd u 4bitnom interfejsu

	fd = wiringPiI2CSetup(RTC); //inicijalizacija RTC i postavljanje pocetnih vrednosti u Registre za SATE MINUTE i SEKUNDE
	wiringPiI2CWriteReg8(fd, HOU, 0x23); 
	wiringPiI2CWriteReg8(fd, MIN, 0x59);
	wiringPiI2CWriteReg8(fd, SEC, 0x00);

	while(1)
	{		
			
			sensor_value = digitalRead(SENSOR); 
			//citamo vrednost sa senzora 1 je ako je detektovao nesto , 0 ako nije 

			if(sensor_value == 1)
			{
				enable1=1;
				if(enable !=enable1){
					fp=fopen("/home/pi/Desktop/pumpa.txt","w+");
					fprintf(fp,"land is dry : %d %d %d \n", detc_hours, detc_minutes, detc_seconds);
					
					fclose(fp);
					enable=enable1;
					}
				//enable = 1;
				detc_hours = hours;
				detc_minutes = minutes;
				detc_seconds = seconds;
				 digitalWrite(PUMP_SIGNAL, LOW);


				sprintf(txt_hours, "%d", detc_hours); 
				sprintf(txt_minutes, "%d", detc_minutes);
				sprintf(txt_seconds, "%d", detc_seconds);
				//mozda stavi ovde jos jedan lcdClear(lcd_h) 
				lcdPosition(lcd_h, 0, 0);  
				lcdPrintf(lcd_h, "LAND IS DRY!"); 
				lcdPosition(lcd_h, 0, 1);
				lcdPrintf(lcd_h, "Time: ");
				lcdPosition(lcd_h, 6, 1);
				lcdPrintf(lcd_h,txt_hours);
				lcdPosition(lcd_h, 8, 1);
				lcdPrintf(lcd_h,":");
				lcdPosition(lcd_h, 9, 1);
				lcdPrintf(lcd_h,txt_minutes);
				lcdPosition(lcd_h, 11, 1);
				lcdPrintf(lcd_h,":");
				lcdPosition(lcd_h, 12, 1);
				lcdPrintf(lcd_h,txt_seconds);
				printf("land is dry : %d %d %d \n", detc_hours, detc_minutes, detc_seconds);
				  								//	printf("Vrednost na senzoru: %d ",sensor_value);


				button_value = digitalRead(BUTTON);  //citamo vrednost sa dugmeta (ili 1 ili 0)
				if(button_value == 0)  //ako je dugme stisnuto
				{
					enable = 0;
					lcdClear(lcd_h); //brisemo sadrzaj sa LCD-a
				}
				//printf("Vrednost na senzoru: %d ",sensor_value);
			}else
				{
					enable1=0;
				if(enable !=enable1){
					
					enable=enable1;
					}
								//	printf("Vrednost na senzoru: %d ",sensor_value);
									    digitalWrite(PUMP_SIGNAL, HIGH);

					hours = changeHexToInt(wiringPiI2CReadReg8(fd, HOU)& 0x3f);   // Citamo vreme iz registara za sata, prebacujemo u int i smestamo u hours , maska 0x3f jer nekoristi 7 i 6 bit pa 0011 1111 je 0x3f u hexu
					minutes = changeHexToInt(wiringPiI2CReadReg8(fd, MIN)& 0x7f); //isto ko i gore samo druga maska za MIN
					seconds = changeHexToInt(wiringPiI2CReadReg8(fd, SEC)& 0x7f);

					sprintf(txt_hours,"%d", hours);  //prebacujemo integere u stringove 
					sprintf(txt_minutes,"%d", minutes);
					sprintf(txt_seconds,"%d", seconds);

					printf("Vreme HH:MM:SS %d %d %d \n ", hours, minutes, seconds);
					lcdPosition(lcd_h, 0, 0); //namestamo kursor LCDa na 0,0 (x,y) kordinate
					lcdPrintf(lcd_h,"LAND IS WET"); //ispisujemo na toj poziciji dati string
					lcdPosition(lcd_h, 0, 1);
					lcdPrintf(lcd_h,"Time:");
					lcdPosition(lcd_h, 6, 1);
					lcdPrintf(lcd_h,txt_hours);
					lcdPosition(lcd_h, 8, 1);
					lcdPrintf(lcd_h,":");
					lcdPosition(lcd_h, 9, 1);
					lcdPrintf(lcd_h,txt_minutes);
					lcdPosition(lcd_h, 11, 1);
					lcdPrintf(lcd_h,":");
					lcdPosition(lcd_h, 12, 1);
					lcdPrintf(lcd_h,txt_seconds);
									printf("land is wet : %d %d %d \n", detc_hours, detc_minutes, detc_seconds);

				}
		
	}
	return 0;
}

