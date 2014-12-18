#include "lcd.h"

unsigned long pcount=0;    // used in interrupt to count PER_COUNTS
unsigned char calc_bit=0;
unsigned int RPM_Value=0;
unsigned int VOLT=0;
unsigned char settings_on=0;
unsigned char trip_status=0;
unsigned char prev_trip_status=0;

unsigned int HIGH_volt=350;
unsigned char Vo_HT_delay=0;
unsigned int LOW_volt=250;
unsigned char Vo_LT_delay=0;

unsigned int HIGH_freq=60;
unsigned char Fq_HT_delay=0;
unsigned int LOW_freq=0;
unsigned char Fq_LT_delay=0;

#define TRIP1 LATCbits.LC1
#define TRIP2 LATCbits.LC2
#define BUZZER LATCbits.LC3

#define MODE  PORTCbits.RC6
#define UP    PORTCbits.RC5
#define RIGHT PORTCbits.RC7
#define ENTER PORTCbits.RC4

#define MODE_VAL 1
#define UP_VAL 2
#define RIGHT_VAL 3
#define ENTER_VAL 4
#define FALSE 0


void lcd_First_stage()
{
    LCD_clear();
    LCD_goto(1,0);
    __delay_ms(1);
    lcd_puts ("Current Values");
    __delay_ms(1);
    LCD_goto(2,5);
    __delay_ms(1);
    lcd_puts ("Hz");
    __delay_ms(1);
    LCD_goto(2,13);
    __delay_ms(1);
    lcd_puts ("V");
}

void initADC()
{
   //We use default value for +/- Vref

   //VCFG0=0,VCFG1=0
   //That means +Vref = Vdd (5v) and -Vref=GEN

   //Port Configuration
   //We also use default value here too
   //All ANx channels are Analog

   /*
      ADCON2

      *ADC Result Right Justified.
      *Acquisition Time = 2TAD
      *Conversion Clock = 32 Tosc
   */

   ADCON2=0b10001010;


    ADCON1bits.VCFG1=0;        //VCFG0=0,VCFG1=0
    ADCON1bits.VCFG0=0;

    ADCON1bits.PCFG0 = 0;          // These 4 settings below determines the analog or digital input
    ADCON1bits.PCFG1 = 1;          // In our case we are making all the pins digital
    ADCON1bits.PCFG2 = 1;         // by setting them as 1111
    ADCON1bits.PCFG3 = 1;         // Check with the datasheet for a nice desc of these bits and config.

}


void select_adc()
{


   ADCON0=0x00;
    CHS3=CHS2=CHS1=CHS0=0;
 

   ADON=1;  //switch on the adc module

   GODONE=1;  //Start conversion

   while(GODONE); //wait for the conversion to finish

   ADON=0;  //switch off adc


    VOLT=(unsigned int)(50*(ADRES)/1023)*10;
}



unsigned char get_key()
{

    if(MODE==0)
    {
        while(MODE==0);
        return MODE_VAL;
    }

    else if(UP==0)
    {
        //while(UP==1);

        __delay_ms(10);
        __delay_ms(10);
        __delay_ms(10);
        __delay_ms(10);
        __delay_ms(10);
        __delay_ms(10);

        return UP_VAL;
    }

    else if(RIGHT==0)
    {
        while(RIGHT==0);
        return RIGHT_VAL;
    }

    else if(ENTER==0)       //check if key4 is pressed
    {
        while(ENTER==1);   //wait for release
        return ENTER_VAL;     //return key number
    }

    return FALSE;                   //return false if no key pressed
}


void check_TRIPPing()
{

	


    if ( VOLT > HIGH_volt )
    {
        trip_status = 1;


    }


    else if ( VOLT < LOW_volt )
    {
        trip_status = 2;


    }

    else if ( RPM_Value > HIGH_freq )
    {
        trip_status = 3;

    }


    else if ( RPM_Value < LOW_freq )
    {
        trip_status = 4;

    }


    else
    {

        trip_status = 0;

    }

    if (trip_status == 0 )
    {


    if ( VOLT < 1.1 * LOW_volt || VOLT > 0.9 * HIGH_volt ) // 10% warning
    {
        BUZZER = 1;

    }

	else  BUZZER = 0;

        if (trip_status != prev_trip_status)
        {
            TRIP1 =0;
            TRIP2 =0;
            BUZZER=0;

            lcd_First_stage();


        }

    }


    if (trip_status > 0 )
    {

        if (trip_status != prev_trip_status)
        {
            TRIP1 =1;
            TRIP2 =1;
            BUZZER=1;

            LCD_clear();
            __delay_ms(1);
            LCD_goto(1,4);
            lcd_puts ("WARNING !!");
            LCD_goto(2,0);
            __delay_ms(1);

            if (trip_status == 1)
                lcd_puts ("Very high voltage");
            else if (trip_status == 2)
                lcd_puts ("Very low voltage");
            else if (trip_status == 3)
                lcd_puts ("Very high frequency");
            else if (trip_status == 4)
                lcd_puts ("Very low frequency");



        }

    }


    prev_trip_status=trip_status;

}



