 #include "Wire.h"

//----------------------------------------------
//  설정 - 키이벤트를 발생하는 문턱값. 무게중심의 좌표값. (범위 : -1.0 ~ 1.0)
//        문턱값을 넘기면 키가 눌리게 됨. 0 에 가까울수록 민감하게 동작.
const float THRESHOLD_COP_forth   = 0.3;    // 앞 쏠림. Y가 이 값보다 커야 키이벤트 발생.
const float THRESHOLD_COP_back    = -0.85;  // 뒤 쏠림. Y가 이 값보다 작아야 키이벤트 발생.
const float THRESHOLD_COP_left    = -0.22;  // 좌 쏠림. X가 이 값보다 작아야 키이벤트 발생.
const float THRESHOLD_COP_right   = 0.22;   // 우 쏠림. X가 이 값보다 커야 키이벤트 발생.
 
//----------------------------------------------
//  무효 판정 문턱값. 측정값이 아래 문턱값보다 낮으면 무시함.
const int THRESHOLD_SUM_row_1st   = 60;   // 1st row, front row
const int THRESHOLD_SUM_row_3rd   = 60;   // 3rd row, back row
const int THRESHOLD_SUM_row_2nd   = 105;  // 2nd row, left and right
const int THRESHOLD_SUM_VERT      = 60; // SUM? PRESSURE?
 
//----------------------------------------------
//  상수 - 방석의 셀 개수
const int CONST_SEAT_CELL_NUM_TOTAL = 31; // Total number of sensor cells = 31.
const int CONST_SEAT_CELL_NUM_ROW_1ST = 6;
const int CONST_SEAT_CELL_NUM_ROW_2ND = 15;
const int CONST_SEAT_CELL_NUM_ROW_3RD = 10;
 
// STRING buffer
char Xstring_arr [16];
char Ystring_arr [16];
char sendAllstring[20]; //add all the string together
 
//----------------------------------------------
//  변수
int adc_value[32]; // 센서 측정값 버퍼
bool is_key_pressed[256];  // 특정 키가 눌렸는지를 기록.
 
//----------------------------------------------
//  하드웨어 핀
int En0 = 7;  //  4 to 16 decoder 0, Low enabled
int En1 = 6;  //  4 to 16 decoder 1, Low enabled
 
int S0  = 5;
int S1  = 4;
int S2  = 3;
int S3  = 2;
int SIG_pin = A3; // common output of two decoder
 
 
void setup() {
  Wire.begin(); //I2C connection start
  Serial.begin(115200);
  pinMode(En0, OUTPUT);
  pinMode(En1, OUTPUT);
 
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(9, OUTPUT); //우측 진동 센서
  pinMode(10, OUTPUT); //좌측 진동 센서
  pinMode(11, OUTPUT); //전방 진동 센서
  pinMode(12, OUTPUT); //후방 전동 센서
}
 
