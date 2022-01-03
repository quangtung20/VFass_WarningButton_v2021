/*
 * sim7600.c
 *
 *  Created on: June 21, 2021
 *      Author: LENOVO
 */


#include "hw.h"
#include "systemEvent.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===================================== ALL FUNCTION DEFINITIONS =============================================//

debug simDebug(&DEBUG_UART) ;
unDebug simUnDebug ;
lrwan LRW(&huart4) ;
#ifdef SIM7600_DEBUG
	#define SIM_DEBUG simDebug
#else
	#undef SIM_DEBUG
	#define SIM_DEBUG simUnDebug
#endif



//===================ĐIỀU CHỈNH SỐ ĐIỆN THOẠI CHO PHÉP CẢNH BÁO TẠI ĐÂY==============================//

#define MAX_PHONE_NUMBER_ALLOWED           6  // số điện thoại tối đa cho phép

const char* phoneNumbersAllowed[MAX_PHONE_NUMBER_ALLOWED] = { "+84334293870",     // SĐT của Toàn
													   "+84337477332",      // SĐT a Dũng
													   "+84969146385",		// SĐT a Cảnh
													   "+84914486786",		// SĐT thầy Thanh
													   "+84981605944",
													   "+84365106405",

};


//====================ĐIỀU CHỈNH MÃ CẢNH BÁO TẠI ĐÂY ==========================//

#define MAX_WARNING_CODES                   5  					// số mã cảnh báo tối đa, lưu ý, số mã phải đủ trong mảng dưới


const char* warningCodesAllowed[MAX_WARNING_CODES] = { "canhbao1",	// sắp xếp theo thứ tự ưu tiên từ trên xuống
								 	 	 	     "canhbao2",
											     "canhbao3",
											     "canhbao4",
											     "canhbao5"
};

const char* stopWarningCodesAllowed = 				 "stop" ; 	     // mã dừng cảnh báo

//====================( NÂNG CAO ) CẤU HÌNH CHO MQTT =======================//
#define  MQTT_CLIENT_ID      "button_002"

#define  MQTT_USER_NAME      ""

#define  MQTT_PASSWORD       ""

#define  MQTT_WILL_TOPIC     "offline"

#define  MQTT_WILL_MESSAGE   "disconnected_msg"

#define  MQTT_BROKER_ADDR    "52.230.105.94"

#define  MQTT_BROKER_PORT    "1883"

#define  MQTT_TOPIC          "application/3/device/c44853fd67d240e9/event/cll_up"

//__________________________________________________________________________________________________________________________________________________//
//*****************************************************/ SIM FUNCTIONS /*************************************************************************//


/*____________________________________________________________________________________________________________________________*/
sim7x00::sim7x00( UART_HandleTypeDef * _huartx ){
	huartx = &*_huartx ;
}


