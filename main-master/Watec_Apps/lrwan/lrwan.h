/*
 * lrwan.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef LRWAN_LRWAN_H_
#define LRWAN_LRWAN_H_

enum lrwan_event_list {

	// WARNING :
	// Trạng thái sau khi đã thực hiện xong các event yêu cầu
	NON = 0 ,

	// Thực hiện cảnh báo theo các level :
	LR_REQ_WARNING_LV1 = 1 ,
	LR_REQ_WARNING_LV2 = 2 ,
	LR_REQ_WARNING_LV3 = 3 ,
	LR_REQ_WARNING_LV4 = 4 ,
	LR_REQ_WARNING_LV5 = 5 ,

	// Tắt cảnh báo
	LR_REQ_STOP_WARNING    = 6 ,

	/****************************************************/
	// UPDATE :
	// Yêu cầu cập nhật tình trạng qua SMS từ người dùng
	LR_REQ_PERIOD_DATA_UPDATE = 7 ,


};

struct lr_events_t {
	int warning = NON ;
	int update = NON ;
};



// Tạo class để sử dụng cho module SIM ( kế thừa class debug để debug )
class lrwan {
public:

	// Cờ ngắt nhận data
	bool rxDone_FLAG = 0 ;

	// Byte đệm
	char rxBuff[1] ;

	uint8_t lrwanStatus;

	// Dữ liệu nhận được từ module SIM
	char rxData[RX_DATA_MAX] ;
	int  dataCount = 0 ;


	lrwan( UART_HandleTypeDef * _huartx ) ;

	void IRQhandler() ;

	void memreset() ;

	void init() ;


	// Gửi dữ liệu lên Server
	int checkConnect();

	bool sendData(const char* sms, const char* expectAnswer, int timeout, int retrytime);

	// Kiểm tra và xử lý sự kiện đến
	void checkingAndProcess() ;
	lr_events_t identifyEvent() ;

	void response (const char* response, int timeout) ;

private:

	// UART control module sim
	UART_HandleTypeDef *huartx ;


	bool sendCommand (const char* command, const char* expected_answer, int timeout) ;



	//
	char lrData[30];
};

#endif /* LRWAN_LRWAN_H_ */
