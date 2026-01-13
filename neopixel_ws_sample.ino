//  Copyright (c) 2026 Hasebe Masahiko.
//  Released under the MIT license
//  https://opensource.org/licenses/mit-license.php
//
//  Board: Waveshare RP2040 Zero
//  Port: UF2_Board
//  書き込み方法: 左BootSW押しながら右ResetSWを押す
//  (USB Stack: Pico SDK(TinyUSBは選ばないこと))

// Arduino の自動プロトタイプ生成対策として前方宣言
struct Color;

#include <Arduino.h>              // Arduino API
#include <pico/stdlib.h>          // IRAM_ATTR, repeating_timer
#include "RPi_Pico_TimerInterrupt.h"
#include "sk6812.h"

// --- 定数
constexpr uint8_t  NUM_LEDS   = 6;
constexpr uint8_t  DATA_PIN   = 26;
constexpr uint32_t SPEED      = 10000;   // 10 ms

// --- グローバル
SK6812 sk(NUM_LEDS, DATA_PIN);
volatile uint32_t counter = 0;                // ISR で変更するので volatile
RPI_PICO_Timer ITimer1(1);                    // ハードウェアタイマー #1

struct Color {
  int red;
  int green;
  int blue;
  int white;
};

// LED パターン生成(初期データ)
// counter10msec: カウンタ(0.01sec=10msec)
// led_num: LED 番号
// clr: 出力カラー
void generate_led_pattern(uint32_t counter10msec, size_t led_num, Color& clr) {
  int counter = counter10msec / 50; // 0.5sec 単位に変換
  int period = counter % 4; // 4 段階の周期
  if (period == 0) {
    clr.red = 100;
    clr.green = 0;
    clr.blue = 0;
    clr.white = 0;
  } else if (period == 1) {
    clr.red = 0;
    clr.green = 100;
    clr.blue = 0;
    clr.white = 0;  
  } else if (period == 2) {
    clr.red = 0;
    clr.green = 0;
    clr.blue = 100;
    clr.white = 0;  
  } else if (period == 3) {
    clr.red = 0;
    clr.green = 0;
    clr.blue = 0;
    clr.white = 100;  
  }
}


// LED パターン生成(色が流れる)
void generate_led_pattern2(uint32_t counter10msec, size_t led_num, Color& clr) {
  int phase = (counter10msec + led_num * 10) % 300;
  if (phase < 100) {
    clr.red   = 100;
    clr.green = 0;
    clr.blue  = 0;
    clr.white = 0;
  } else if (phase < 200) {
    clr.red   = 0;
    clr.green = 100;
    clr.blue  = 0;
    clr.white = 0;
  } else if (phase < 300) {
    clr.red   = 0;
    clr.green = 0;
    clr.blue  = 100;
    clr.white = 0;
  } else {
    clr.red   = 0;
    clr.green = 0;
    clr.blue  = 0;
    clr.white = 100;
  }
}


// LED パターン生成（グラデーション）
void generate_led_pattern3(uint32_t counter10msec, size_t led_num, Color& clr) {
  int phase = (counter10msec + led_num * 10) % 300;
  if (phase < 100) {
    clr.red   = phase;
    clr.green = 0;
    clr.blue  = 100 - phase;
    clr.white = 0;
  } else if (phase < 200) {
    clr.red   = 200 - phase;
    clr.green = phase - 100;
    clr.blue  = 0;
    clr.white = 0;
  } else {
    clr.red   = 0;
    clr.green = 300 - phase;
    clr.blue  = phase - 200;
    clr.white = 0;
  }
}




// --- 割り込みハンドラ
//     repeating_timer* 引数で呼び出される
bool TimerHandler(struct repeating_timer* /*rt*/) {
  counter = counter + 1;
  return true;
}

// --- setup / loop
void setup() {
  sk.begin();
  delay(500);

  // SPEED ms ごとに TimerHandler を呼び出し
  if (!ITimer1.attachInterruptInterval(SPEED, TimerHandler)) {
    // 失敗したら止める
    while (true) {}
  }
}

void loop() {
  sk.clear();
  for (int i = 0; i < NUM_LEDS; ++i) {
    Color clr;
    generate_led_pattern(counter, i, clr);
    limit_brightness(clr);
    sk.setPixelColor(i, 
      static_cast<uint8_t>(clr.red), 
      static_cast<uint8_t>(clr.green),
      static_cast<uint8_t>(clr.blue),
      static_cast<uint8_t>(clr.white)
    );
  }
  sk.show();
  delay(1);
}

void limit_brightness(Color& clr) {
  const int max_brightness = 255; // 最大輝度の閾値
  clr.red = (clr.red <= 100)?(clr.red * max_brightness) / 100: max_brightness;
  clr.green = (clr.green <= 100)?(clr.green * max_brightness) / 100: max_brightness;
  clr.blue = (clr.blue <= 100)?(clr.blue * max_brightness) / 100: max_brightness;
  clr.white = (clr.white <= 100)?(clr.white * max_brightness) / 100: max_brightness;
}