/*____________________________________________________________________________________________________________________________*/
void sim7x00::IRQhandler(){
	// khi nào nhận được ký tự \r thì sẽ hiểu là đã kết thúc lệnh.
	if(  rxBuff[0] == '\r' ){
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


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::sendATcommand (const char* ATcommand, const char* expected_answer, int timeout) {

    int answer=0;

    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    memset(rxData, 0, RX_DATA_MAX );

    SIM_DEBUG.println("Requested to SIM7600 : ") ;
    SIM_DEBUG.print(ATcommand) ;

    // Bắt đầu gửi lệnh AT ngay tại đây
    HAL_UART_Transmit(huartx, (uint8_t*)ATcommand, strlen(ATcommand), timeout) ;

    SIM_DEBUG.println("SIM7600E responsed : ") ;

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

    // In thông tin mà module sim đã phản hồi về :
    SIM_DEBUG.println(rxData) ;

    // Nếu kết quả phản hồi không như mong muốn :
    if ( answer == 0 )
    	SIM_DEBUG.println("Fail to get responses from SIM7600E! ") ;

    HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, (GPIO_PinState) 0 ) ;

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::sendATcommand (const char* ATcommand, const char* expected_answer, int timeout, int retryTime ) {

	int retryTimeCounter = 0 ;

    int answer=0;

    while ( answer == 0 && retryTimeCounter < retryTime ) {

		dataCount = 0 ;
		rxDone_FLAG = 0 ;
		memset(rxData, 0, RX_DATA_MAX );

		SIM_DEBUG.println("Requested to SIM7600 : ") ;
		SIM_DEBUG.print(ATcommand) ;

		// Bắt đầu gửi lệnh AT ngay tại đây
		HAL_UART_Transmit(huartx, (uint8_t*)ATcommand, strlen(ATcommand), timeout) ;

		SIM_DEBUG.println("SIM7600E responsed : ") ;

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

		// In thông tin mà module sim đã phản hồi về :
		SIM_DEBUG.println(rxData) ;

		// Nếu kết quả phản hồi không như mong muốn :
		if ( answer == 0 ){
			SIM_DEBUG.println("Fail to get responses from SIM7600E! ") ;
			if ( retryTimeCounter < retryTime ) {
				retryTimeCounter++ ;
			}
		}

		HAL_GPIO_WritePin(LED_Working_Status_GPIO_Port, LED_Working_Status_Pin, (GPIO_PinState) 0 ) ;
    }

    // Reset lại các biến phục vụ trong quá trình lấy dữ liệu từ module sim
    dataCount = 0 ;
    rxDone_FLAG = 0 ;
    //  Do hiện tại, rxData được sử dụng cho các tác vụ kế tiếp sau AT cmd, nên không được reset hàm  memset(rxData, 0, sizeof(rxData) ) ;

    return answer;
}

/*____________________________________________________________________________________________________________________________*/
bool sim7x00::init(int timeout){
	// Khởi tạo UART 1 và kích hoạt ngắt :
//	HAL_UART_Receive_IT(&huart1, (uint8_t*)rxBuff, 1) ;

	// Khởi động module sim bằng cách set chân PWRKEY mức 0 trên module sim
	SIM_DEBUG.println("Sim7x00e booting ..." ) ;
	HAL_UART_Receive_IT( huartx, (uint8_t*)rxBuff, 1 ) ;

	HAL_GPIO_WritePin( SIM7600_PWRKEY_GPIO_Port, SIM7600_PWRKEY_Pin, (GPIO_PinState)0 ) ;
	HAL_GPIO_WritePin( SIM7600_FLIGHTMODE_GPIO_Port, SIM7600_FLIGHTMODE_Pin, (GPIO_PinState)0 ) ;
	HAL_GPIO_WritePin( SIM7600_RESET_GPIO_Port, SIM7600_RESET_Pin, (GPIO_PinState)0 ) ;

	// Khi Start, nếu Sim đã khởi động trước đó ( == true ) thì bỏ qua, còn không ( == false ) thì kích chân

		HAL_GPIO_WritePin( SIM7600_PWRKEY_GPIO_Port, SIM7600_PWRKEY_Pin, (GPIO_PinState)1 ) ;



		HAL_GPIO_WritePin( SIM7600_FLIGHTMODE_GPIO_Port, SIM7600_FLIGHTMODE_Pin, (GPIO_PinState)0 ) ;


		HAL_GPIO_WritePin( SIM7600_RESET_GPIO_Port, SIM7600_RESET_Pin, (GPIO_PinState)0 ) ;

	uint32_t tikStarted = HAL_GetTick();
	while ((unsigned int)(HAL_GetTick() - tikStarted) <= 90000)
	{
		SIM_DEBUG.println(".");
		HAL_Delay(3000);
		IWDG->KR = 0xAAAA;
	}

	sendATcommand( "AT", "OK", 100 ) ;
	sendATcommand( "AT+CREG?", "OK", 100 ) ;
	sendATcommand( "AT+CNMP=2", "OK", 100 ) ;
	sendATcommand( "ATI", "OK", 5000 ) ;
	sendATcommand( "AT+CNMI=2,1,0,0,0", "OK", 100 ) ;
	sendATcommand( "AT+CNMP=2", "OK", 100 ) ;
	sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"", "OK", 200 ); // Select đường dẫn bộ nhớ tin nhắn
	sendATcommand("AT+CMGL=\"ALL\"", "OK", 5000);
	sendATcommand( "AT+CMGD=,4", "OK", 100 ) ; // Xóa hết toàn bộ tin nhắn
	sendATcommand( "AT+CMGF=1", "OK", 100 ) ;
	// update date and time automatically
	sendATcommand("AT+CTZU=1", "OK", 5000);
	IWDG->KR = 0xAAAA;

	return true ;

}


/*____________________________________________________________________________________________________________________________*/
void sim7x00::memreset(){
	rxDone_FLAG = false ;
	dataCount = 0 ;
	memset ( rxData, 0 , (size_t)sizeof(rxData)/sizeof(char) ) ;
}


/*____________________________________________________________________________________________________________________________*/
sim_events_t sim7x00::identifyEvent(){
	sim_events_t simEvent ;
	simEvent.update = NONE ;
	simEvent.warning = NONE ;

	// Kiểm tra sự kiện đến từ module sim :
	switch ( checkIncommingEvent() ) {
		/*------------------------------------------------------------------------
		 * TIN NHẮN :
		 */
		case IS_SMS:
			// Thực hiện theo quy trình sau :

			memset(SMSContent, 0, RX_DATA_MAX) ;

			// Đầu tiên phải mở SMS ra đọc :
			readSMS() ;

			// Tiến hành xác thực Số điện thoại :
			if( isAuthorizePhoneNumber(SMSContent) ){
				// Nếu đúng số điện thoại cho phép, thì kiểm tra nội dung yêu cầu :
				int result = NONE ;
				result = identifySMSRequest(SMSContent) ;

				switch (result) {
					case SIM_REQ_WARNING_LV1:
						simEvent.warning = SIM_REQ_WARNING_LV1 ;
						SIM_DEBUG.print("WARNING_LEVEL_1" ) ;
						break;
					case SIM_REQ_WARNING_LV2:
						simEvent.warning = SIM_REQ_WARNING_LV2 ;
						SIM_DEBUG.print("WARNING_LEVEL_2" ) ;
						break;
					case SIM_REQ_WARNING_LV3:
						simEvent.warning = SIM_REQ_WARNING_LV3 ;
						SIM_DEBUG.print("WARNING_LEVEL_3" ) ;
						break;
					case SIM_REQ_WARNING_LV4:
						simEvent.warning = SIM_REQ_WARNING_LV4 ;
						SIM_DEBUG.print("WARNING_LEVEL_4" ) ;
						break;
					case SIM_REQ_WARNING_LV5:
						simEvent.warning = SIM_REQ_WARNING_LV5 ;
						SIM_DEBUG.print("WARNING_LEVEL_5" ) ;
						break;
					case SIM_REQ_STOP_WARNING:
						simEvent.warning = SIM_REQ_STOP_WARNING ;
						SIM_DEBUG.print("STOP_WARNING" ) ;
						break;

						// Các case về update trạng thái :
					case SIM_REQ_SMS_STATUS_UPDATE:
						simEvent.update = SIM_REQ_SMS_STATUS_UPDATE ;
						SIM_DEBUG.print("SMS STATUS QUERY" ) ;
						break;
					case SIM_REQ_AUDIO_UPDATE:
						simEvent.update = SIM_REQ_AUDIO_UPDATE ;
						SIM_DEBUG.print("AUDIO UPDATE" ) ;
						break;
					case SIM_REQ_ADD_PHONENUMBER:
						simEvent.update = SIM_REQ_ADD_PHONENUMBER ;
						SIM_DEBUG.print("ADD PHONE NUMBER") ;
						break;
					case SIM_REQ_REMOVE_PHONENUMBER:
						simEvent.update = SIM_REQ_REMOVE_PHONENUMBER ;
						SIM_DEBUG.print("REMOVE PHONE NUMBER") ;
						break;
					case SIM_REQ_FOTA:
						simEvent.update = SIM_REQ_FOTA ;
						SIM_DEBUG.print("FIRMWARE UPDATE OTA") ;
						break;

					default:
						break;
				}

			}else{} // Nếu không phải số điện thoại được cấp phép, thi bỏ qua, đỡ tốn thời gian !

			break ;


		/*------------------------------------------------------------------------
		 * CUỘC GỌI ĐẾN :
		 */
		case IS_INCOMMING_CALL:
			// Bỏ qua
			break;


		default:
			break;
	}

	// reset lại nội dung tin nhắn
//	memset(SMSContent, 0, RX_DATA_MAX) ;
	return simEvent ;

}


/*____________________________________________________________________________________________________________________________*/
int sim7x00::checkingRespond( const char* rpdMsg, uint32_t timeout){
	int answer=0;
	uint32_t tickStart = HAL_GetTick() ;
	while ( (uint32_t)( HAL_GetTick() - tickStart )  <=  timeout ) {
		if((checkIncommingEvent()==1)&&(strstr(readSMS(),rpdMsg)!=NULL)){
		answer =1;
		break;
		}
	}
	return answer;

}


/*____________________________________________________________________________________________________________________________*/
int sim7x00::checkIncommingEvent(){
	int result = IS_NOTHING ;

	// Kiểm tra có phải SMS đến ?
	char* isSMS = strstr(rxData, "SM") ;

	// Kiểm tra có phải cuộc gọi đến ?
	char* isPhoneCall = strstr(rxData, "RING") ;

	if( isSMS != NULL){
		SIM_DEBUG.println("Received SMS !") ;
		result = IS_SMS ;
	}
	if( isPhoneCall != NULL){
		SIM_DEBUG.println("Incomming call !") ;
		result = IS_INCOMMING_CALL ;
	}

	return result ;
}


/*____________________________________________________________________________________________________________________________*/
char* sim7x00::readSMS(){

	// Reset lại bộ nhớ SMS
	memset(SMSContent, 0, RX_DATA_MAX) ;

	// Selects the memory
	sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"\r", "OK", 3000);

	// Gửi lệnh đọc SMS mới nhất đó
	sendATcommand("AT+CMGR=0\r", "OK", 5000);

	// Copy nội dung lệnh từ bộ đệm đọc được sang biến SMSContent để xử lý
	strcpy(SMSContent, rxData) ;

	// debug nội dung SMS để kiểm tra
	SIM_DEBUG.println(SMSContent) ;

	// check xong rồi thì xóa tin nhắn đó đi
	sendATcommand("AT+CMGD=,4\r", "OK", 1000);

	return SMSContent;

}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::isAuthorizePhoneNumber( const char *_SMScontent ){
	SIM_DEBUG.println("Authorizing phone number ... ") ;

	bool result = 0 ;

	// Bắt đầu kiểm tra số điện thoại cho phép
	for(int i = 0 ; i < MAX_PHONE_NUMBER_ALLOWED ; i++){
		if(  strstr(_SMScontent, phoneNumbersAllowed[i]) != NULL  ){
			result = 1 ;
		}
	}

	if(result == 1)
		SIM_DEBUG.println("Phone number is accepted !" ) ;
	else
		SIM_DEBUG.println("Phone number is invalid !" ) ;

	return result ;
}


