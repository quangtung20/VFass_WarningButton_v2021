/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hw.h"
#include "systemEvent.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define DATALOGGER_NUMBER "0365106405" // sdt Datalogger
#define SIM_WARNING_MSG "*canhbao5" // tin nhan canh bao nut nhan gui sang datalogger bang song vien thong
#define SIM_STOP_MSG "*stop" // tin nhan dung canh bao nut nhan gui sang datalogger bang song vien thong
#define LRWAN_WARNING_MSG "MS+VFASS00100000001WNLV5" // tin nhan canh bao gui qua AT mega bang giao thuc UART
#define LRWAN_STOP_MSG "MS+VFASS00100000001WSTOP" // tin nhan dung canh bao gui qua AT mega bang giao thuc UART
#define LRWAN_WARNING_RES_MSG "OK" // tin nhan nhan ve sau khi AT mega hoan thanh canh bao
#define LRWAN_STOP_RES_MSG "OK" // tin nhan nhan ve sau khi AT mega hoan thanh dung canh bao
#define LRWAN_DATA_RES_MSG "SM+PERDATA_OK" // tin nhan nhan ve tu AT mega sau khi du lieu da duoc truyen qua AT mega
#define LRWAN_CLC_OK "SM+CLC_OK" // tin nhan nhan ve tu AT mega khi co song lora
#define LRWAN_CLC_LOST "SM+CLC_LOST" // tin nhan nhan ve tu AT mega khi khong co song lora
#define LOW_BAT_MSG "pin yeu" // tin nhan gui di den cac so dien thoai duoc chi dinh khi pin yeu
#define LOW_BAT_PHONE_NUMBER_1 "0365106405" // So dien thoai chi dinh duoc gui tin nhan bao pin yeu
#define TIME_MSG_DISPLAY 5000 // thoi gian hien thi hoan thanh, hoac that bai khi canh bao
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

sim7x00 SIM7600E(&huart1) ;
debug SYSTEM(&huart5) ;
lrwan LRWAN(&huart4) ;

uint8_t warningProcess = IDLE ;
uint8_t stopProcess = IDLE ;
uint8_t updateProcess = IDLE ;
uint8_t	btnWarning 	= IDLE ;
uint8_t btnStop 	= IDLE ;
uint8_t sleepStatus = IDLE ;
bool initFlag = false ;
bool waitFlag = false ;
bool doneWarningFlag = false ;

sim_events_t sim7600Event ;
lr_events_t lrwanEvent ;

/* USER CODE END PV */
int warningEvent = IDLE_M ;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/*____________________________________Xử lí ngắt UART______________________________________________*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if ( huart -> Instance == USART1 ) {
		SIM7600E.IRQhandler() ;
	}
	if(huart->Instance == USART4){
		LRWAN.IRQhandler() ;
	}
}


/*____________________________________Xử lí ngắt của các nút nhấn______________________________________________*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==WARNING_BUTTON_Pin){
		// Vì vi điều khiển mới thức dậy và systick đã dừng trước lúc ngủ nên phải config lại systick
		SystemClock_Config ();
		HAL_ResumeTick();

		if((stopProcess == 0 && initFlag == true)){
			waitFlag = false;
			warningProcess=BTN_START_WARNING_PRESSED;
//			SSD1306_Putstring(4, 5, (char*)"SAP TOI : CANH BAO", &Font_5x7, SSD1306_COLOR_WHITE);
		}else {
			SSD1306_Putstring(4, 5, (char*)"KHONG AN CANH BAO", &Font_5x7, SSD1306_COLOR_WHITE);
		}


		for(uint16_t i=0;i<10000;i++);
		while(HAL_GPIO_ReadPin(WARNING_BUTTON_GPIO_Port, WARNING_BUTTON_Pin)==0);
		for(uint16_t i=0;i<10000;i++);
		EXTI->PR |= WARNING_BUTTON_Pin;
	}
	if(GPIO_Pin==STOP_BUTTON_Pin){
		// Vì vi điều khiển mới thức dậy và systick đã dừng trước lúc ngủ nên phải config lại systick
		SystemClock_Config ();
		HAL_ResumeTick();

		if((warningProcess==0 && initFlag == true)){
			waitFlag = false;
			stopProcess =BTN_STOP_WARNING_PRESSED;
//			SSD1306_Putstring(4, 5, (char*)"SAP TOI : DUNG ", &Font_5x7, SSD1306_COLOR_WHITE);
		}else {
			SSD1306_Putstring(4, 5, (char*)"KHONG DUNG CANH BAO", &Font_5x7, SSD1306_COLOR_WHITE);
		}

		for(uint16_t i=0;i<10000;i++);
		while(HAL_GPIO_ReadPin(STOP_BUTTON_GPIO_Port, STOP_BUTTON_Pin)==0);
		for(uint16_t i=0;i<10000;i++);
		EXTI->PR |= STOP_BUTTON_Pin;
	}
}


/*____________________________________Xử lí sự kiện ngắt RTC____________________________________________*/
//void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
//{
//	SystemClock_Config();
//	HAL_ResumeTick();
//	updateProcess=MQTT_PUBLIC_DATA;
//	SSD1306_Putstring(4, 5, (char*)"SAP TOI : CAP NHAT", &Font_5x7, SSD1306_COLOR_WHITE);
//}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/*____________________________________Đọc dữ liệu Pin____________________________________________*/

