#include <Servo.h>

<<<<<<< HEAD
#define MAXSN 12
#define POWER 17

// LEDピン定義
#define RED_PIN   6
#define GREEN_PIN 5
#define BLUE_PIN  3

Servo servo[MAXSN];

int servoPins[MAXSN] = {
  10, 11, 9, 8, 7, 12, 13, 14, 4, 2, 15, 16
};

int currentAngles[MAXSN];

// ---------------------------
// サーボを滑らかに動かす（単体）
// ---------------------------
void moveServoSmooth(int id, int targetAngle, int stepDelay = 5) {
  int current = currentAngles[id];
  int step = (targetAngle > current) ? 1 : -1;

  for (int angle = current; angle != targetAngle; angle += step) {
    servo[id].write(angle);
    delay(stepDelay);
  }

  currentAngles[id] = targetAngle;
}

// ---------------------------
// 同期で複数サーボを動かす（肩用）
// ---------------------------
void moveArmsSynced(int rollRightTarget, int rollLeftTarget, int pitchRightTarget, int pitchLeftTarget, int stepDelay = 5) {
  bool moving = true;
  while (moving) {
    moving = false;

    if (currentAngles[2] < rollRightTarget) { currentAngles[2]++; moving = true; }
    else if (currentAngles[2] > rollRightTarget) { currentAngles[2]--; moving = true; }

    if (currentAngles[5] < rollLeftTarget) { currentAngles[5]++; moving = true; }
    else if (currentAngles[5] > rollLeftTarget) { currentAngles[5]--; moving = true; }

    if (currentAngles[3] < pitchRightTarget) { currentAngles[3]++; moving = true; }
    else if (currentAngles[3] > pitchRightTarget) { currentAngles[3]--; moving = true; }

    if (currentAngles[6] < pitchLeftTarget) { currentAngles[6]++; moving = true; }
    else if (currentAngles[6] > pitchLeftTarget) { currentAngles[6]--; moving = true; }

    servo[2].write(currentAngles[2]);
    servo[5].write(currentAngles[5]);
    servo[3].write(currentAngles[3]);
    servo[6].write(currentAngles[6]);

    delay(stepDelay);
  }
}

// ---------------------------
// 初期位置（胸寄り）
// ---------------------------
void moveArmsToInitial() {
  moveArmsSynced(90, 90, 130, 40);
}

// ---------------------------
// 動作1：右肩非対称動作
// ---------------------------
void moveRightShoulderAction() {
  moveArmsSynced(150, 20, 80, 40);
}

// ---------------------------
// 動作1：左肩動作
// ---------------------------
void moveLeftShoulderAction() {
  moveArmsSynced(130, 10, 130, 80);
}

// ---------------------------
// 動作2：足踏み＋首振り（同時）
// ---------------------------
void stepFeetAndHead() {
  // 右足と同時に首を右へ
  moveServoSmooth(9, 60);   // 右脚ピッチ
  moveServoSmooth(0, 120);   // 首右
  moveServoSmooth(9, 90);   // 右脚戻す
  moveServoSmooth(0, 90);   // 首中央に戻す

  // 左足と同時に首を左へ
  moveServoSmooth(11, 60);  // 左脚ピッチ
  moveServoSmooth(0, 60);  // 首左
  moveServoSmooth(11, 90);  // 左脚戻す
  moveServoSmooth(0, 90);   // 首中央に戻す
}

// ---------------------------
// ライト制御（赤→青→緑へゆっくり）
// ---------------------------
void fadeLights() {
  static int colorPhase = 0;      // 0=赤→青, 1=青→緑, 2=緑→赤
  static int fadeValue = 0;       // 0～255
  static int fadeDirection = 1;   // 1=増加, -1=減少

  // フェード速度UP（2ステップずつ）
  if (fadeDirection == 1) fadeValue += 5;
  else fadeValue -= 5;

  if (fadeValue >= 255) { fadeDirection = -1; colorPhase = (colorPhase + 1) % 3; }
  if (fadeValue <= 0) { fadeDirection = 1; }

  int r = 0, g = 0, b = 0;
  if (colorPhase == 0) { r = 255 - fadeValue; b = fadeValue; }      // 赤→青
  else if (colorPhase == 1) { b = 255 - fadeValue; g = fadeValue; } // 青→緑
  else if (colorPhase == 2) { g = 255 - fadeValue; r = fadeValue; } // 緑→赤

  analogWrite(RED_PIN,   r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN,  b);
}

// ---------------------------
// setup
// ---------------------------
void setup() {
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);

  pinMode(RED_PIN,   OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN,  OUTPUT);

  for (int i = 0; i < MAXSN; i++) {
    servo[i].attach(servoPins[i]);
    currentAngles[i] = 90;
    servo[i].write(90);
  }

  delay(500);

  moveArmsToInitial();
  delay(2000);
}

// ---------------------------
// loop
// ---------------------------
void loop() {
  // ライト制御を常時更新
  fadeLights();

  // 動作2（足踏み＋首振り）を数秒
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    stepFeetAndHead();
    fadeLights(); // 動作中もライト更新
  }

  // 動作1（肩動作）
  moveRightShoulderAction();
  moveArmsToInitial();

  moveLeftShoulderAction();
  moveArmsToInitial();
=======
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
>>>>>>> ae56e1c5fc7e2026f07aac2f271d43eb5b5fb44c
}
