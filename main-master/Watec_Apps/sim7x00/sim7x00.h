/*
 * sim7600x.h
 *
 *  Created on: May 1, 2021
 *      Author: ASUS
 */

#ifndef SIM7X00_SIM7X00_H_
#define SIM7X00_SIM7X00_H_

enum sim7600_event_list {

	// WARNING : *giá trị list phải đồng bộ với lora

	// Trạng thái sau khi đã thực hiện xong các event yêu cầu
	NONE = 0 ,

	// Thực hiện cảnh báo theo các level :
	SIM_REQ_WARNING_LV1 = 1 ,
	SIM_REQ_WARNING_LV2 = 2 ,
	SIM_REQ_WARNING_LV3 = 3 ,
	SIM_REQ_WARNING_LV4 = 4 ,
	SIM_REQ_WARNING_LV5 = 5 ,

	// Tắt cảnh báo
	SIM_REQ_STOP_WARNING    = 6 ,

	/****************************************************/
	// UPDATE :

	// Yêu cầu cập nhật tình trạng qua SMS từ người dùng
	SIM_REQ_SMS_STATUS_UPDATE = 9 ,

	// Yêu cầu cập nhật lại tất cả file âm thanh
	SIM_REQ_AUDIO_UPDATE = 10,

	// Yêu cầu thêm số điện thoại
	SIM_REQ_ADD_PHONENUMBER = 11 ,

	// Yêu cầu xóa SĐT
	SIM_REQ_REMOVE_PHONENUMBER = 12,

	// Cập nhật firmware từ xa
	SIM_REQ_FOTA     = 13 ,


};



struct sim_events_t {
	int warning = NONE ;
	int update = NONE ;
};

/*
 * Hàm kiểm tra và xác định yêu cầu từ thông tin đến
 */
enum incomming_event {
	IS_NOTHING = 0,
	IS_SMS = 1,
	IS_INCOMMING_CALL = 2

};


#define RX_DATA_MAX 2000


// Tạo class để sử dụng cho module SIM ( kế thừa class debug để debug )
class sim7x00 {
public:
	// Cờ ngắt nhận data
	bool rxDone_FLAG = 0 ;

	// Byte đệm
	char rxBuff[1] ;

	// Dữ liệu nhận được từ module SIM
	char rxData[RX_DATA_MAX] ;
	int  dataCount = 0 ;

	// Dữ liệu nội dung tin nhắn SMS :
	char SMSContent[RX_DATA_MAX] ;


	sim7x00( UART_HandleTypeDef * _huartx ) ;

	void IRQhandler() ;

	// Hàm khởi tạo setup ban đầu cho module sim
	bool init(int timeout) ;

	// Hàm reset bộ nhớ đệm
	void memreset() ;

	// Kiểm tra sự kiện đến và xử lý yêu cầu
	sim_events_t identifyEvent() ;

	// kiểm tra datalogger có phản hồi về nút nhấn không:
	int checkingRespond(const char* msg,uint32_t timeout );

	// Kiểm tra sự kiện đang tới là cuộc gọi, tin nhắn ...
	int checkIncommingEvent() ;

	// Hàm Mở và đọc nội dung tin nhắn :
	char* readSMS() ;

	// Hàm xác minh số điện thoại có phải số được hệ thống cấp phép điều hành hay không
	bool isAuthorizePhoneNumber( const char *_SMScontent ) ;

	// Hàm xác thực nội dung yêu cầu từ tin nhắn
	int identifySMSRequest( const char *_SMScontent ) ;

	// ngắt kết nối server
	void MQTTStop();

	// kích hoạt MQTT và kết nối đến server
	bool MQTTStart (const char* MQTTClientId, const char* MQTTWillTopic, const char* MQTTWillMsg, const char*MQTTHost, const char* MQTTPort) ;

	// Lấy giá trị cường độ sóng lưu ở kiểu kí tự
	char* getCSQ();

	// Lấy giá trị cường độ sóng lưu ở kiểu số
	int getIntCSQ();

	// Đăng nhập vào broker
	bool MQTTlogin ( 	 const char* _clientID,
						 const char* _willTopic,
						 const char* _willMessage,
						 const char* _brokerAddr,
						 const char* _port ) ;

	// cập nhật dữ liệu lên topic
	bool MQTTpublish ( const char* _topic, const char* _payload, int _QoS, int _timeOut ) ;

	// Đăng xuất khỏi broker
	void MQTTstop() ;

	// Hàm toàn bộ quá trình đăng nhập, gửi dữ liệu và đăng xuất của 1 gói dữ liệu
	bool mqttUpdateData ( const char* _data) ;

	// Hàm gửi tin nhắn cảnh báo
	bool sendSMS(const char* PhoneNumber,const char* Message);

	bool powerOff() ;
private:

	// UART control module sim
	UART_HandleTypeDef *huartx ;
	// Gửi lệnh qua module sim
	bool sendATcommand (const char* ATcommand, const char* expected_answer, int timeout) ;

	bool sendATcommand (const char* ATcommand, const char* expected_answer, int timeout, int retryTime ) ;
	// Kiểm tra phản hồi từ module sim
	int response (const char* ATcommand, int timeout) ;

	char simData[30] ;

};


#endif /* SIM7X00_SIM7X00_H_ */
