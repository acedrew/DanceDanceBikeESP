#include <Arduino.h>
#include <SmartLeds.h>
#include <driver/pcnt.h>

#define COUNTER_IN 26
#define DERIVCOUNT 10

#define PRINT_LOOP 10
#define SPEED_LOOP 100

#define LED1_DATA 13
#define LED1_CHANNEL 0
#define SHOW_DELAY 15

const int LED_COUNT = 100;
uint8_t hue = 0;

int printMillis = 0;
int speedMillis = 0;
int16_t rawCount = 0;
int16_t lastCount = 0;
int16_t lastSentCount = 0;
int16_t loopCount = 0;
int deltas[DERIVCOUNT] = {0};
uint8_t deltai = 0;
int currentDelta = 0;

int avgCount = 0;
int avg = 0;
int lastShow = 0;

pcnt_config_t pcnt_config = {
    COUNTER_IN,
    -1,
    PCNT_MODE_KEEP,
    PCNT_MODE_KEEP,
    PCNT_COUNT_INC,
    PCNT_COUNT_DIS,
    -1,
    0,
    PCNT_UNIT_0,
    PCNT_CHANNEL_0,
};


SmartLed led1( LED_WS2812, LED_COUNT, LED1_DATA, LED1_CHANNEL, DoubleBuffer );

void setup() {
    pcnt_unit_config(&pcnt_config);
    pcnt_set_filter_value(PCNT_UNIT_0, 1000);
    pcnt_filter_enable(PCNT_UNIT_0);
    Serial.begin(115200);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
    // put your setup code here, to run once:
}

void display() {
    
    hue+= 10;
    // hue++;
    for ( int i = 0; i != LED_COUNT; i++ ) {
        led1[ i ] = Hsv{ static_cast< uint8_t >( hue + 50 * i ), 255, (uint8_t)(map(avg, 0, 20, 0, 255)) };
        // led1[ i ] = Hsv{ static_cast< uint8_t >( hue + 30), 255, 255 };
    }
    led1.show();
}

int getAverage() {
    int avg = 0;
    for(int i = 0; i < DERIVCOUNT; i++) {
        avg += deltas[i];
    }
    return avg / DERIVCOUNT;
}

void loop() {
    int loopMillis = millis();
    pcnt_get_counter_value(PCNT_UNIT_0, &rawCount);
    if(rawCount != loopCount) {
        loopCount = rawCount;
        avg = getAverage();
        if(loopMillis > lastShow + SHOW_DELAY) {
            display();
            lastShow = loopMillis;

        }
    }
    if(loopMillis > speedMillis + SPEED_LOOP) {
        deltas[deltai] = rawCount - lastCount;
        deltai++;
        if(deltai == DERIVCOUNT) {
            deltai = 0;
        }
        lastCount = rawCount;
        speedMillis = loopMillis;
    }
    if(loopMillis > printMillis + PRINT_LOOP) {
        printMillis = loopMillis;
        // Serial.printf("raw: %i Average: %i\n", rawCount, avg);
        if(rawCount != lastSentCount) {
            Serial.write(1);
            Serial.write(rawCount % 256);
            lastSentCount = rawCount;
        }
        // display();
    }
    
    
    
    // put your main code here, to run repeatedly:
}