void main()
{


    TRISC=0XF0;
    TRISB=0x00;
    TRISA=0x0FF;


    initLCD();
    lcd_First_stage();
    initADC();


    unsigned char key=0;
    unsigned char men_cnt=0;
    unsigned char mode_pressed=0;



    while(1)
    {




        if ((key=get_key()) !=FALSE)

        {
            settings_on =1;
            if (key == MODE_VAL)
            {
                mode_pressed=1;
                men_cnt++;
                LCD_clear();
                LCD_goto(1,0);
                lcd_puts ("Set the");
                LCD_goto(2,10);
                lcd_puts ("[");
                LCD_Write ( 48 + men_cnt,1);
                lcd_puts ("/");
                LCD_Write ('9',1);
                lcd_puts ("]");

                if (men_cnt == 1)
                {

                    LCD_goto(1,8);
                    lcd_puts ("High V");
                    LCD_goto(2,0);
                    LCD_num (HIGH_volt);


                }

                if (men_cnt == 2)
                {
                    LCD_goto(1,8);
                    lcd_puts ("H Time Del");
                    LCD_goto(2,0);
                    LCD_num (Vo_HT_delay);




                }

                if (men_cnt == 3)
                {

                    LCD_goto(1,8);
                    lcd_puts ("Low V");
                    LCD_goto(2,0);
                    LCD_num (LOW_volt);

                }

                if (men_cnt == 4)
                {

                    LCD_goto(1,8);
                    lcd_puts ("L Time Del");
                    LCD_goto(2,0);
                    LCD_num (Vo_LT_delay);

                }


                if (men_cnt == 5)
                {

                    LCD_goto(1,8);
                    lcd_puts ("High Frequency");
                    LCD_goto(2,0);
                    LCD_num (HIGH_freq);

                }

                if (men_cnt == 6)
                {
                    LCD_goto(1,8);
                    lcd_puts ("H Time Del");
                    LCD_goto(2,0);
                    LCD_num (Fq_HT_delay);


                }

                if (men_cnt == 7)
                {

                    LCD_goto(1,8);
                    lcd_puts ("Low Frequency");
                    LCD_goto(2,0);
                    LCD_num (LOW_freq);

                }

                if (men_cnt == 8)
                {
                    LCD_goto(1,8);
                    lcd_puts ("L Time Del");
                    LCD_goto(2,0);
                    LCD_num (Fq_LT_delay);

                }


                if (men_cnt == 9)
                {
                    LCD_goto(1,0);
                    lcd_puts ("See all the values");
                    men_cnt=0;

                }



            }


            else if (mode_pressed==1 && key == ENTER_VAL)
            {
                men_cnt=0;
                lcd_First_stage();
                settings_on =0;

            }



            else if (mode_pressed && (key == UP_VAL || key == RIGHT_VAL) )
            {
                if (men_cnt == 1)
                {
                    if (key == UP_VAL ) HIGH_volt++;
                    if (key == RIGHT_VAL) HIGH_volt--;

                    LCD_goto(2,0);
                    LCD_num (HIGH_volt);

                }

                if (men_cnt == 2)
                {
                    if (key == UP_VAL ) Vo_HT_delay++;
                    if (key == RIGHT_VAL) Vo_HT_delay--;
                    LCD_goto(2,0);
                    LCD_num (Vo_HT_delay);


                }

                if (men_cnt == 3)
                {
                    if (key == UP_VAL ) LOW_volt++;
                    if (key == RIGHT_VAL) LOW_volt--;
                    LCD_goto(2,0);
                    LCD_num (LOW_volt);

                }

                if (men_cnt == 4)
                {
                    if (key == UP_VAL ) Vo_LT_delay++;
                    if (key == RIGHT_VAL) Vo_LT_delay--;
                    LCD_goto(2,0);
                    LCD_num (Vo_LT_delay);

                }


                if (men_cnt == 5)
                {
                    if (key == UP_VAL ) HIGH_freq++;
                    if (key == RIGHT_VAL) HIGH_freq--;

                    LCD_goto(2,0);
                    LCD_num (HIGH_freq);

                }

                if (men_cnt == 6)
                {
                    if (key == UP_VAL ) Fq_HT_delay++;
                    if (key == RIGHT_VAL) Fq_HT_delay--;
                    LCD_goto(2,0);
                    LCD_num (Fq_HT_delay);

                }

                if (men_cnt == 7)
                {
                    if (key == UP_VAL ) LOW_freq++;
                    if (key == RIGHT_VAL) LOW_freq--;

                    LCD_goto(2,0);
                    LCD_num (LOW_freq);

                }

                if (men_cnt == 8)
                {
                    if (key == UP_VAL ) Fq_LT_delay++;
                    if (key == RIGHT_VAL) Fq_LT_delay--;
                    LCD_goto(2,0);
                    LCD_num (Fq_LT_delay);

                }
            }



        }





 
            select_adc();

            if (settings_on ==0)
                check_TRIPPing();

            if (settings_on ==0 && trip_status ==0)
            {
                LCD_goto(2,0);
                LCD_num (RPM_Value);
                LCD_goto(2,8);
                LCD_num (VOLT);

            }

            TMR1L = 0;
            TMR1H = 0;
            calc_bit=0;
            TMR1ON = 1;

       

    }




}