/*____________________________________________________________________________________________________________________________*/
int sim7x00::identifySMSRequest( const char *_SMScontent ){
	SIM_DEBUG.println("Identifying SMS request content ... ") ;
	int result = IDLE ;

	for(int i = 0 ; i < MAX_WARNING_CODES ; i++){

		// kiểm tra xem đã đúng cú pháp chưa
		if( strstr(_SMScontent, warningCodesAllowed[i]) != NULL ){
			if     (i == 0 ) result = WARNING_LEVEL_1 ;
			else if(i == 1 ) result = WARNING_LEVEL_2 ;
			else if(i == 2 ) result = WARNING_LEVEL_3 ;
			else if(i == 3 ) result = WARNING_LEVEL_4 ;
			else if(i == 4 ) result = WARNING_LEVEL_5 ;

		// kiểm tra xem có phải là stop không
		}else if( strstr(_SMScontent, stopWarningCodesAllowed) != NULL ){
			result = STOP_WARNING ;
		}else if( strstr(_SMScontent, "ok") != NULL ){
			result = OK ;
		}
		else{
			// do nothing
			// Ngoài ra còn nhiều chức năng ngoài cảnh báo, như là add thêm số điện thoại, hẹn giờ cảnh báo ... to be continue
		}
	}

	if(result == WARNING_LEVEL_5  )
		SIM_DEBUG.println("Datalogger was responded : " ) ;
	else if(result == STOP_WARNING  )
		SIM_DEBUG.println("Datalogger was responded : " ) ;
	else
		SIM_DEBUG.println("Datalogger was not respond !!!\n" ) ;

	// Ngoài ra còn nhiều chức năng ngoài cảnh báo, như là add thêm số điện thoại, hẹn giờ cảnh báo ... to be continue

	return result ;
}

