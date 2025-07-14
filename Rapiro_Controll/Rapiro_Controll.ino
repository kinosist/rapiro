//動かし方
1.右上の虫眼鏡マーク(SerialMonitor)を開く
2.ボーレートを57600 baudに合わせる
3.コマンドを打つと動く
例)#HY130T010
#<部位(今回はHeadYawなのでHY)><任意の角度(初期90°)>T<動きにかける時間３桁(0.1秒単位)>

#define SHIFT 7

#define R 0           // 赤色LEDのインデックス
#define G 1           // 緑色LEDのインデックス
#define B 2           // 青色LEDのインデックス

#define MAXSN 12      // サーボモーターの最大数 (0から11まで)
#define POWER 17      // サーボモーターの電源供給を制御するピン番号
#define ERR -1        // エラーを示すための戻り値

int i = 0; // ループカウンタ
Servo servo[MAXSN];
uint8_t eyes[3] = { 0, 0, 0}; //{Rピン, Gピン, Bピン}

int trim[MAXSN] = { 0,  // servo[0]: Head yaw (頭の左右回転 HY)
                    0,  // servo[1]: Waist yaw (腰の左右回転 WY)
                    0,  // servo[2]: R Sholder roll (右肩のロール軸 - 腕を上下 RSR)
                    0,  // servo[3]: R Sholder pitch (右肩のピッチ軸 - 腕を前後 RSP)
                    0,  // servo[4]: R Hand grip (右手グリップ - 開閉 RHG)
                    0,  // servo[5]: L Sholder roll (左肩のロール軸 - 腕を上下 LSR)
                    0,  // servo[6]: L Sholder pitch (左肩のピッチ軸 - 腕を前後 LSP)
                    0,  // servo[7]: L Hand grip (左手グリップ - 開閉 LHG)
                    0,  // servo[8]: R Foot yaw (右足のヨー軸 - 足首の左右 RFY)
                    0,  // servo[9]: R Foot pitch (右足のピッチ軸 - 足首の前後 RFP)
                    0,  // servo[10]: L Foot yaw (左足のヨー軸 - 足首の左右 LFY)
                    0}; // servo[11]: L Foot pitch (左足のピッチ軸 - 足首の前後 LFP)

