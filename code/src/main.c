// These are wrong, just taken from the ATTiny85 video
#define PORTD *((volatile unsigned char *) 0x32)
#define DDRD *((volatile unsigned char *) 0x31)

#define SET_PIN(PORT, PIN) (PORT) |= (1 << (PIN))
#define CLEAR_PIN(PORT, PIN) (PORT) &= ~(1 << (PIN))

int main()
{
    SET_PIN(DDRD, 0);

    while (1)
    {
        SET_PIN(PORTD, 0);
        for (long i=0; i<1000000; i++) { SET_PIN(PORTD, 0); }
        CLEAR_PIN(PORTD, 0);
        for (long i=0; i<1000000; i++) { CLEAR_PIN(PORTD, 0);}
    }
}