/*____________________________________________________________________________________________________________________________*/
void sim7x00::MQTTstop() {
	sendATcommand("AT+CMQTTDISC=0,120\r", "CMQTTDISC: 0,0", 2000 ) ;

	// Release client first
	sendATcommand("AT+CMQTTREL=0\r", "OK", 5000 ) ;

	sendATcommand( "AT+CMQTTSTOP\r", "OK", 2000 ) ;
}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::MQTTpublish ( const char* _topic, const char* _payload, int _QoS, int _timeOut ) {

	if ( _timeOut < 1000 || _timeOut > 60000 ) _timeOut = 5000 ;
	// Set topic :
	int topic_Len = strlen ( _topic ) ;
	char cmdTopic [30] = {0} ;
	sprintf ( cmdTopic, "AT+CMQTTTOPIC=0,%d\r", topic_Len ) ;
	if ( sendATcommand ( cmdTopic, ">", 5000 ) == false ) return 0 ;

	if ( sendATcommand ( _topic, "OK", 2000 ) == false ) return 0 ;

	// Set payload :
	int payload_Len = strlen ( _payload ) ;
	char cmdPayload [30] = {0} ;
	sprintf ( cmdPayload, "AT+CMQTTPAYLOAD=0,%d\r", payload_Len ) ;
	if ( sendATcommand ( cmdPayload, ">", 5000 ) == false ) return 0 ;

	if ( sendATcommand ( _payload, "OK", 2000 ) == false ) return 0 ;

	// Publish lên broker
	char cmdPub [30] = {0} ;
	sprintf ( cmdPub, "AT+CMQTTPUB=0,%d,120\r", _QoS ) ;
	if ( sendATcommand ( cmdPub, "CMQTTPUB: 0,0", _timeOut ) == false ) return 0 ;

	return 1 ;
}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::MQTTlogin ( const char* _clientID,
				 	 	  const char* _willTopic,
					      const char* _willMessage,
						  const char* _brokerAddr,
						  const char* _port ) {

	SIM_DEBUG.println("Login mqtt... ") ;
	if ( sendATcommand ( "AT+CMQTTSTART\r", "CMQTTSTART: 0", 200 ) == false ) {
		MQTTstop() ;
		sendATcommand ( "AT+CMQTTSTART\r", "CMQTTSTART: 0", 200 ) ;
	}

	// Set client ID
	sendATcommand ( "AT+CMQTTREL=0\r", "OK", 200 ) ; // xóa client ID trước đó ở vị trí 0
	char cmdClient[50] = {0} ;
	sprintf ( cmdClient, "AT+CMQTTACCQ=0,\"%s\"\r", _clientID ) ;
	if ( sendATcommand ( cmdClient, "OK", 200 ) == false ) return 0  ;

	// Set will topic
	int willTopic_Len = strlen ( _willTopic ) ;
	char cmdWillTopic[30] = {0} ;
	sprintf ( cmdWillTopic, "AT+CMQTTWILLTOPIC=0,%d\r", willTopic_Len ) ;
	if ( sendATcommand ( cmdWillTopic, ">", 1000 ) == false ) return 0  ;
	if ( sendATcommand ( _willTopic, "OK", 3000 ) == false ) return 0  ;

	// Set will message và QoS = 2
	int willMsg_Len = strlen ( _willMessage ) ;
	char cmdWillMsg[30] = {0} ;
	sprintf ( cmdWillMsg, "AT+CMQTTWILLMSG=0,%d,2\r", willMsg_Len ) ;
	if ( sendATcommand ( cmdWillMsg, ">", 1000 ) == false ) return 0  ;
	if ( sendATcommand ( _willMessage, "OK", 1000 ) == false ) return 0  ;

	// Now connect to broker server :
	char cmdConnect[60] = {0} ;
	sprintf ( cmdConnect, "AT+CMQTTCONNECT=0,\"tcp://%s:%s\",60,1\r", _brokerAddr, _port ) ;
	if ( sendATcommand ( cmdConnect, "CMQTTCONNECT: 0,0", 30000 ) == false ) return 0  ;

	return true ;
}