char* readPin() {

	 static char bat[3];
	 uint16_t adc;
	 float battery;
	 HAL_ADC_Start(&hadc);
	 adc=HAL_ADC_GetValue(&hadc);
	 battery=((adc/4095.0)*100);
	 sprintf(bat,"%d",(int)battery);
	 return bat;
}


/*_______________________Hiển thị cột sóng viễn thông, biểu tượng lrwan, thời lượng pin ______________________*/
void preHeader(){
	SSD1306_Clear_Header();
	SSD1306_DrawBitmap(0, 0, simSignal, 15, 15, SSD1306_COLOR_WHITE );
	SSD1306_UpdateScreen();
	// Hiển thị Pin
	char bat[50]="";
	strcat( bat, readPin());

	if (atoi( bat )>20){
	SSD1306_DrawBitmap( 90, 1, fullBat, 14, 15, SSD1306_COLOR_WHITE );
	}else {
	SSD1306_DrawBitmap( 90, 1, lowBat, 9, 15, SSD1306_COLOR_WHITE );
	}

	strcat(bat,"%");
	SSD1306_GotoXY ( 103, 5);
	SSD1306_Puts ( (char*)bat, &Font_5x7, SSD1306_COLOR_WHITE );
	SSD1306_UpdateScreen();

	// Hiển thị cây cột sóng

//	SSD1306_DrawBitmap( 40, 1, signal, 23, 15, SSD1306_COLOR_WHITE );
//	SSD1306_UpdateScreen();
}

