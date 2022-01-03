/*
 * lrwan.cpp
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#include "hw.h"
#include "systemEvent.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


// ===================================== ALL FUNCTION DEFINITIONS =============================================//

debug lrDebug(&DEBUG_UART) ;
unDebug lrUnDebug ;
// Lệnh cập nhật dữ liệu định kì
#ifndef LR_REQ_DATA_PERIODLY_CODE
#define   LR_REQ_DATA_PERIODLY_CODE "RDP"
#endif

// Lệnh yêu cầu phát cảnh báo theo các mức :
#ifndef LR_REQ_WARNING_LV1_CODE
#define   LR_REQ_WARNING_LV1_CODE "RWL1"
#endif

#ifndef LR_REQ_WARNING_LV2_CODE
#define   LR_REQ_WARNING_LV2_CODE "RWL2"
#endif

#ifndef LR_REQ_WARNING_LV3_CODE
#define   LR_REQ_WARNING_LV3_CODE "RWL3"
#endif

#ifndef LR_REQ_WARNING_LV4_CODE
#define   LR_REQ_WARNING_LV4_CODE "RWL4"
#endif

#ifndef LR_REQ_WARNING_LV5_CODE
#define   LR_REQ_WARNING_LV5_CODE "RWL5"
#endif


// Lệnh dừng cảnh báo :
#ifndef LR_REQ_STOP_WARNING_CODE
#define   LR_REQ_STOP_WARNING_CODE "RSW"
#endif

#ifdef LORAWAN_DEBUG
	#define LR_DEBUG lrDebug
#else
	#undef LR_DEBUG
	#define LR_DEBUG lrUnDebug
#endif
#define LRWAN_CLC_OK "SM+CLC_OK"
#define LRWAN_CLC_LOST "SM+CLC_LOST"
lrwan::lrwan( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}

/*
 * Hàm sim7x00::IRQhandler(); sẽ được đặt vào hàm ngắt UART : void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) ;
 */
/*____________________________________________________________________________________________________________________________*/

void lrwan::IRQhandler(){
	// khi nào nhận được ký tự \r thì sẽ hiểu là đã kết thúc lệnh.
	if(  rxBuff[0] == '\r' ){
		if( strstr(rxData, LRWAN_CLC_LOST) != NULL ){
			lrwanStatus = 2;
			SSD1306_DrawFilledRectangle(66, 0, 15, 15, SSD1306_COLOR_BLACK);
			SSD1306_DrawBitmap( 66, 3, loraFailEvent, 13, 12, SSD1306_COLOR_WHITE );
			SSD1306_UpdateScreen();
		}
		if( strstr(rxData, LRWAN_CLC_OK) != NULL ){
			lrwanStatus =1;
			SSD1306_DrawFilledRectangle(66, 0, 15, 15, SSD1306_COLOR_BLACK);
			SSD1306_DrawBitmap( 66, 2, loraDoneEvent, 15, 14, SSD1306_COLOR_WHITE );
			SSD1306_UpdateScreen();
		}
		rxDone_FLAG = 1 ;
		memset(rxBuff,0,1) ;
	}else{
		// tạo ra 1 biến dữ liệu  rxData = \nOK\n
		rxData[dataCount] = rxBuff[0] ;
		// Cộng dồn dữ liệu lên nhưng phải đảm bảo nằm trong vùng nhớ của bộ đệm ! Không được để quá, sẽ gây treo VĐK ( nguy hiểm )
		if ( dataCount < RX_DATA_MAX ) dataCount++ ;
		else {}
	}
	HAL_UART_Receive_IT(huartx, (uint8_t*)rxBuff, 1) ;
}

void lrwan::memreset(){
	rxDone_FLAG = false ;
	dataCount = 0 ;
	memset ( rxData, 0 , (size_t)sizeof(rxData)/sizeof(char) ) ;
}

/*
 * Hàm gửi dữ liệu và nhận phản hồi từ AT mega*/
