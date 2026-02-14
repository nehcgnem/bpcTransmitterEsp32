/*
 * ESP32-S3 BPC Generator
 * 根据 Wiki BPC 码表编写
 * https://en.wikipedia.org/wiki/BPC_(time_signal)
 */

#include <WiFi.h>
#include <time.h>
#include "driver/ledc.h"

// ================= 配置区域 =================
const char* ssid     = "SSID";
const char* password = "PASSWORD";

const int BPC_PIN = 21;        // GPIO 输出
const int LED_INDICATOR_PIN = 19;        // LED 指示灯

const int FREQ_BPC = 68500;   // 68.5kHz

const int LEDC_CHANNEL = 0;
const int LED_INDICATOR_CHANNEL = 1;
const int LEDC_FREQ = FREQ_BPC;

const char* ntpServer = "ntp.aliyun.com";
const long  gmtOffset_sec = 8 * 3600;
const int   daylightOffset_sec = 0;

struct tm timeinfo;
// ===========================================

void setup() {
    Serial.begin(115200);
    
    pinMode(BPC_PIN, OUTPUT);
    pinMode(LED_INDICATOR_PIN, OUTPUT);


    ledcSetup(LEDC_CHANNEL, LEDC_FREQ, 8); 
    ledcSetup(LED_INDICATOR_CHANNEL, LEDC_FREQ, 8); 


    ledcAttachPin(BPC_PIN, LEDC_CHANNEL);
    ledcAttachPin(LED_INDICATOR_PIN, LED_INDICATOR_CHANNEL);

    ledcWrite(LEDC_CHANNEL, 0); 
    ledcWrite(LED_INDICATOR_CHANNEL, 0); 

    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    while (!getLocalTime(&timeinfo)) {
        delay(1000);
    }
    Serial.println("Time Synced!");
}

// 发送四进制脉冲
// 根据 BPC 标准：
// Value 0 (00): 100ms 停 (无载波)
// Value 1 (01): 200ms 停
// Value 2 (10): 300ms 停
// Value 3 (11): 400ms 停
void send_quat(int value) {
    int off_ms = 100;
    switch (value) {
        case 0: off_ms = 100; break;
        case 1: off_ms = 200; break;
        case 2: off_ms = 300; break;
        case 3: off_ms = 400; break; 
        default: off_ms = 0; break; // for signal start
    }

    ledcWrite(LEDC_CHANNEL, 0);
    ledcWrite(LED_INDICATOR_CHANNEL, 0);

    long start = millis();
    while (millis() - start < off_ms);

    ledcWrite(LED_INDICATOR_CHANNEL, 127);
    ledcWrite(LEDC_CHANNEL, 127);

}

int get_bit(int value, int bit_pos) {
    return (value >> bit_pos) & 1;
}

int combine_bits(int msb, int lsb) {
    return (msb << 1) | lsb;
}

void loop() {
    if (!getLocalTime(&timeinfo)) {
        ledcWrite(LED_INDICATOR_CHANNEL, 0);
        ledcWrite(LEDC_CHANNEL, 0); delay(100); return;
    }

    int seconds = timeinfo.tm_sec;

    if (seconds % 20 != 0) {
        ledcWrite(LED_INDICATOR_CHANNEL, 127);
        ledcWrite(LEDC_CHANNEL, 127); 
        delay(50); 
        return;
    }

    int hour24 = timeinfo.tm_hour;
    int hour12 = hour24 % 12; 
    int is_pm  = (hour24 >= 12) ? 1 : 0; // 0=AM, 1=PM
    int min   = timeinfo.tm_min;
    int wday  = timeinfo.tm_wday;
    int bpc_wday = (wday == 0) ? 7 : wday;
    int day   = timeinfo.tm_mday;
    int month = timeinfo.tm_mon + 1;
    int year  = timeinfo.tm_year % 100;
    int frame_idx = seconds / 20; 
    int q_codes[20];
    q_codes[0] = 6;  

    if (frame_idx == 0)      q_codes[1] = 0; // 00
    else if (frame_idx == 1) q_codes[1] = 1; // 01
    else                     q_codes[1] = 2; // 10

    // [Sec 02] Unused (0, 0)
    q_codes[2] = 0;

    // [Sec 03] Hour (8, 4)
    q_codes[3] = combine_bits(get_bit(hour12, 3), get_bit(hour12, 2));

    // [Sec 04] Hour (2, 1)
    q_codes[4] = combine_bits(get_bit(hour12, 1), get_bit(hour12, 0));

    // [Sec 05] Minute (32, 16)
    q_codes[5] = combine_bits(get_bit(min, 5), get_bit(min, 4));

    // [Sec 06] Minute (8, 4)
    q_codes[6] = combine_bits(get_bit(min, 3), get_bit(min, 2));

    // [Sec 07] Minute (2, 1)
    q_codes[7] = combine_bits(get_bit(min, 1), get_bit(min, 0));

    // [Sec 08] Unused (MSb:0), Day of week (LSb:4 - Wait)
    q_codes[8] = combine_bits(0, get_bit(bpc_wday, 2));

    // [Sec 09] Day of week (2, 1)
    q_codes[9] = combine_bits(get_bit(bpc_wday, 1), get_bit(bpc_wday, 0));

    int p1_calc = 0;
    for (int i = 1; i <= 9; i++) {
        p1_calc ^= (q_codes[i] >> 1) & 1; // MSb
        p1_calc ^= (q_codes[i] >> 0) & 1; // LSb
    }

    // [Sec 10] Hour 12 (AM/PM), P1
    // MSb: 12 (AM/PM bit), LSb: P1
    q_codes[10] = combine_bits(is_pm, p1_calc);

    // [Sec 11] Unused (0), Day (16)
    q_codes[11] = combine_bits(0, get_bit(day, 4));

    // [Sec 12] Day (8, 4)
    q_codes[12] = combine_bits(get_bit(day, 3), get_bit(day, 2));

    // [Sec 13] Day (2, 1)
    q_codes[13] = combine_bits(get_bit(day, 1), get_bit(day, 0));

    // [Sec 14] Month (8, 4)
    q_codes[14] = combine_bits(get_bit(month, 3), get_bit(month, 2));

    // [Sec 15] Month (2, 1)
    q_codes[15] = combine_bits(get_bit(month, 1), get_bit(month, 0));

    // [Sec 16] Year (32, 16)
    q_codes[16] = combine_bits(get_bit(year, 5), get_bit(year, 4));

    // [Sec 17] Year (8, 4)
    q_codes[17] = combine_bits(get_bit(year, 3), get_bit(year, 2));

    // [Sec 18] Year (2, 1)
    q_codes[18] = combine_bits(get_bit(year, 1), get_bit(year, 0));

    int p2_calc = 0;
    for (int i = 11; i <= 18; i++) {
        p2_calc ^= (q_codes[i] >> 1) & 1;
        p2_calc ^= (q_codes[i] >> 0) & 1;
    }

    // [Sec 19] Year (64), P2
    // MSb: Year bit 6 (64), LSb: P2
    q_codes[19] = combine_bits(get_bit(year, 6), p2_calc);

    Serial.printf("\nSending Frame %02d:%02d:%02d (202%d-%02d-%02d)\n", 
                  hour24, min, seconds, year, month, day);

    for (int i = 0; i < 20; i++) {
        long t_start = millis();
        
        send_quat(q_codes[i]);

        // 对齐 1000ms
        long t_elapsed = millis() - t_start;
        if (t_elapsed < 1000) {
            delay(1000 - t_elapsed);
        }
    }
}