// サーボモーターの現在の角度、目標角度、そして10ミリ秒あたりの変化量を格納する配列。
// SHIFT値で拡大された内部表現の数値が格納されます。
int nowAngle[MAXSN] =        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 現在の角度
int targetAngle[MAXSN] =     { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 目標角度
int deltaAngle[MAXSN] =      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 各ステップでの角度変化量

// LEDの現在の明るさ、目標明るさ、そして10ミリ秒あたりの変化量を格納する配列。
// SHIFT値で拡大された内部表現の数値が格納されます。
int nowBright[3] =        { 0, 0, 0};  // 現在の明るさ
int targetBright[3] =     { 0, 0, 0};  // 目標明るさ
int deltaBright[3] =      { 0, 0, 0};  // 各ステップでの明るさ変化量

double startTime =    0;               // 動きの開始時刻 (millis()で取得したミリ秒)
double endTime =      0;               // 動きの終了時刻 (ミリ秒)
int remainingTime =  0;                // 動きの残り時間 (ミリ秒)

// プロトタイプ宣言 (関数が定義される前に使用されるため)
void moveSingleServo(int servoIndex, int targetAngleValue, int durationTime);
int readOneDigit();
int readThreeDigit(int maximum);
int printThreeDigit(int buf);
// 新しい個別制御関数のプロトタイプ宣言
void moveHeadYaw(int angle, int time);
void moveWaistYaw(int angle, int time);
void moveRSholderRoll(int angle, int time);
void moveRSholderPitch(int angle, int time);
void moveRHandGrip(int angle, int time);
void moveLSholderRoll(int angle, int time);
void moveLSholderPitch(int angle, int time);
void moveLHandGrip(int angle, int time);
void moveRFootYaw(int angle, int time);
void moveRFootPitch(int angle, int time);
void moveLFootYaw(int angle, int time);
void moveLFootPitch(int angle, int time);


// --- セットアップ関数 ---
// Arduinoが起動した際に一度だけ実行されます
void setup()  {
  // 各サーボをArduinoの指定ピンにアタッチします。
  // サーボインデックスと対応するピン番号、そして部位名
  servo[0].attach(10);    // Head yaw
  servo[1].attach(11);    // Waist yaw
  servo[2].attach(9);     // R Sholder roll
  servo[3].attach(8);     // R Sholder pitch
  servo[4].attach(7);     // R Hand grip
  servo[5].attach(12);    // L Sholder roll
  servo[6].attach(13);    // L Sholder pitch
  servo[7].attach(14);    // L Hand grip
  servo[8].attach(4);     // R Foot yaw
  servo[9].attach(2);     // R Foot pitch
  servo[10].attach(15);   // L Foot yaw
  servo[11].attach(16);   // L Foot pitch
  
  // 目のRGB LEDのピン番号を設定します。
  eyes[R] = 6;            // 赤色LEDピン
  eyes[G] = 5;            // 緑色LEDピン
  eyes[B] = 3;            // 青色LEDピン
  
  // すべてのサーボとLEDを初期状態（中心、消灯）に設定します。
  for( i = 0; i < MAXSN; i++) {
    // 90度（中心）を初期目標として設定し、内部表現に変換
    targetAngle[i] = 90 << SHIFT;
    nowAngle[i] = targetAngle[i]; // 現在の角度も目標と同じに
    // サーボを90度＋微調整角度に設定
    servo[i].write((nowAngle[i] >> SHIFT) + trim[i]);
  }
  for(i = 0; i < 3; i++) {
    targetBright[i] = 0 << SHIFT; // LEDの目標明るさを0に設定
    nowBright[i] = targetBright[i]; // 現在の明るさも目標と同じに
    analogWrite(eyes[i], nowBright[i] >> SHIFT); // LEDを消灯
  }
  
  // シリアル通信を開始します。PCとのデータ送受信に使用します。
  Serial.begin(57600); // ボーレートは57600bpsに設定

  delay(500); // 起動安定化のための短い遅延

  // サーボモーターへの電源供給を制御するピンを設定し、電源をオンにします。
  pinMode(POWER, OUTPUT);     // POWERピンを出力として設定
  digitalWrite(POWER, HIGH);  // HIGHにすることでサーボ電源をオン
}  

// --- メインループ関数 ---
// setup()関数の実行後に繰り返し実行されます
void loop()  {
  int buf = ERR; // 一時的なバッファ変数

  // シリアルポートにデータが来ているかを確認します
  if(Serial.available()) {
    // コマンドの開始マーカー '#' を読み込みます
    if(Serial.read() == '#') {
      while(!Serial.available()){} // 次のコマンド文字が来るまで待機
      char commandChar1 = Serial.read(); // 1文字目のコマンドを読み込み
      while(!Serial.available()){} // 2文字目のコマンドが来るまで待機
      char commandChar2 = Serial.read(); // 2文字目のコマンドを読み込み

      // 2文字のコマンドを結合して処理します
      // 例: 'H' + 'Y' -> "HY" (Head Yaw)
      if (commandChar1 == 'H' && commandChar2 == 'Y') { // Head Yaw
        int angle = readThreeDigit(180);
        if (angle != ERR) {
          while(!Serial.available()){}
          if (Serial.read() == 'T') {
            int time = readThreeDigit(255);
            if (time != ERR) {
              moveHeadYaw(angle, time);
            } else { Serial.print("#EHY_T"); }
          } else { Serial.print("#EHY_NT"); }
        } else { Serial.print("#EHY_A"); }
      } else if (commandChar1 == 'W' && commandChar2 == 'Y') { // Waist Yaw
        int angle = readThreeDigit(180);
        if (angle != ERR) {
          while(!Serial.available()){}
          if (Serial.read() == 'T') {
            int time = readThreeDigit(255);
            if (time != ERR) {
              moveWaistYaw(angle, time);
            } else { Serial.print("#EWY_T"); }
          } else { Serial.print("#EWY_NT"); }
        } else { Serial.print("#EWY_A"); }
      } else if (commandChar1 == 'R' && commandChar2 == 'S') { // Right Shoulder (次の文字でRoll/Pitchを判別)
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'R') { // Right Shoulder Roll
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveRSholderRoll(angle, time);
              } else { Serial.print("#ERSR_T"); }
            } else { Serial.print("#ERSR_NT"); }
          } else { Serial.print("#ERSR_A"); }
        } else if (commandChar3 == 'P') { // Right Shoulder Pitch
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveRSholderPitch(angle, time);
              } else { Serial.print("#ERSP_T"); }
            } else { Serial.print("#ERSP_NT"); }
          } else { Serial.print("#ERSP_A"); }
        } else { Serial.print("#ERS_INV"); } // 無効な右肩コマンド
      } else if (commandChar1 == 'R' && commandChar2 == 'H') { // Right Hand Grip
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'G') {
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveRHandGrip(angle, time);
              } else { Serial.print("#ERHG_T"); }
            } else { Serial.print("#ERHG_NT"); }
          } else { Serial.print("#ERHG_A"); }
        } else { Serial.print("#ERH_INV"); } // 無効な右手コマンド
      } else if (commandChar1 == 'L' && commandChar2 == 'S') { // Left Shoulder (次の文字でRoll/Pitchを判別)
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'R') { // Left Shoulder Roll
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveLSholderRoll(angle, time);
              } else { Serial.print("#ELSR_T"); }
            } else { Serial.print("#ELSR_NT"); }
          } else { Serial.print("#ELSR_A"); }
        } else if (commandChar3 == 'P') { // Left Shoulder Pitch
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveLSholderPitch(angle, time);
              } else { Serial.print("#ELSP_T"); }
            } else { Serial.print("#ELSP_NT"); }
          } else { Serial.print("#ELSP_A"); }
        } else { Serial.print("#ELS_INV"); } // 無効な左肩コマンド
      } else if (commandChar1 == 'L' && commandChar2 == 'H') { // Left Hand Grip
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'G') {
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveLHandGrip(angle, time);
              } else { Serial.print("#ELHG_T"); }
            } else { Serial.print("#ELHG_NT"); }
          } else { Serial.print("#ELHG_A"); }
        } else { Serial.print("#ELH_INV"); } // 無効な左手コマンド
      } else if (commandChar1 == 'R' && commandChar2 == 'F') { // Right Foot (次の文字でYaw/Pitchを判別)
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'Y') { // Right Foot Yaw
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveRFootYaw(angle, time);
              } else { Serial.print("#ERFY_T"); }
            } else { Serial.print("#ERFY_NT"); }
          } else { Serial.print("#ERFY_A"); }
        } else if (commandChar3 == 'P') { // Right Foot Pitch
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveRFootPitch(angle, time);
              } else { Serial.print("#ERFP_T"); }
            } else { Serial.print("#ERFP_NT"); }
          } else { Serial.print("#ERFP_A"); }
        } else { Serial.print("#ERF_INV"); } // 無効な右足コマンド
      } else if (commandChar1 == 'L' && commandChar2 == 'F') { // Left Foot (次の文字でYaw/Pitchを判判)
        while(!Serial.available()){}
        char commandChar3 = Serial.read();
        if (commandChar3 == 'Y') { // Left Foot Yaw
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveLFootYaw(angle, time);
              } else { Serial.print("#ELFY_T"); }
            } else { Serial.print("#ELFY_NT"); }
          } else { Serial.print("#ELFY_A"); }
        } else if (commandChar3 == 'P') { // Left Foot Pitch
          int angle = readThreeDigit(180);
          if (angle != ERR) {
            while(!Serial.available()){}
            if (Serial.read() == 'T') {
              int time = readThreeDigit(255);
              if (time != ERR) {
                moveLFootPitch(angle, time);
              } else { Serial.print("#ELFP_T"); }
            } else { Serial.print("#ELFP_NT"); }
          } else { Serial.print("#ELFP_A"); }
        } else { Serial.print("#ELF_INV"); } // 無効な左足コマンド
      } else {
        Serial.print("#E"); // 未知のコマンドに対するエラー応答
      }
      
      // --- ここから追加された部分 ---
      // コマンド処理後、シリアルバッファに残っている可能性のある余分なデータをクリア
      // 特にシリアルモニターから送信される改行コード（CR/LF）などを読み飛ばします。
      while (Serial.available()) {
        Serial.read(); // 1バイトずつ読み捨てます
      }
      // --- 追加された部分ここまで ---
    }
  }
  
  // --- サーボとLEDの補間処理 ---
  // 動きの終了時刻が現在時刻より未来（まだ動きの途中）である場合
  if(endTime > millis()) {
    remainingTime = (endTime - millis()) / 10; // 残り時間を10ミリ秒単位で計算
    
    // 各サーボについて線形補間を行います
    for( i = 0; i < MAXSN; i++) {
      // 目標角度から残り時間に応じた変化量を引いて、現在の角度を計算
      nowAngle[i] = targetAngle[i] - (deltaAngle[i] * remainingTime);
      // 計算された角度をサーボに書き込みます。内部表現から元のスケールに戻し、微調整角度を適用。
      servo[i].write((nowAngle[i] >> SHIFT) + trim[i]);
    }
    // 各LEDについても同様に線形補間を行います (このスケッチではLED制御コマンドは含まれていませんが、ロジックは残しています)
    for( i = 0; i < 3; i++) {
      nowBright[i] = targetBright[i] - (deltaBright[i] * remainingTime);
      analogWrite(eyes[i], nowBright[i] >> SHIFT);
    }
  } 
  // 動きが完了している場合は、特に何もしません。
  // ここに、動きが完了した後の処理（例えば、サーボ電源をオフにするなど）を追加できます。
  else {
    // 例: 動きが完全に停止してから500ミリ秒以上経過したら、サーボ電源をオフにする（現在コメントアウト）
    // if(millis() > endTime + 500) {
    //   digitalWrite(POWER, LOW); // サーボ電源オフ
    // }
  }
}