/*____________________________________________________________________________________________________________________________*/
bool lrwan::sendCommand (const char* command, const char* expected_answer, int timeout) {

    int answer=0;

    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    memset(rxData, 0, RX_DATA_MAX );

    LR_DEBUG.println("Requested to LRWAN module : ") ;
    LR_DEBUG.print(command) ;

    // Bắt đầu gửi lệnh cmd ngay tại đây
    HAL_UART_Transmit(huartx, (uint8_t*)command, strlen(command), timeout) ;

    LR_DEBUG.println("LRWAN responsed : ") ;

    // lệnh này để bắt đầu lấy mốc t.g để check Timeout
    uint32_t tickStart = HAL_GetTick() ;


    while( answer == 0 ) {
    	// Kiểm tra thời gian phản hồi, nếu lâu quá timeout thì break, và báo lỗi
		if ( (int)( HAL_GetTick() - tickStart )  >=  timeout ) {
			break ;
		}

	    /*
	     * Kiểm tra module sim có phản hồi về đúng expected_answer hay khong
	     * Note : Hàm strstr(X, x) sẽ tìm 1 chuỗi con x trong chuỗi lớn hơn X
	     *  	  Nếu x xuất hiện trong X, thì hàm strstr sẽ trả về 1 chuỗi bắt đầu từ x và chuỗi
	     *  	  kéo dài còn lại phía sau từ x trong X
	     */
		if ( strstr(rxData, expected_answer) != NULL ){
			// Nếu có thi la OK, answer == 1, thoát khỏi vòng while
			answer = 1;
		}

		// Led nhấp nháy báo hiệu đang tương tác với module sim
		if ( ( (int)( HAL_GetTick() - tickStart )/(int)100 % 2 ) == 0 )
			HAL_GPIO_TogglePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin) ;
	}

    // In thông tin mà module lorawan đã phản hồi về :
    LR_DEBUG.println(rxData) ;

    // Nếu kết quả phản hồi không như mong muốn :
    if ( answer == 0 )
    	LR_DEBUG.println("Fail to get responses from LRWAN! ") ;

    HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, (GPIO_PinState) 0 ) ;

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;
    return answer;
}

/*
 * Hàm phản hồi lại AT mega*/
/*____________________________________________________________________________________________________________________________*/
void lrwan::response( const char* response, int timeout ){
	HAL_UART_Transmit(huartx, (uint8_t*)response, strlen(response), timeout) ;
}

/*
 * Hàm xử lí khi dữ liệu đến*/
/*____________________________________________________________________________________________________________________________*/
lr_events_t lrwan::identifyEvent(){
	lr_events_t lrEvent ;
	lrEvent.warning = NON ;
	lrEvent.update = NON ;

	if( strstr(rxData, LR_REQ_WARNING_LV1_CODE) != NULL ){
		lrEvent.warning = LR_REQ_WARNING_LV1 ;
		LR_DEBUG.println("EVENT: [ WARNING ] - WARNING_LEVEL_1" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_WARNING_LV2_CODE) != NULL ){
		lrEvent.warning = LR_REQ_WARNING_LV2 ;
		LR_DEBUG.println("EVENT: [ WARNING ] - WARNING_LEVEL_2" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_WARNING_LV3_CODE) != NULL ){
		lrEvent.warning = LR_REQ_WARNING_LV3 ;
		LR_DEBUG.println("EVENT: [ WARNING ] - WARNING_LEVEL_3" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_WARNING_LV4_CODE) != NULL ){
		lrEvent.warning = LR_REQ_WARNING_LV4 ;
		LR_DEBUG.println("EVENT: [ WARNING ] - WARNING_LEVEL_4" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_WARNING_LV5_CODE) != NULL ){
		lrEvent.warning = LR_REQ_WARNING_LV5 ;
		LR_DEBUG.println("EVENT: [ WARNING ] - WARNING_LEVEL_5" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_STOP_WARNING_CODE) != NULL ){
		lrEvent.warning = LR_REQ_STOP_WARNING ;
		LR_DEBUG.println("EVENT: [ WARNING ] - STOP_WARNING" ) ;
		response("OK\r", 1000 );
	}else if( strstr(rxData, LR_REQ_DATA_PERIODLY_CODE) != NULL ){
		lrEvent.update = LR_REQ_PERIOD_DATA_UPDATE ;
		LR_DEBUG.println("EVENT: [ UPDATE ] - LORAWAN UPDATE DATA" ) ;
		response("OK\r", 1000 );
	}

	return lrEvent ;
}


/*
 * Hàm khởi khởi tạo lrwan*/
/*____________________________________________________________________________________________________________________________*/
void lrwan::init(){
	// Init uart4 :
	MX_USART4_UART_Init();

	// Khoi dong ngat uart4
	HAL_UART_Receive_IT(&huart4, (uint8_t*)rxBuff, 1) ;
}

/*
 * Hàm reset và gửi dữ liệu với retrytime lần*/
/*____________________________________________________________________________________________________________________________*/
bool lrwan::sendData(const char* sms,const char* expectAnswer, int timeout, int retrytime){
	bool tmp =0;
	for(int i=0;i<retrytime;i++){
		LR_DEBUG.println("Prepare for reset LORAWAN");
		HAL_GPIO_WritePin(LRWAN_ONOFF_GPIO_Port, LRWAN_ONOFF_Pin, GPIO_PIN_RESET);
		HAL_Delay(1000);
		HAL_GPIO_WritePin(LRWAN_ONOFF_GPIO_Port, LRWAN_ONOFF_Pin, GPIO_PIN_SET);
		HAL_Delay(4000);
			LR_DEBUG.println("Done Reset LORAWAN");
			if(sendCommand (sms,expectAnswer, timeout)==1){
				tmp=1;
//				response("OK", 1000);
//				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
//				HAL_Delay(1000);
//				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
				break;
			}
	}
	return tmp;
}


/*____________________________________________________________________________________________________________________________*/



#ifdef __cplusplus
}
#endif
