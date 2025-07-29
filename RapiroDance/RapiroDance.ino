#include <Servo.h>

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
}