/*_______________________ Hiển thị sóng viễn thông, tình trạng kết nối lrwan, thời lượng pin __________________________*/
void displayHeader(){
	SSD1306_Clear_Header();
	// Hiển thị cột sóng

	SSD1306_DrawBitmap(0, 0, simSignal, 15, 15, SSD1306_COLOR_WHITE );
	if ( SIM7600E.getIntCSQ() > 6 ){
		SSD1306_DrawFilledRectangle(13, 10, 2, 5, SSD1306_COLOR_WHITE );
	}
	if ( SIM7600E.getIntCSQ() > 12 ){
		SSD1306_DrawFilledRectangle(17, 7, 2, 8, SSD1306_COLOR_WHITE );
	}
	if ( SIM7600E.getIntCSQ() > 18){
		SSD1306_DrawFilledRectangle(21, 4, 2, 11, SSD1306_COLOR_WHITE );
	}
	if ( SIM7600E.getIntCSQ() > 24){
		SSD1306_DrawFilledRectangle(25, 1, 2, 14, SSD1306_COLOR_WHITE );
	}
	SSD1306_UpdateScreen();
	// Hiển thị Pin
	char bat[50]="";
	strcat( bat, readPin());

	if (atoi( bat )>20){
	SSD1306_DrawBitmap( 90, 1, fullBat, 14, 15, SSD1306_COLOR_WHITE );
	}else {
	SSD1306_DrawBitmap( 90, 1, lowBat, 9, 15, SSD1306_COLOR_WHITE );
//	SIM7600E.sendSMS(LOW_BAT_PHONE_NUMBER_1, LOW_BAT_MSG);
	}

	strcat(bat,"%");
	SSD1306_GotoXY ( 103, 5);
	SSD1306_Puts ( (char*)bat, &Font_5x7, SSD1306_COLOR_WHITE );
	SSD1306_UpdateScreen();

	// Hiển thị cây cột sóng

//	SSD1306_DrawBitmap( 40, 1, signal, 23, 15, SSD1306_COLOR_WHITE );
//	SSD1306_UpdateScreen();
}


/*____________________Kiểm tra sim đã hoàn thành cảnh báo chưa, nếu chưa thì cảnh báo bằng lrwan_______________________*/

bool simCheckingRespond(const char* resMsg){

	if ( SIM7600E.checkingRespond( resMsg, 20000) == 1 ){
	   SYSTEM.println( "OK" );
	   return 1;
	}else {

		SYSTEM.println("Button was not responded by SIM");
		HAL_Delay(1000);
		return 0;
	}
	return 0 ;
}


/*______________________Kiểm tra lrwan đã hoàn thành cảnh báo chưa__________________________*/
//
int lrwanSendAndCheckingRespond(const char* sms, const char* expectAnswer ){

	char payload[50]={0};
	strcat(payload,sms);
	strcat(payload,"\r");
	// setup payload dữ liệu gửi lora
	if(LRWAN.sendData(sms,expectAnswer,15000,3)!=0){
		return 1;
	}else{
		HAL_Delay(2000);
		SYSTEM.println("!!!!Failed to send by LORA!!!\n");
		return 0;
	}
}


/*_____________________________Cấp nguồn cho SIM, AT MEGA___________________________________*/
void prepareStopMode(){

	HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LRWAN_ONOFF_GPIO_Port, LRWAN_ONOFF_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SIM7600_ONOFF_GPIO_Port, SIM7600_ONOFF_Pin, GPIO_PIN_RESET);

}


/*_______________________________Ngắt nguồn SIM, AT MEGA____________________________________*/
void prepareForProcess(){

	HAL_GPIO_WritePin(LRWAN_ONOFF_GPIO_Port, LRWAN_ONOFF_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SIM7600_ONOFF_GPIO_Port, SIM7600_ONOFF_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LRWAN_RESRT_GPIO_Port,LRWAN_RESRT_Pin, GPIO_PIN_SET);

}

/*____________________________________Khởi tạo các thông số, Sim, lrwan, oled_____________________________________*/


void displayDoneWarning(bool doneWarningFlag){
	SSD1306_Clear();
	SSD1306_DrawBitmap(2, 16, logo, 128, 35, SSD1306_COLOR_WHITE);
	displayHeader();
	while(waitFlag == true){
		HAL_Delay(1000);
		if(doneWarningFlag == true){
			SSD1306_Putstring(4,10, (char*)"HOAN THANH CANH BAO", &Font_5x7, SSD1306_COLOR_WHITE);
		}else{
			SSD1306_Putstring(4,8, (char*)"XIN VUI LONG THU LAI", &Font_5x7, SSD1306_COLOR_WHITE);
		}

		HAL_Delay(2000);
		SSD1306_Clear_Bottom();
	}
	SSD1306_Clear_Data();
}