// --- 個別サーボ制御の基盤関数 ---
// 指定されたサーボを、目標角度に、指定された時間で動かします。
// servoIndex: 動かすサーボのインデックス (0-11)
// targetAngleValue: サーボが到達すべき目標角度 (0-180度)
// durationTime: 移動にかける時間 (0.1秒単位, 0の場合は即時移動)
void moveSingleServo(int servoIndex, int targetAngleValue, int durationTime) {
  // サーボインデックスが有効な範囲内か確認します
  if (servoIndex < 0 || servoIndex >= MAXSN) {
    Serial.print("#ES_INVALID_INDEX"); // エラー応答
    return;
  }

  // サーボモーターの電源をオンにします（既にオンでも問題なし）
  digitalWrite(POWER, HIGH); 

  // 移動時間が0の場合、サーボを即座に目標角度に設定します
  if (durationTime == 0) {
    nowAngle[servoIndex] = targetAngleValue << SHIFT; // 現在の角度を目標に設定（内部表現）
    // サーボを即時移動。内部表現を通常の角度に戻し、微調整角度を適用。
    servo[servoIndex].write((nowAngle[servoIndex] >> SHIFT) + trim[servoIndex]);
    deltaAngle[servoIndex] = 0; // 補間は不要なので変化量は0
    endTime = millis(); // 動きの終了時間を現在時刻に設定
  } else { // 移動時間が指定されている場合、スムーズに移動させます
    targetAngle[servoIndex] = targetAngleValue << SHIFT; // 目標角度を設定（内部表現）
    // 角度変化量（10ミリ秒あたりの移動量）を計算します。
    // (目標角度の内部表現 - 現在角度の内部表現) / (指定時間(0.1秒) * 10 (ミリ秒/0.1秒))
    deltaAngle[servoIndex] = (targetAngle[servoIndex] - nowAngle[servoIndex]) / (durationTime * 10);

    startTime = millis(); // 動きの開始時間を記録
    endTime = startTime + (durationTime * 100); // 動きの終了時間を計算 (durationTimeは0.1秒単位なので100を掛けてミリ秒に変換)
  }

  // コマンド成功の応答をPCに返します
  Serial.print("#SS_OK"); // Single Servo Move Success
  // printThreeDigit(servoIndex);      // デバッグ用: 動かしたサーボのインデックス
  // printThreeDigit(targetAngleValue); // デバッグ用: 設定した目標角度
  // printThreeDigit(durationTime);    // デバッグ用: 設定した移動時間
}

