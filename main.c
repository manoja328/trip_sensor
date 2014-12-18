#include "lcd.h"
#define TRUE 1
#define FALSE 0

unsigned int task0_counter=0;
unsigned int task1_counter=0;
unsigned int task2_counter=0;

#define TASK0_COUNTER_MAX 1
#define TASK1_COUNTER_MAX 1
#define TASK2_COUNTER_MAX 1000


volatile unsigned char task0_enable=TRUE;
volatile unsigned char task2_enable=TRUE;
volatile unsigned char task2_go=FALSE;


volatile unsigned int VOLT=0;
volatile unsigned int RPM_Value=0;
unsigned char settings_on=0;
unsigned char trip_status=0;
unsigned char prev_trip_status=0;

unsigned int HIGH_volt=350;
unsigned char Vo_HT_delay=0;
unsigned int LOW_volt=250;
unsigned char Vo_LT_delay=0;

unsigned int HIGH_freq=55;
unsigned char Fq_HT_delay=0;
unsigned int LOW_freq=45;
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


void setup_multitasking(void)
{

    TMR1CS=0;
    T1CKPS0=0;
    T1CKPS1=0;
#define  TICKS_BETWEEN_INTERRUPTS      1000
#define  INTERRUPT_OVERHEAD            19
#define TMR1RESET (0x0000-(TICKS_BETWEEN_INTERRUPTS-INTERRUPT_OVERHEAD))
#define TMR1RESET_HIGH TMR1RESET >> 8
#define TMR1RESET_LOW TMR1RESET & 0xFF
    TMR1ON=0;
    TMR1H=TMR1RESET_HIGH;
    TMR1L=TMR1RESET_LOW;
    TMR1ON=1;
    TMR1IF=0;
    TMR1IE=1;
    PEIE=1;
    GIE=1;
}

void interrupt isr(void)
{

    if (INT0IF && INT0IE)
    {
        RPM_Value++;
        INT0IF = 0;

    }

    if (TMR1IF && TMR1IE)
    {

        TMR1IF=0;
        TMR1ON=0;
        TMR1H=TMR1RESET_HIGH;
        TMR1L=TMR1RESET_LOW;
        TMR1ON=1;
        task0_counter++;
        if (task0_counter>=TASK0_COUNTER_MAX)
        {
            task0_counter=0;
            if (task0_enable==TRUE)
            {

    ADON=1;
    GODONE=1;
    while(GODONE);
    ADON=0;
    VOLT=(unsigned int)(50*(ADRES)/1023)*10;


            }
        }

        task2_counter++;
        if (task2_counter>=TASK2_COUNTER_MAX)
        {
            task2_counter=0;
            if (task2_enable==TRUE)
            {

                task2_go=TRUE;
            }
        }
    }
}


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
    ADCON2=0b10001010;
    ADCON1bits.VCFG1=0;
    ADCON1bits.VCFG0=0;
    ADCON1bits.PCFG0 = 0;
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG3 = 1;
    ADCON0=0x00;
    CHS3=CHS2=CHS1=CHS0=0;

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

else BUZZER = 0;

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
    TRISB=0X01;;
    TRISA=0x0FF;

    initLCD();
    lcd_First_stage();
    initADC();


    unsigned char key=0;
    unsigned char men_cnt=0;
    unsigned char mode_pressed=0;


    INT0IE = 1;
    INTEDG0 = 1;
    setup_multitasking();


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




            if (settings_on ==0 && task2_go==TRUE){
                check_TRIPPing();

if (trip_status !=0){
task2_go=FALSE;
RPM_Value=0;
}

}



        if (task2_go==TRUE && settings_on ==0 && trip_status ==0)
        {


            	task2_go=FALSE;
                LCD_goto(2,0);
                LCD_num (RPM_Value);
                LCD_goto(2,8);
                LCD_num (VOLT);            	
				RPM_Value=0;
        }



    }
}