void loop() {
  Wire.beginTransmission(9);// transmit to device 9
   // Read 32 sensor cells.
  for(int i = 0 ; i < 32 ; i++){
    adc_value[i] = readMux(i);
  }
 
  /*
   *    방석 센서의 번호. 센서는 총 31개가 가로로 3줄로 배열되어 있습니다.
   *    아래 그림은 adc_value에 담긴 측정값이 어느 위치의 센서값인지를 설명합니다.
   *    예를 들어 adc_value[14]는 1번째 줄의 3번째 센서의 측정값입니다. 
   *    adc_value[2]는 3번째 줄의 3번째 센서의 측정값입니다.
   *  1st row (6 cells):                       10, 12, 14, 16, 18, 20
   *  2nd row (15 cells) :   5, 6, 7, 8, 9, 11, 13, 15(center), 17, 19, 21, 22, 23, 24, 25
   *  3rd row (10 cells) :                 0, 1, 2, 3, 4, 26, 27, 28, 29, 30
   * 
   * 무게 중심의 X 좌표는 2번 줄의 측정값으로 계산합니다.
   * 무게 중심의 Y 좌표는 1, 3번 줄의 측정값으로 계산합니다.
   */
 //---------------------------------------------------------
 //     1, 3번 줄의 측정값으로 무게 중심의 Y 좌표 계산하기
 //---------------------------------------------------------
  int sum_row_1st = adc_value[10]+adc_value[12]+adc_value[14]+adc_value[16]+adc_value[18]+adc_value[20];
 
  int sum_row_3rd =   adc_value[0]+adc_value[1]+adc_value[2]+adc_value[3]+adc_value[4]
                    +adc_value[26]+adc_value[27]+adc_value[28]+adc_value[29]+adc_value[30];
 
  int sum_vertical = sum_row_1st + sum_row_3rd;
 
  double avg_row_1st = sum_row_1st / CONST_SEAT_CELL_NUM_ROW_1ST;
  double avg_row_3rd = sum_row_3rd / CONST_SEAT_CELL_NUM_ROW_3RD;
 
  //  무게 중심의 Y 좌표 계산
  float cop_vertical = 0.0;
  if( 0 < sum_vertical) {
    cop_vertical = (avg_row_1st * (1) + avg_row_3rd * (-1)) / (avg_row_1st + avg_row_3rd);
  }
 
  //  키 이벤트 발생 - 앞 쏠림, 뒤 쏠림
  if( sum_vertical < THRESHOLD_SUM_VERT) { //합이 특정값이하일때 아이에 반응 노! 
  }
  else {
    if(THRESHOLD_COP_forth < cop_vertical) { //
      strcat (sendAllstring, "Front f");
      analogWrite(11, 128); //전방 센서 진동 시작
    }
    else if(cop_vertical < THRESHOLD_COP_back) {
       strcat (sendAllstring, "Back f");
       analogWrite(12, 128); //후방 센서 진동 시작
    }
    else {
    }
  }
 
 //---------------------------------------------------------
 //     2번 줄의 측정값으로 무게 중심의 X 좌표 계산하기
 //---------------------------------------------------------
  int sum_row_2nd =  adc_value[5]+adc_value[6]+adc_value[7]+adc_value[8]+adc_value[9]
                    +adc_value[11]+adc_value[13]+adc_value[15]+adc_value[17]+adc_value[19]
                    +adc_value[21]+adc_value[22]+adc_value[23]+adc_value[24]+adc_value[25];
 
  //  센서 2 번째 줄의 15개 센서의 측정값에 위치별 가중치(-7~7))를 부여하여 더합니다. 
  //  그것을 7로 나눠서 좌표 범위를 -1~1 로 한정합니다.
  int sum_wp_horizon = (  (-7)*adc_value[5]+(-6)*adc_value[6]+(-5)*adc_value[7]
                          +(-4)*adc_value[8]+(-3)*adc_value[9]+(-2)*adc_value[11]
                          +(-1)*adc_value[13]+(0)*adc_value[15]
                          +(1)*adc_value[17]+(2)*adc_value[19]+(3)*adc_value[21]
                          +(4)*adc_value[22]+(5)*adc_value[23]+(6)*adc_value[24]
                          +(7)*adc_value[25] ) / 7.0; // divide 7.0 ==> unitize. (-7.0~7.0)
 
  float cop_horizon = 0.0;
  //  무게 중심의 X 좌표 계산
  if(0 < sum_row_2nd) {
    cop_horizon = sum_wp_horizon / (double)sum_row_2nd;
  }
 
  //  키 이벤트 발생 - 좌 쏠림, 우 쏠림
  if(sum_row_2nd < THRESHOLD_SUM_row_2nd) {
  }
  else {
    //  press left
    if(cop_horizon < THRESHOLD_COP_left) {
      strcat (sendAllstring, "Left f");
      analogWrite(10, 128); //우측 센서 진동 시작
    }
    //  press right
    else if(THRESHOLD_COP_right < cop_horizon) {
     strcat (sendAllstring, "Right f");
     analogWrite(9, 128); //좌측 센서 진동 시작
    }
  }
  //Serial.print(sendcharstring);
  //  무게 중심 계산값을 출력하여 확인하기.
  Print_XY(cop_horizon, cop_vertical);  
  Serial.println(sendAllstring);
  delay(4000);
  memset(sendAllstring, '\0', 16);
  //  키 이벤트 발생 - 앞 쏠림, 뒤 쏠림
  if(THRESHOLD_COP_forth >= cop_vertical) {
    analogWrite(11, 0); //전방 센서 진동 정지
  }
  if(cop_vertical > THRESHOLD_COP_back) {
     analogWrite(12, 0); //후방 센서 진동 정지
  }
  //  키 이벤트 발생 - 좌 쏠림, 우 쏠림
  if(cop_horizon >= THRESHOLD_COP_left) {
    analogWrite(10, 0); //우측 센서 진동 정지
  }
  //  press right
  if(THRESHOLD_COP_right > cop_horizon) {
    analogWrite(9, 0); //좌측 센서 진동 정지
  }
  delay(1000);//delay
}
 
int readMux(int channel){
  int controlPin[] = {S0,S1,S2,S3,En0,En1};
 
  int muxChannel[32][6]={
    {0,0,0,0,0,1}, //channel 0
    {0,0,0,1,0,1}, //channel 1
    {0,0,1,0,0,1}, //channel 2
    {0,0,1,1,0,1}, //channel 3
    {0,1,0,0,0,1}, //channel 4
    {0,1,0,1,0,1}, //channel 5
    {0,1,1,0,0,1}, //channel 6
    {0,1,1,1,0,1}, //channel 7
    {1,0,0,0,0,1}, //channel 8
    {1,0,0,1,0,1}, //channel 9
    {1,0,1,0,0,1}, //channel 10
    {1,0,1,1,0,1}, //channel 11
    {1,1,0,0,0,1}, //channel 12
    {1,1,0,1,0,1}, //channel 13
    {1,1,1,0,0,1}, //channel 14
    {1,1,1,1,0,1}, //channel 15
    {0,0,0,0,1,0}, //channel 16
    {0,0,0,1,1,0}, //channel 17
    {0,0,1,0,1,0}, //channel 18
    {0,0,1,1,1,0}, //channel 19
    {0,1,0,0,1,0}, //channel 20
    {0,1,0,1,1,0}, //channel 21
    {0,1,1,0,1,0}, //channel 22
    {0,1,1,1,1,0}, //channel 23
    {1,0,0,0,1,0}, //channel 24
    {1,0,0,1,1,0}, //channel 25
    {1,0,1,0,1,0}, //channel 26
    {1,0,1,1,1,0}, //channel 27
    {1,1,0,0,1,0}, //channel 28
    {1,1,0,1,1,0}, //channel 29
    {1,1,1,0,1,0}, //channel 30
    {1,1,1,1,1,0}  //channel 31
  };
 
  //loop through the 6 sig (muxChannel has 6 values)
  for(int i = 0; i < 6; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
 
  //read the value at the SIG pin
  int val = analogRead(SIG_pin);
 
  return val;
}
 
void Print_XY(float x, float y) {
  dtostrf(x, 0, 2, Xstring_arr);
  strcat(Xstring_arr, "s");
  strcat (sendAllstring, Xstring_arr);
  dtostrf(y, 0, 2, Ystring_arr);
  strcat(Ystring_arr, "l");
  strcat (sendAllstring, Ystring_arr);
}