// --- 各部位ごとの制御関数 ---
// これらの関数は、対応するサーボのインデックスを指定して moveSingleServo を呼び出します。

void moveHeadYaw(int angle, int time) { // 頭のヨー軸 (左右回転)
  moveSingleServo(0, angle, time);
  Serial.print("#HY_OK"); // 応答
}

void moveWaistYaw(int angle, int time) { // 腰のヨー軸 (左右回転)
  moveSingleServo(1, angle, time);
  Serial.print("#WY_OK"); // 応答
}

void moveRSholderRoll(int angle, int time) { // 右肩のロール軸 (腕を上下)
  moveSingleServo(2, angle, time);
  Serial.print("#RSR_OK"); // 応答
}

void moveRSholderPitch(int angle, int time) { // 右肩のピッチ軸 (腕を前後)
  moveSingleServo(3, angle, time);
  Serial.print("#RSP_OK"); // 応答
}

void moveRHandGrip(int angle, int time) { // 右手グリップ (開閉)
  moveSingleServo(4, angle, time);
  Serial.print("#RHG_OK"); // 応答
}

void moveLSholderRoll(int angle, int time) { // 左肩のロール軸 (腕を上下)
  moveSingleServo(5, angle, time);
  Serial.print("#LSR_OK"); // 応答
}