void init(){

	initFlag = false;
	SSD1306_Init();
	SSD1306_Clear();
	SSD1306_DrawBitmap(2, 16, logo, 128, 35, SSD1306_COLOR_WHITE);
	SSD1306_Putstring(4,15, (char*)"DANG KHOI DONG...", &Font_5x7, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();

	// Hiển thị logo
	prepareForProcess();
	preHeader();
	LRWAN.init() ;

	if(SIM7600E.init(30000)==1){
		SSD1306_Putstring(4,30, (char*)"HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
	}else{
		SSD1306_Putstring(4,30, (char*)"SIM BI LOI!", &Font_5x7, SSD1306_COLOR_WHITE);
	}

	// hiển thị header sau khi khởi động xong
	displayHeader();
	SYSTEM.println("DONE INIT");
	HAL_Delay(TIME_MSG_DISPLAY);
	initFlag = true;
	waitFlag = true;
}


/*________________________________Cài đặt để vào chế độ ngủ________________________________________*/

void enterStopMode(){
	// khi đang vào chế độ ngủ thì set cờ sleepStatus lên 1 để không thể thực hiện tác vụ nào trong lúc chuẩn bị ngủ nữa
	sleepStatus = 1;
	SSD1306_Clear_Data();
	prepareStopMode();
	SSD1306_Putstring(3,5, (char*)"TIET KIEM NANG LUONG", &Font_5x7, SSD1306_COLOR_WHITE);
	SYSTEM.println("ENTER STOP MODE");
	HAL_Delay(TIME_MSG_DISPLAY);
	SSD1306_Clear();
	sleepStatus = 0;
	// Vì systick là ngắt hệ thống nên cần ngừng lại để tránh đánh thức vi điều kiển khi vào chế độ ngủ
	HAL_SuspendTick();
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);
}


/*__________________________________Gửi dữ liệu thiết bị bằng lrwan____________________________________*/

void lrwanUpdateData(){

		char* csqData = SIM7600E.getCSQ() ;
		char* pin = readPin();
		char lwPayload[100]="MS+PERDATA:";
		strcat(lwPayload,csqData);
		strcat(lwPayload,pin);
		strcat(lwPayload,"\r");
		// setup payload để gửi dữ liệu qua atmega
		SSD1306_Clear_Status2();
		SSD1306_Putstring(4, 5, (char*)"                        ", &Font_5x7, SSD1306_COLOR_WHITE);
		SSD1306_Putstring(2, 5, (char*)"______CAP NHAT______", &Font_5x7, SSD1306_COLOR_WHITE);

		SSD1306_Putstring(3, 5, (char*)"LoRa/LoRa ... ", &Font_5x7, SSD1306_COLOR_WHITE);

		if ( LRWAN.sendData(lwPayload, LRWAN_DATA_RES_MSG, 8000,3 )){
			SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
		}else {
			SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
		}
		HAL_Delay(1000);
		memset ( csqData, 0, strlen(csqData) ) ;
		memset ( pin, 0, strlen(pin) ) ;
		HAL_Delay(TIME_MSG_DISPLAY);
}


/*_______________________________________Gửi dữ liệu thiết bị bằng SIM______________________________________*/

void MQTTPublishData(){
		char* csqData = SIM7600E.getCSQ() ;
		char* pin = readPin();
		char lwPayload[100]="{\"data\": \"BA+PERDATA:";
		strcat(lwPayload,csqData);
		strcat(lwPayload,pin);
		strcat(lwPayload,"\"}\r");
		// setup payload để gửi dữ liệu lên broker
		SSD1306_Clear_Data();
		SSD1306_Putstring(2, 5, (char*)"______CAP NHAT______", &Font_5x7, SSD1306_COLOR_WHITE);

		SSD1306_Putstring(3, 5, (char*)"3G/4G LTE ... ", &Font_5x7, SSD1306_COLOR_WHITE);

		if(SIM7600E.mqttUpdateData(lwPayload)){
		   SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
		}else {
			SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
		}
		HAL_Delay(1000);
		memset ( csqData, 0, strlen(csqData) ) ;
		memset ( pin, 0, strlen(pin) ) ;
		HAL_Delay(TIME_MSG_DISPLAY);

}


/*______________________________________Hàm chính chạy chương trình_______________________________________*/

void OS_loop(){
	if (  SIM7600E.rxDone_FLAG == true  ){
		SYSTEM.println("\n==================================================") ;
		sim7600Event = SIM7600E.identifyEvent() ;
		SIM7600E.memreset() ;
	}

	if (  LRWAN.rxDone_FLAG == true  ){
		SYSTEM.println("\n==================================================") ;
		lrwanEvent = LRWAN.identifyEvent() ;
		LRWAN.memreset() ;
	}
	init();
	while(waitFlag == true){
			HAL_Delay(1000);
			SSD1306_Putstring(4,12, (char*)"SAN SANG KICH HOAT", &Font_5x7, SSD1306_COLOR_WHITE);
			HAL_Delay(1000);
			SSD1306_Clear_Bottom();
	}
	SSD1306_Clear_Data();

	// kiển tra trạng thái nếu không làm công việc gì thì sẽ ngủ
	while(warningProcess!=0||stopProcess!=0){
		// kiểm tra trạng thái nút cảnh báo
		switch(warningProcess){
			case BTN_START_WARNING_PRESSED:
				warningProcess = SEND_CMD_BY_SIM;
				SYSTEM.println("ENTER START WARNING PROCESS");
				break;

			// Đầu tiên, gửi cảnh báo qua SMS trước
			case SEND_CMD_BY_SIM:
				SSD1306_Clear_Data();
				SSD1306_Putstring(2, 5, (char*)"______CANH BAO______", &Font_5x7, SSD1306_COLOR_WHITE);
				HAL_Delay(TIME_MSG_DISPLAY);
				SSD1306_Putstring(3, 5, (char*)"Dang gui SMS... ", &Font_5x7, SSD1306_COLOR_WHITE);
				SYSTEM.println("Send SMS ...");
				SIM7600E.sendSMS(DATALOGGER_NUMBER, SIM_WARNING_MSG);
				warningProcess=SIM_CHECKING_RESPOND;
				break;
			// Sau đó, chờ Datalogger phản hồi qua SMS
			case SIM_CHECKING_RESPOND:
				//nếu gửi thành công thì startProcess = SEND_CMD_COMPLETED, thất bại thì tiếp tục gửi bằng lora
				if(simCheckingRespond("OK")){
					SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SMS SENT SUCCESS");
					warningProcess = SEND_CMD_COMPLETED;
				}else{
					SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SMS SENT FAILED");
					warningProcess = SEND_CMD_BY_LRWAN;
				}
				HAL_Delay(TIME_MSG_DISPLAY);
				break;

			case SEND_CMD_BY_LRWAN:
				SSD1306_Putstring(2, 5, (char*)"______CANH BAO______", &Font_5x7, SSD1306_COLOR_WHITE);
				SSD1306_Putstring(3, 5, (char*)"Dang gui LoRa... ", &Font_5x7, SSD1306_COLOR_WHITE);
				SYSTEM.println("SEND BY LORA");
				SSD1306_Clear_Bottom();
				warningProcess = LRWAN_CHECKING_RESPOND;
				break;

			case LRWAN_CHECKING_RESPOND:
				//nếu gửi thành công thì startProcess = SEND_CMD_COMPLETED, thất bại thì startProcess = IDLE
				if(lrwanSendAndCheckingRespond(LRWAN_WARNING_MSG,LRWAN_WARNING_RES_MSG)){
					SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SEND BY LORA SUCCESS");
					warningProcess = SEND_CMD_COMPLETED;
				}else{
					SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SEND BY LORA FAILED");
					SYSTEM.println("WARNING FAILED");
					warningProcess = IDLE;
				}
				HAL_Delay(TIME_MSG_DISPLAY);
				break;
			case SEND_CMD_COMPLETED:
				HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, GPIO_PIN_SET);
				SYSTEM.println("DONE WARNING PROCESS");
				warningProcess=IDLE;
				doneWarningFlag = true;
				break;
		}
		// kiểm tra trạng thái nút dừng cảnh báo
		switch(stopProcess){
			case BTN_STOP_WARNING_PRESSED:
				stopProcess = SEND_CMD_BY_SIM;
				SYSTEM.println("ENTER STOP WARNING PROCESS");
				break;

			case SEND_CMD_BY_SIM:
				SSD1306_Clear_Data();
				SSD1306_Putstring(2, 5, (char*)"____DUNG CANH BAO____", &Font_5x7, SSD1306_COLOR_WHITE);
				HAL_Delay(TIME_MSG_DISPLAY);
				SSD1306_Putstring(3, 5, (char*)"Dang gui SMS... ", &Font_5x7, SSD1306_COLOR_WHITE);
				SYSTEM.println("Send SMS ...");
				SIM7600E.sendSMS(DATALOGGER_NUMBER, SIM_STOP_MSG);
				stopProcess=SIM_CHECKING_RESPOND;
				break;
			case SIM_CHECKING_RESPOND:
				//nếu gửi thành công thì startProcess = SEND_CMD_COMPLETED, thất bại thì tiếp tục gửi bằng lora
				if(simCheckingRespond("OK")){
					SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SMS SENT SUCCESS");
					stopProcess = SEND_CMD_COMPLETED;
				}else{
					SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SMS SENT FAILED");
					stopProcess = SEND_CMD_BY_LRWAN;
				}
				HAL_Delay(TIME_MSG_DISPLAY);
				break;
			case SEND_CMD_BY_LRWAN:
				SSD1306_Putstring(2, 5, (char*)"____DUNG CANH BAO____", &Font_5x7, SSD1306_COLOR_WHITE);
				SSD1306_Putstring(3, 5, (char*)"Dang gui LoRa... ", &Font_5x7, SSD1306_COLOR_WHITE);
				SYSTEM.println("SEND BY LORA");
				SSD1306_Clear_Bottom();
				stopProcess = LRWAN_CHECKING_RESPOND;
				break;
			case LRWAN_CHECKING_RESPOND:
				//nếu gửi thành công thì stopProcess = SEND_CMD_COMPLETED, thất bại thì stopProcess = IDLE
				if(lrwanSendAndCheckingRespond(LRWAN_STOP_MSG,LRWAN_STOP_RES_MSG)){
					SSD1306_Putstring(4, 5, (char*)"-> HOAN THANH!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SENT BY LORA SUCCESS");
					stopProcess = SEND_CMD_COMPLETED;
				}else{
					SSD1306_Putstring(4, 5, (char*)"-> THAT BAI!", &Font_5x7, SSD1306_COLOR_WHITE);
					SYSTEM.println("SENT BY LORA FAILED");
					SYSTEM.println("STOP WARNING FAILED");
					stopProcess = IDLE;
				}
				HAL_Delay(TIME_MSG_DISPLAY);
				break;
			case SEND_CMD_COMPLETED:
				HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, GPIO_PIN_SET);
				SYSTEM.println("DONE STOP WARNING PROCESS");
				doneWarningFlag = true;
				stopProcess=IDLE;
				break;
		}
		if(stopProcess == 0 && warningProcess == 0){
			HAL_Delay(TIME_MSG_DISPLAY);
			waitFlag = true;

		//	// khi đã kiểm tra và hoàn tất các công việc thì bước vào trạng thái ngủ
			displayDoneWarning(doneWarningFlag);
		}
	}

}


/*________________________________________________________________________________________________*/
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
  MX_RTC_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  updateProcess=MQTT_PUBLIC_DATA;
  /* USER CODE END 2 */
  /* Cấu hình thời gian cho RTC với chu kì ngắt 120s */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 600, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
    {
      Error_Handler();
    }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  OS_loop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
