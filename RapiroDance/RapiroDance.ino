#include <Servo.h>

#define POWER_PIN 17   // サーボ電源制御ピン
#define MAXSN     12   // サーボ数

Servo servo[MAXSN];

// 各サーボ信号線をつないだデジタルピン番号
const int servoPin[MAXSN] = {
   10, // 0: Head yaw
   11, // 1: Waist yaw
    9, // 2: R Shoulder roll
    8, // 3: R Shoulder pitch ← 腕
    7, // 4: R Hand grip
   12, // 5: L Shoulder roll
   13, // 6: L Shoulder pitch ← 腕
   14, // 7: L Hand grip
    4, // 8: R Foot yaw   ← 足ヨー
    2, // 9: R Foot pitch ← 足ピッチ（必要なら使う）
   15, // 10: L Foot yaw  ← 足ヨー
   16  // 11: L Foot pitch← 足ピッチ（必要なら使う）
};

// ホームポジション（モーション0のフレーム0相当）
const int initAngle[MAXSN] = {
    90,  90,   0, 130,  90, 180,
    50,  90,  90,  90,  90,  90
};

// 微調整オフセット（必要なら変更）
int trim[MAXSN] = {
    0,0,0, 0,0, 0,0, 0,0,0, 0,0
};

void setup() {
  // サーボ電源 ON
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  delay(100);

  // 各サーボをアタッチ＆ホームポジションへ
  for (int i = 0; i < MAXSN; i++) {
    servo[i].attach(servoPin[i]);
    servo[i].write(initAngle[i] + trim[i]);
  }
  delay(500);
}

void loop() {
  // ---- 腕を肩の高さまで上げつつ、足を開く ----
  servo[3].write( 60  + trim[3]);  // 右腕ピッチ：60°
  servo[6].write(120  + trim[6]);  // 左腕ピッチ：120°
  servo[8].write( 60  + trim[8]);  // 右足ヨー：60°（外側へ開く）
  servo[10].write(120 + trim[10]); // 左足ヨー：120°（外側へ開く）
  // 足ピッチを動かしたい場合は下記も有効化
  // servo[9].write(70  + trim[9]);   // 右足ピッチ
  // servo[11].write(110 + trim[11]); // 左足ピッチ
  delay(500);

  // ---- 腕を水平に戻し、足も閉じる ----
  servo[3].write( 90 + trim[3]);
  servo[6].write( 90 + trim[6]);
  servo[8].write( 90 + trim[8]);   // 足ヨーを水平（閉じる）
  servo[10].write(90 + trim[10]);  // 足ヨーを水平（閉じる）
  // 足ピッチ戻し
  // servo[9].write( 90 + trim[9]);
  // servo[11].write(90 + trim[11]);
  delay(500);
}
