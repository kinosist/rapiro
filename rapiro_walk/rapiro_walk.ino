// by ShotaIshiwatari is licensed under the Creative Commons - Public Domain Dedication license.

#include <Servo.h>

#define SHIFT   7      // 角度シフトビット数
#define TIME    15     // フレーム内の時間インデックス
#define MAXSN   12     // サーボ数
#define MAXFN   8      // フレーム数
#define POWER   17     // サーボ電源制御ピン

Servo servo[MAXSN];

// 微調整（必要に応じて調整してください）
int trim[MAXSN] = {
  0,  // Head yaw
  0,  // Waist yaw
  0,  // R Sholder roll
  0,  // R Sholder pitch
  0,  // R Hand grip
  0,  // L Sholder roll
  0,  // L Sholder pitch
  0,  // L Hand grip
  0,  // R Foot yaw
  0,  // R Foot pitch
  0,  // L Foot yaw
  0   // L Foot pitch
};

// ランタイム用変数
int     nowAngle[MAXSN];
int     targetAngle[MAXSN];
int     deltaAngle[MAXSN];
uint8_t bufferAngle[MAXSN];

uint8_t frameNumber   = 0;
uint8_t bufferTime    = 0;
uint8_t motionNumber  = 1;   // 1 = 前進モーション
char    mode          = 'M';

double  startTime     = 0;
double  endTime       = 0;

// motion[motionNumber][frame][0..11]=サーボ角度,
//                       [12..14]=LED R,G,B,
//                       [15]=TIME(0.1秒単位)
uint8_t motion[2][MAXFN][16] = {
  // 0: 停止モーション（unused）
  {
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,255, 10},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0,130, 90,180, 50, 90, 90, 90, 90, 90,   0,  0,  0,  0}
  },
  // 1: 前進モーション
  {
    { 90,90,  0, 90, 90,180, 90, 90, 80,110, 80,120,   0,  0,  0,  5},
    { 90,90,  0, 90, 90,180, 90, 90, 70, 90, 70, 90,   0,  0,255,  5},
    { 90,90,  0, 90, 90,180, 90, 90, 70, 70, 70, 80,   0,  0,255,  5},
    { 90,90,  0, 90, 90,180, 90, 90,100, 60,100, 70,   0,  0,  0,  5},
    { 90,90,  0, 90, 90,180, 90, 90,110, 90,110, 90,   0,  0,255,  5},
    { 90,90,  0, 90, 90,180, 90, 90,110,100,110,110,   0,  0,255,  5},
    { 90,90,  0, 90, 90,180, 90, 90, 90, 90, 90, 90,   0,  0,  0,  0},
    { 90,90,  0, 90, 90,180, 90, 90, 90, 90, 90, 90,   0,  0,  0,  0}
  }
};

void setup() {
  // サーボピンアタッチ
  servo[0].attach(10);  // Head yaw
  servo[1].attach(11);  // Waist yaw
  servo[2].attach( 9);  // R Sholder roll
  servo[3].attach( 8);  // R Sholder pitch
  servo[4].attach( 7);  // R Hand grip
  servo[5].attach(12);  // L Sholder roll
  servo[6].attach(13);  // L Sholder pitch
  servo[7].attach(14);  // L Hand grip
  servo[8].attach( 4);  // R Foot yaw
  servo[9].attach( 2);  // R Foot pitch
  servo[10].attach(15); // L Foot yaw
  servo[11].attach(16); // L Foot pitch

  // 電源ON
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);

  // 初期角度を前進モーションの第0フレームに設定
  for(int i = 0; i < MAXSN; i++){
    targetAngle[i] = motion[motionNumber][0][i] << SHIFT;
    nowAngle[i]    = targetAngle[i];
    servo[i].write((nowAngle[i] >> SHIFT) + trim[i]);
  }

  // モーション開始準備
  startTime = millis();
  endTime   = 0;
}

void loop() {
  // モーション中か？
  if (endTime > millis()) {
    int remaining = (endTime - millis()) / 10;
    for(int i = 0; i < MAXSN; i++){
      nowAngle[i] = targetAngle[i] - deltaAngle[i] * remaining;
      servo[i].write((nowAngle[i] >> SHIFT) + trim[i]);
    }
  }
  // 次フレームへ
  else {
    nextFrame();
  }
}

// フレーム切り替え
void nextFrame() {
  frameNumber = (frameNumber + 1) % MAXFN;
  for(int i = 0; i < MAXSN; i++){
    bufferAngle[i] = motion[motionNumber][frameNumber][i];
  }
  bufferTime = motion[motionNumber][frameNumber][TIME];
  nextPose();
}

// ポーズ（サーボ角度を補間設定）
void nextPose() {
  if (bufferTime > 0) {
    for(int i = 0; i < MAXSN; i++){
      targetAngle[i] = bufferAngle[i] << SHIFT;
      deltaAngle[i]  = ((targetAngle[i] - nowAngle[i]) / (bufferTime * 10));
    }
  } else {
    for(int i = 0; i < MAXSN; i++){
      deltaAngle[i] = 0;
    }
  }
  startTime = millis();
  endTime   = startTime + (bufferTime * 100);
  bufferTime = 0;
}