void moveLSholderPitch(int angle, int time) { // 左肩のピッチ軸 (腕を前後)
  moveSingleServo(6, angle, time);
  Serial.print("#LSP_OK"); // 応答
}

void moveLHandGrip(int angle, int time) { // 左手グリップ (開閉)
  moveSingleServo(7, angle, time);
  Serial.print("#LHG_OK"); // 応答
}

void moveRFootYaw(int angle, int time) { // 右足のヨー軸 (足首の左右)
  moveSingleServo(8, angle, time);
  Serial.print("#RFY_OK"); // 応答
}

void moveRFootPitch(int angle, int time) { // 右足のピッチ軸 (足首の前後)
  moveSingleServo(9, angle, time);
  Serial.print("#RFP_OK"); // 応答
}

void moveLFootYaw(int angle, int time) { // 左足のヨー軸 (足首の左右)
  moveSingleServo(10, angle, time);
  Serial.print("#LFY_OK"); // 応答
}

void moveLFootPitch(int angle, int time) { // 左足のピッチ軸 (足首の前後)
  moveSingleServo(11, angle, time);
  Serial.print("#LFP_OK"); // 応答
}


// --- シリアル通信ヘルパー関数 ---

// 整数値を3桁の文字列としてシリアル出力する関数 (例: 5 -> "005", 25 -> "025")
int printThreeDigit(int buf) {
  String s = String(buf);
  if(s.length() == 2){
    Serial.print("0");
  } else if (s.length() == 1) {
    Serial.print("00");
  }
  Serial.print(s);
  return 0; // 戻り値は特に意味はないが、関数の型に合わせる
}

int digit; // readThreeDigit() で使用される一時変数

// シリアルからASCII文字の3桁の数字を読み込み、整数に変換する関数
// maximum: 読み込んだ数値の最大許容値
int readThreeDigit(int maximum) {
  int buf;
  buf = readOneDigit(); // 1桁目を読み込み
  if(buf != ERR) {
    digit = buf * 100;
    buf = readOneDigit(); // 2桁目を読み込み
    if(buf != ERR) {
      digit += buf * 10;
      buf = readOneDigit(); // 3桁目を読み込み
      if(buf != ERR) {
        digit += buf;
        if(digit <= maximum) { // 最大値を超えていないか確認
          buf = digit;
        } else {
          buf = ERR; // 最大値を超えている場合エラー
        }
      }
    }
  }
  return buf; // 読み込んだ数字、またはエラーを返す
}

// シリアルからASCII文字の1桁の数字を読み込む関数
int readOneDigit() {
  int buf;
  while(!Serial.available()) {} // シリアルデータが来るまで待機
  buf = Serial.read() - 48; // ASCII '0' ('48') を引いて数値に変換
  if(buf < 0 || 9 < buf){ // 読み込んだ文字が数字 (0-9) でない場合
    buf = ERR; // エラー
  }
  return buf; // 読み込んだ数字、またはエラーを返す
}