/*____________________________________________________________________________________________________________________________*/
char* sim7x00::getCSQ() {

		static char result [3] = {0} ;
		sendATcommand( "AT\r", "OK", 100) ;

		// Bắt đầu lấy dữ liệu:
		if ( sendATcommand( "AT+CSQ\r" , "OK", 2000) == true ) {
			char data [ strlen( strstr( rxData, "+CSQ:") ) ] = {0} ;
			strcpy( data, strstr( rxData, "+CSQ:") ) ;

			char temp[3] = {0} ;
			for ( int i = 0; i < 3; i++ ) {
				if ( data[i+6] == ',' ){
					if ( i == 1 ) {
						strcat( result, "00") ;
						strcat( result, temp) ;
					}
					if ( i == 2 ) {
						strcat( result, "0") ;
						strcat( result, temp) ;
					}

					break ;
				}
				else temp [i] = data [i+6] ;
			}
		}
		return result ;
	}



/*____________________________________________________________________________________________________________________________*/
int sim7x00::getIntCSQ(){
	sendATcommand( "AT+CSQ\r" , "OK", 2000) ;

		static char csqData[3] ;
		memset ( csqData, 0 , 3 ) ;

		int len = strlen( rxData ) ;
		char rxDataCpy [len]  ;
		memset ( rxDataCpy, 0, len ) ;
		memcpy ( rxDataCpy, rxData, len ) ;

		for ( int i = 0; i < 2; i++ ) {
			csqData[i] = rxDataCpy[ i + 13 ] ;
		}
		return atoi(csqData);
}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::sendSMS(const char* PhoneNumber,const char* Message){

	bool answer = 0;
	char aux_string[30];

	SIM_DEBUG.println("Sending short message... ");

	sendATcommand("AT+CMGF=1\r", "OK", 1000);    // sets the SMS mode to text

	memset( aux_string, 0, 30 ) ;
	strcat( aux_string, "AT+CMGS=\"" ) ;
	strcat( aux_string, PhoneNumber ) ;
	strcat( aux_string, "\"\r" ) ;

	answer = sendATcommand(aux_string, ">", 3000);    // send the SMS number
	if (answer == 1){
		int SMSsize = strlen(Message) + 1 ;
		char msgPackage[SMSsize] ;
		char end[2] = { 0x1A, 0 } ;
		memset(msgPackage, 0, strlen(Message)+1 ) ;
		strcat( msgPackage, Message ) ;
		strcat( msgPackage, end ) ;
		answer = sendATcommand(msgPackage, "OK", 5000);
	}

	SIM_DEBUG.print("Done");
	return answer ;
}


/*____________________________________________________________________________________________________________________________*/
bool sim7x00::mqttUpdateData ( const char* _data) {
	if ( MQTTlogin ( MQTT_CLIENT_ID, MQTT_WILL_TOPIC, MQTT_WILL_MESSAGE, MQTT_BROKER_ADDR, MQTT_BROKER_PORT ) == false ) return 0 ;
	if ( MQTTpublish( MQTT_TOPIC, _data , 2, 2000 ) == false ) return 0 ;
	MQTTstop() ;
	return 1 ; //
}



/*____________________________________________________________________________________________________________________________*/
bool sim7x00::powerOff(){
	if( sendATcommand( "AT+CPOF\r", "OK", 1000 ) == true )
		return true ;
	else return false ;
}


#ifdef __cplusplus
}
#endif
