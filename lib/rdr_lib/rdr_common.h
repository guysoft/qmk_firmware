#pragma once

#include "quantum.h"
#include "usb_main.h"
#include "raw_hid.h"
#include "debounce.h"
#include "action_util.h"
#include "report.h"
#include "md_dma.h"

/************************IO 口**************************/
/************************IO 口**************************/
/************************IO 口**************************/
#define WHEEL_ZA_IO         (B2)
#define WHEEL_ZB_IO         (B10)

#define MODE_2P4G_IO        (B13)
#define MODE_BLE_IO         (B12)

#define ES_BATT_STDBY_IO    (A13)
#define ES_USB_POWER_IO     (C5)
#define ES_SPI_ACK_IO       (A4)
#define ES_PWM_DMA_IO       (A2)

#define ES_WUKEUP_IO        (D1)
#define ES_SDB_POWER_IO     (A3)
#define ES_LED_POWER_IO     (D0)

#define LVGL_RESET_IO       (A5)
#define LVGL_BOOT_IO        (A6)
#define LVGL_POWER_IO       (A8)

/************************SPI 命令**************************/
/************************SPI 命令**************************/
/************************SPI 命令**************************/
#define USER_EMI_COMMAND	    0XBB
#define USER_KEYBOARD_COMMAND	0X0A
#define USER_KEYBOARD_LENGTH    (64)

#define USER_SWITCH_2P4G_MODE	0X00
#define USER_SWITCH_BLE_1_MODE	0X01
#define USER_SWITCH_BLE_2_MODE	0X02
#define USER_SWITCH_BLE_3_MODE	0X03
#define USER_SWITCH_2P4G_PAIR	0X04
#define USER_SWITCH_BLE_1_PAIR	0X05
#define USER_SWITCH_BLE_2_PAIR	0X06
#define USER_SWITCH_BLE_3_PAIR	0X07
#define USER_SWITCH_USB_MODE	0X08

#define USER_KEYBOARD_SLEEP		0X09
#define USER_KEYBOARD_WAKEUP	0X0A

#define USER_KEY_BYTE_DATA		0X0B
#define USER_KEY_BIT_DATA		0X0C
#define USER_MOUSE_DATA			0X0D
#define USER_CONSUMER_DATA		0X0E
#define USER_SYSTEM_DATA		0X0F

#define USER_BATTERY_DATA		0X10

#define USER_GET_RF_STATUS	    0X11

#define USER_BLE1_WRITE_NAME	0X12
#define USER_BLE2_WRITE_NAME    0X13
#define USER_BLE3_WRITE_NAME    0X14

#define USER_SLEEP_TIME_WRITE   0X15       // 一级休眠时间
#define USER_DSLEEP_TIME_WRITE  0X16       // 二级休眠时间

#define USER_KEY_BYTE_LENGTH	0X08
#define USER_KEY_BIT_LENGTH		0X0F
#define USER_MOUSE_LENGTH		0X08
#define USER_CONSUMER_LENGTH	0X03
#define USER_SYSTEM_LENGTH		0X03
#define USER_BATTERY_LENGTH		0X02

#define KB_REPORT_ID            0x06    // Extend keyboard report ID.
#define SYS_REPORT_ID     	    0x03    // Extend System   report ID.
#define CON_REPORT_ID     	    0x04    // Extend Consumer report ID.
#define MOUSE_REPORT_ID  	    0x02    // Extend mouse	   report ID.

#define LOGO_LED_ENABLE         (0)
#define SIDE_LED_ENABLE         (0)

/************************核心定义**************************/
/************************核心定义**************************/
/************************核心定义**************************/
enum Custom_KeyModes {
    QMK_BLE_MODE = 0,
    QMK_2P4G_MODE,
    QMK_USB_MODE
};

enum Custom_BleChannels {
    QMK_BLE_CHANNEL_1 = 1,
    QMK_BLE_CHANNEL_2,
    QMK_BLE_CHANNEL_3
};

enum Custom_Spi_Ack_S {
    SPI_NACK,
    SPI_ACK
};

enum Custom_Spi_Busy_S {
    SPI_BUSY,
    SPI_IDLE
};

enum Custom_Ble_24G_Status_S {
    BLE_24G_NONE,
    BLE_24G_PIAR,
    BLE_24G_RETURN
};

typedef enum {
    KB_MODE_CONNECT_OK,  	        //连接成功
    KB_MODE_CONNECT_PAIR,	        //配对
    KB_MODE_CONNECT_RETURN,	        //回连
} keyboard_System_state_e;

typedef enum {
    USER_SLEEP_PASS,	            //休眠成功
    USER_SLEEP_FIAL,	            //休眠失败
} keyboard_System_Sleep_Status_s;

/************************默认工作模式**************************/
/************************默认工作模式**************************/
/************************默认工作模式**************************/
#define INIT_WORK_MODE              (QMK_USB_MODE)                                          // 默认工作模式
#define INIT_BLE_CHANNEL            (QMK_BLE_CHANNEL_1)                                     // 默认蓝牙通道
#define INIT_BATT_NUMBER            (50)                                                    // 上电的默认电池电量

#define INIT_SIX_KEY                (0)                                                     // 六键
#define INIT_ALL_KEY                (1)                                                     // 全键
#define INIT_ALL_SIX_KEY            (INIT_ALL_KEY)                                          // 全键

#define INIT_WIN_MODE               (0)                                                     // Windows
#define INIT_MAC_MODE               (1)                                                     // Mac
#define INIT_WIN_MAC_MODE           (INIT_WIN_MODE)                                         // Windows

#define INIT_WIN_NLOCK              (0)                                                     // 不锁WIN
#define INIT_WIN_LOCK               (1)                                                     // 锁WIN
#define INIT_WIN_LOCK_NLOCK         (INIT_WIN_NLOCK)                                        // 不锁WIN

#define INIT_LED_ON                 (0)                                                     // 开背光
#define INIT_LED_OFF                (1)                                                     // 不开背光
#define INIT_LED_ON_OFF             (INIT_LED_ON)                                           // 开背光

#define SLEEP_TIME_ONE              (60)                                                    // 休眠时间1分钟
#define SLEEP_TIME_TWO              (180)                                                   // 休眠时间3分钟
#define SLEEP_TIME_THREE            (600)                                                   // 休眠时间10分钟
#define SLEEP_TIME_FOUR             (1800)                                                  // 休眠时间30分钟
#define SLEEP_TIME_CLASS            (SLEEP_TIME_TWO)                                        // 默认休眠3分钟

#define USER_DSLEEP_TIME            0X147AE0                                                // 二级休眠时间 单位 S

#define DEBOUNCE_DELAY_ONE          (2)                                                     // 消抖等级1
#define DEBOUNCE_DELAY_TWO          (5)                                                     // 消抖等级2
#define DEBOUNCE_DELAY_CLASS        (DEBOUNCE_DELAY_TWO)                                    // 默认消抖等级

#define USER_DEFINE_KEY             (QK_KB)

enum Custom_Keycodes {
    QMK_KB_MODE_2P4G = USER_DEFINE_KEY,
    QMK_KB_MODE_BLE1,
    QMK_KB_MODE_BLE2,
    QMK_KB_MODE_BLE3,
    QMK_KB_MODE_USB,
    QMK_BATT_NUM,
    QMK_WIN_LOCK,
    QMK_KB_SIX_N_CH,
    QMK_TEST_COLOUR,
	IOS_SIRI,
    RGB_RTOG,
    U_EE_CLR,
    QMK_DEBOUNCE,
    QMK_TIME_SET,
    QMK_DTIME_SET,
    HOME_MODE,
    KEY_MODE,
    GIF_MODE,
    LOOP_MODE,
    GIF_CHAGE,
    LVGL_SHUTDOWN,
    LVGL_BOOT,
    LVGL_RESET_BOOT,
    LVGL_OPEN,
#if LOGO_LED_ENABLE
    LOGO_TOG,
    LOGO_MOD,
    LOGO_RMOD,
    LOGO_HUI,
    LOGO_HUD,
    LOGO_SAI,
    LOGO_SAD,
    LOGO_VAI,
    LOGO_VAD,
    LOGO_SPI,
    LOGO_SPD,
#endif
#if SIDE_LED_ENABLE
    SIDE_TOG,
    SIDE_MOD,
    SIDE_RMOD,
    SIDE_HUI,
    SIDE_HUD,
    SIDE_SAI,
    SIDE_SAD,
    SIDE_VAI,
    SIDE_VAD,
    SIDE_SPI,
    SIDE_SPD,
#endif
    QMK_KB_2P4G_PAIR,
    QMK_KB_BLE1_PAIR,
    QMK_KB_BLE2_PAIR,
    QMK_KB_BLE3_PAIR,
    LVGL_NEXT,
    LVGL_PREV,
    LVGL_ENT
};

#define WIN_COL                     (1)
#define WIN_ROL                     (3)

#define MAC_COL                     (2)
#define MAC_ROL                     (3)

#define KC_K29 	                    KC_BACKSLASH
#define KC_K42 	                    KC_NONUS_HASH
#define KC_K45 	                    KC_NONUS_BACKSLASH
#define KC_K56 	                    KC_INTERNATIONAL_1
#define KC_K14                      KC_INTERNATIONAL_3
#define KC_K132	                    KC_INTERNATIONAL_4
#define KC_K131	                    KC_INTERNATIONAL_5
#define KC_K133	                    KC_INTERNATIONAL_2
#define KC_K151	                    KC_LANGUAGE_1
#define KC_K150	                    KC_LANGUAGE_2

#define MD_24G	                    QMK_KB_MODE_2P4G
#define MD_BLE1	                    QMK_KB_MODE_BLE1
#define MD_BLE2	                    QMK_KB_MODE_BLE2
#define MD_BLE3	                    QMK_KB_MODE_BLE3
#define MD_USB	                    QMK_KB_MODE_USB
#define QK_BAT                      QMK_BATT_NUM
#define QK_WLO	                    QMK_WIN_LOCK
#define SIX_N	                    QMK_KB_SIX_N_CH
#define TEST_CL	                    QMK_TEST_COLOUR
#define KEY_DEB	                    QMK_DEBOUNCE
#define TIME_ST	                    QMK_TIME_SET
#define TIME_DT	                    QMK_DTIME_SET
#define L_HOME	                    HOME_MODE
#define L_KEY	                    KEY_MODE
#define L_GIF	                    GIF_MODE
#define L_LOOP                      LOOP_MODE
#define L_GIFCH	                    GIF_CHAGE
#define L_SHUT                      LVGL_SHUTDOWN
#define L_BOOT                      LVGL_BOOT
#define L_RE_BO                     LVGL_RESET_BOOT
#define L_NEXT	                    LVGL_NEXT
#define L_PREV	                    LVGL_PREV
#define L_ENT	                    LVGL_ENT
#define L_OPEN	                    LVGL_OPEN

/************************基本变量**************************/
/************************基本变量**************************/
/************************基本变量**************************/
typedef struct {
    uint8_t Key_Mode;               // 键盘工作模式
    uint8_t Ble_Channel;            // 蓝牙通道
    uint8_t Batt_Number;            // 电池电量
    uint8_t Nkro;                   // 六键全键无冲
    uint8_t Mac_Win_Mode;           // MAC系统WIN系统
    uint8_t Win_Lock;               // 锁WIN
	uint8_t Key_Change_Func;        // 切换F区和数字键区
    uint8_t Led_On_Off;             // 背光开关
    uint8_t Debounce_Delay;         // 按键消抖
    uint32_t User_Sleep_Time;       // 一级休眠
    uint32_t User_DSleep_Time;      // 二级休眠

    uint8_t Gif_Count_Init;         // 屏幕GIF动画同步
    uint8_t User_Info_Lcd_Mode;     // 屏幕工作模式
    uint8_t User_Info_Lcd_Power;    // 默认打开屏幕
    uint8_t User_Info_Lcd_Language; // 默认语言
    uint8_t User_Info_Lcd_Gif;      // 默认GIF图片
    uint8_t User_Info_Lcd_Usb;      // USB枚举
#if LOGO_LED_ENABLE
    uint8_t Logo_On_Off;            // LOGO灯光开关
    uint8_t Logo_Mode;              // LOGO灯光模式
    uint8_t Logo_Colour;            // LOGO灯光颜色
    uint8_t Logo_Saturation;        // LOGO灯光饱和度
    uint8_t Logo_Brightness;        // LOGO灯光亮度
    uint8_t Logo_Speed;             // LOGO灯光速度
#endif
#if SIDE_LED_ENABLE
    uint8_t Side_On_Off;            // 测灯灯光开关
    uint8_t Side_Mode;              // 测灯灯光模式
    uint8_t Side_Colour;            // 测灯灯光颜色
    uint8_t Side_Saturation;        // 测灯灯光饱和度
    uint8_t Side_Brightness;        // 测灯灯光亮度
    uint8_t Side_Speed;             // 测灯灯光速度
#endif
} Keyboard_Info_t;
extern Keyboard_Info_t Keyboard_Info;

typedef struct {
    uint8_t System_Work_Status;     // 系统状态
    uint8_t System_Work_Mode;       // 工作模式
    uint8_t System_Work_Channel;    // 工作通道
    uint8_t System_Connect_Status;  // 连接状态
    uint8_t System_Led_Status;      // 系统指示灯
    uint8_t System_Sleep_Mode;      // 系统休眠
} Keyboard_Status_t;
extern Keyboard_Status_t Keyboard_Status;

bool     Key_2p4g_Status;
bool     Key_Ble_1_Status;
bool     Key_Ble_2_Status;
bool     Key_Ble_3_Status;
bool     Key_Fn_Status;

uint8_t  Systick_6ms_Count;
uint8_t  Systick_10ms_Count;
uint8_t  Systick_12ms_Count;
bool     Scan_Switch_Ok;
uint16_t Systick_Interval_Count;
uint16_t Time_3s_Count;
uint8_t  Key_Switch_Scan;
uint8_t  Key_Switch_Check;
uint8_t  Key_Switch_Old;
uint8_t  Key_Switch_delay;
uint8_t  Mac_Win_Point_Count;

uint8_t  INIT_ALL_SIX_KEY_Count;
uint8_t  INIT_ALL_KEY_Count;
bool     Last_Caps_State;
bool     Last_WinL_State;

bool     Test_Led;
uint8_t  Test_Colour;

/**********************中断发送变量************************/
/**********************中断发送变量************************/
/**********************中断发送变量************************/
#define UART_BUF_SIZE       8
#define UART_PACK_SIZE      4
#define UART_POWER_DELAY    20

#define USER_UART_COMMAND1  0x11
#define USER_UART_COMMAND2  0x22
#define USER_UART_COMMAND3  0x33
#define USER_UART_COMMAND4  0x44

uint8_t  Uart_Tx_Buffer[UART_BUF_SIZE];
uint8_t  Uart_Rx_Buffer[UART_BUF_SIZE];
volatile uint8_t Dma1_Interrupt_Tx_Down;
volatile uint8_t Dma1_Interrupt_Rx_Down;
bool     Uart_Reset;
bool     Uart_Tx_Ping;

bool     Uart_Power_Up;
uint8_t  Uart_Power_Up_Start;
uint8_t  Uart_Msg_Count;
uint8_t  Uart_Msg_Count_Repet;
uint16_t Uart_Lvgl_200ms_Delay;

bool     Uart_Lvgl_Scan_OK;

uint8_t  Uart_Work_Mode;
uint8_t  Uart_Work_Mode_Again;

uint8_t  Led_Data_Scan;
uint8_t  Uart_Led_Data;
uint8_t  Uart_Led_Again;

uint8_t  Uart_Batt_Charge;
uint8_t  Uart_Batt_Num;
uint8_t  Uart_Batt_Again;

uint8_t  Uart_Usb_Data;
uint8_t  Uart_Usb_Data_Again;

uint8_t  Uart_RF_Mode_Data;
uint8_t  Uart_RF_Status_Data;
uint8_t  Uart_RF_Channl_Data;
uint8_t  Uart_Rf_Systick;
uint8_t  Uart_Rf_Again;

uint8_t  Uart_Gif_Data;
uint8_t  Uart_Gif_Data_Again;

uint8_t  Uart_Test_Data;

uint8_t  Uart_Home_Mov_Layer;
uint8_t  Uart_Home_Mov_Layer_Switch;
uint8_t  Uart_Home_Mov_Layer_Old_Switch;

uint8_t  Uart_Keycode_Data;
uint8_t  Uart_Key_Count;
uint8_t  Uart_Usb_Change_Delay_200ms;

uint8_t  First_Count;
uint8_t  First_Connect_OK_Count;

const md_uart_init_typedef UART_InitStruct;
md_dma_init_typedef Dma_InitStruct_Tx;
md_dma_init_typedef Dma_InitStruct_Rx;
const uint32_t g_es_dma_ch0_cfg;
const uint32_t g_es_dma_ch1_cfg;

void Uart_Tx_Dma_Start(void);
void Uart_Rx_Dma_Start(void);
void Uart_Dma_Init(void);
void User_Uart_Deinit(void);
void Init_Uart_Information(void);
uint8_t User_Uart_Send_Data(uint8_t Command, uint8_t Busy);
void Uart_While_Send_Commad(uint8_t Command, uint8_t Busy);
void Reset_Uart(void);
void Lvgl_Init(void);
void Reset_Power_Lvgl(void);
void Scan_Uart_Rf(void);
void Scan_Uart_Work_Mode(void);
void Scan_Uart_Func(void);
void Scan_Uart_Batt(void);
void Scan_Uart_Gif(void);
uint8_t Get_Lcd_Usb_Work_Status(void);
void Lcd_Usb_Work_Init(uint8_t Reset, uint8_t Set);
void Scan_Lcd_Usb_Work_Mode(void);
void Uart_Key_Mode_Key_Scan(uint16_t Code, bool Press);
void Lcd_Work_Sync_GIF(uint8_t Gif_Count);
void App_Uart_Main_Loop(void);

/*******************自定义休眠相关变量*********************/
/*******************自定义休眠相关变量*********************/
/*******************自定义休眠相关变量*********************/
uint8_t  User_Sleep_Timer_Count;
bool     User_Sleep_Time_Send;
bool     User_DSleep_Time_Send;

void Sleep_Time_Synchronization(void);
void DSleep_Time_Synchronization(void);

/************************数据队列**************************/
/************************数据队列**************************/
/************************数据队列**************************/
#define APP_2G4_BUF_SIZE            (USER_KEYBOARD_LENGTH)
#define APP_2G4_BUF_CNT             (20)

uint8_t app_2g4_data[APP_2G4_BUF_CNT][APP_2G4_BUF_SIZE];
volatile uint8_t app_2g4_data_send;
volatile uint8_t app_2g4_data_rev;

uint8_t app_2g4_buffer_full(void);
uint8_t app_2g4_buffer_empty(void);
void app_2g4_buffer_rev_add(void);
void app_2g4_buffer_send_add(void);

/**************************EMI****************************/
/**************************EMI****************************/
/**************************EMI****************************/
bool Emi_Test_Start;
void Emi_Init(void);
void Emi_Read_Data(uint8_t *User_Data, uint8_t User_Length);
void Emi_Write_Data(uint8_t *User_Data, uint8_t User_Length);

/**************************SPI****************************/
/**************************SPI****************************/
/**************************SPI****************************/
#define SPI_DELAY_RF_TIME           (100)
#define SPI_DELAY_USB_TIME          (500 * 3)

#define MAX_NAME_LEN                (18)
#define USER_BlE_ID                 (0X008A)
#define USER_BlE1_NAME              "RT82-1"
#define USER_BlE2_NAME              "RT82-2"
#define USER_BlE3_NAME              "RT82-3"

volatile uint8_t Spi_Send_Recv_Flg;
uint16_t Spi_Interval;
uint8_t  g_es_spi_rx_buf[64];
uint8_t  g_es_spi_tx_buf[64];
uint8_t  Repet_Send_Count;
uint8_t  Send_Key_Type;
uint8_t  Spi_Main_Loop_count;
bool     Init_Spi_Power_Up;
uint8_t  Init_Spi_100ms_Delay;
bool     Ble_Name_Spi_Send;
uint8_t  Ble_Name_Spi_Count;

const uint32_t g_es_dma_ch2pri_cfg;
const uint32_t g_es_dma_ch2alt_cfg;
const md_spi_inittypedef SPI2_InitStruct;

void    es_ble_spi_init(void);
void    es_ble_spi_deinit(void);
void    es_spi_send_recv_by_dma(uint32_t num, uint8_t *rx_buf, uint8_t *tx_buf);
void    Spi_Main_Loop(void);
void    Spi_Send_Commad(uint8_t Commad);
uint8_t Spi_Ack_Send_Commad(uint8_t Commad);
void    Get_Spi_Return_Data(uint8_t *Data);

/**************************六键全键转换****************************/
/**************************六键全键转换****************************/
/**************************六键全键转换****************************/
void User_send_6kro_report(void) ;
void User_send_nkro_report(void);
void User_Clear_Board(void);
void add_key_to_report(uint8_t key);
void General_Key_Reorder(uint8_t Spot_Index);
void del_key_from_report(uint8_t key);

/**************************模式切换****************************/
/**************************模式切换****************************/
/**************************模式切换****************************/
volatile host_driver_t *es_qmk_driver;
const    host_driver_t es_user_driver;

uint8_t es_keyboard_leds(void);
void es_send_keyboard(report_keyboard_t *report);
void es_send_nkro(report_nkro_t *report);
void es_send_mouse(report_mouse_t *report);
void es_send_extra(report_extra_t *report);
void Mode_Synchronization(void);
void Ble_Name_Synchronization(void);
void User_bluetooth_send_keyboard(uint8_t *report, uint32_t len);

/************************键盘恢复初始化变量********************/
/************************键盘恢复初始化变量********************/
/************************键盘恢复初始化变量********************/
uint16_t Time_3s_EE_CLR_Count;
bool     User_QMK_EE_CLR_Flag;
bool     User_EE_CLR_Start_Flag;

/**************************按键消抖****************************/
/**************************按键消抖****************************/
/**************************按键消抖****************************/
unsigned int Debounce_Delay;        //键盘消抖次数，最大为127
uint8_t  Debounce_Point_Count;
uint16_t User_Key_3s_Count;
bool     Debounce_Function_Count;
bool     Debounce_Function_Status;

/**************************系统函数****************************/
/**************************系统函数****************************/
/**************************系统函数****************************/
#define KEYBAORD_COL                (16)
#define KEYBAORD_ROL                (6)

#define MATRIX_USER_COL_PINS        { D15, D14, C15, C14, C13, D3, D2, C12, C11, C10, A14, C9, C8, C7, C6, B15 }
#define MATRIX_USER_ROW_PINS        { B0, B3, B4, B5, B6, B7 }

bool     Save_Flash;
bool     Reset_Save_Flash;
uint16_t Save_Flash_3S_Count;
bool     Led_Rf_Pair_Flg;
bool     Usb_Change_Mode_Wakeup;
uint8_t  Temp_System_Led_Status;
bool     Mode_Synchronization_Signal;
uint16_t g_usb_sof_frame_id;
uint16_t g_usb_sof_frame_id_last;
bool     Usb_Dis_Connect;
uint16_t Usb_Suspend_Delay;
uint16_t Usb_Change_Mode_Delay;

void es_mcu_reset(void);
void bootloader_jump(void);
void mcu_reset(void);
void User_Keyboard_Reset(void); // 封装
void Save_Flash_Set(void);
void User_Systime_Init(void);
void User_Systime_Deinit(void);
void Init_Gpio_Infomation(void);
void User_Sleep(void);
void User_Wakeup(void);
void Board_Wakeup_Init(void);
void es_chibios_user_idle_loop_hook(void);
void Init_Keyboard_Infomation(void);
void es_change_qmk_nkro_mode_enable(void);
void es_change_qmk_nkro_mode_disable(void);
void User_Keyboard_Init(void); // 封装
void User_Keyboard_Post_Init(void); // 封装
bool User_process_record_user(uint16_t keycode, keyrecord_t *record);
extern int usbd_ep_start_write(const uint8_t ep, const uint8_t *data, uint32_t data_len);

/************************拨动开关**************************/
/************************拨动开关**************************/
/************************拨动开关**************************/
void Key_Switch_Mode_Scan(void);
void Key_Switch_Mode_Power(void);

/************************USB 插件**************************/
/************************USB 插件**************************/
/************************USB 插件**************************/
void User_Usb_Init(void);
void es_restart_usb_driver(void);
void Usb_Disconnect(void);
void User_Usb_Deinit(void);

/**************************FLASH****************************/
/**************************FLASH****************************/
/**************************FLASH****************************/
void eeprom_driver_init(void);
void eeprom_write_block_user(const void *buf, void *addr, size_t len);
void eeprom_read_block_user(void *buf, const void *addr, size_t len);

/*************************电池******************************/
/*************************电池******************************/
/*************************电池******************************/
#define USER_BATT_POWER_SCAN_COUNT  (10)
#define USER_BATT_SCAN_COUNT        (10)

#define USER_BATT_HIGH_POWER        (2555)      //满电 2565 * 3.3 /4096 = 2.066 4.13V     实际电路存在压降。
#define USER_BATT_LOW_POWER         (2090)      //低电 2065 * 3.3 /4096 = 1.663 3.32V     即使键盘不开灯，电池满电4.2V
#define USER_BATT_STDOWN_POWER      (1865)      //关机 1865 * 3.3 /4096 = 1.502 3.04V     输入到板子也就只有4.1V左右

#define USER_BATT_DELAY_TIME        (100 * 25)  //25S
#define USER_TIME_3S_TIME           (300)       //3S
#define USER_TIME_2S_TIME           (200)       //2S
#define USER_KEY_SHORT_LONG         (50)        //500mS

uint16_t User_Adc_Batt[USER_BATT_SCAN_COUNT];
uint16_t User_Scan_Batt[USER_BATT_SCAN_COUNT];
uint8_t  User_Adc_Batt_Count;
uint8_t  User_Batt_BaiFen;
uint8_t  User_Batt_Old_BaiFen;
uint8_t  User_Batt_10ms_Count;
uint16_t User_Batt_Time_15S_Count;
bool     User_Batt_Power_Up;
bool     User_Batt_Send_Spi;
uint16_t User_Batt_Power_Up_Delay_100ms_Count;
bool     User_Batt_Power_Up_Delay;
bool     User_Power_Low;
uint8_t  User_Power_Low_Count;
uint8_t  es_stdby_pin_state;
bool     User_Key_Batt_Num_Show;
uint8_t  User_Key_Batt_Count;
uint8_t  Batt_Led_Count;

const md_adc_initial adc_initStruct;

void Init_Batt_Infomation(void);
void User_Adc_Init(void);
void User_Adc_Deinit(void);
void U16_Buff_Clear(uint16_t *Buff, uint8_t Len);
void User_Adc_Batt_Power_Up_Init(void);
void User_Adc_Batt_Number(void);

/************************主控灯光**************************/
/************************主控灯光**************************/
/************************主控灯光**************************/
#define ES_PWM_LED_SIZE         (42)
#define ES_PWM_LED_BYTE         (24)
#define ES_PWM_DMA_SIZE         (ES_PWM_LED_SIZE * ES_PWM_LED_BYTE)

#define ES_PWM_WS2812_H_VALUE   (43)
#define ES_PWM_WS2812_L_VALUE   (17)

#define U_PWM                   (RGB_MATRIX_MAXIMUM_BRIGHTNESS)                         // 亮度

#define LED_LOW_P_INDEX         (77)
#define LED_BAT_INDEX           (77)
#define LED_CAP_INDEX           (44)
#define LED_WIN_L_INDEX         (73)
#define LED_KB_SIX_N_INDEX      (64)
#define LED_DEB_INDEX           (50)

#define LED_BLE_1_INDEX         (30)
#define LED_BLE_2_INDEX         (31)
#define LED_BLE_3_INDEX         (32)
#define LED_2P4G_INDEX          (33)
#define LED_USB_INDEX           (34)

uint8_t Led_Colour_Tab[9][3];
uint8_t Led_Wave_Pwm_Tab[128];
uint8_t Led_Batt_Index_Tab[10];
uint8_t Led_Point_buf[2];

uint8_t  Systick_Led_Count;
uint8_t  Led_Point_Count;
uint16_t Usb_If_Ok_Delay;
bool     Led_Flash_Busy;
bool     Led_Off_Start;
bool     Led_Power_Up;
uint16_t Led_Power_Up_Delay;
bool     Usb_If_Ok_Led;
bool     Usb_If_Ok;
bool     Led_Point_Sleep;

rgb_led_t rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];
uint8_t g_es_pwm_rgb_matrix_array_dma_buf[(RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 2];
md_dma_channel_config_typedef DMA_list[5];

void rgb_matrix_driver_init(void);
void User_Pwm_Deinit(void);
void rgb_matrix_driver_flush_pwm_dma_start(void);
void rgb_matrix_driver_flush(void);
void rgb_matrix_driver_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void rgb_matrix_driver_set_color_all(uint8_t r, uint8_t g, uint8_t b);
const rgb_matrix_driver_t rgb_matrix_driver;

void Led_Power_Low_Show(void);
void Led_Rf_Mode_Show(void);
void Led_Batt_Number_Show(void);
void Led_Point_Flash_Show(void);
void User_Led_Show(void); // 封装
void User_Get_Led_Power_Status(void);
void User_Test_Colour_Show(void);

/***************************屏幕*****************************/
/***************************屏幕*****************************/
/***************************屏幕*****************************/
#define UART_NUM_LOCK       0X001
#define UART_CAP_LOCK       0X002
#define UART_SCR_LOCK       0X004
#define UART_WIN_LOCK       0X008
#define UART_MAC_WIN        0X010

typedef struct {
    uint8_t  Home_Mov_Layer;         //当前逻辑层
    uint8_t  Home_Mov_Layer1_Switch; //逻辑层1选择界面
    uint8_t  Home_Mov_Layer2_Switch; //逻辑层2选择界面
    uint8_t  Home_Mov_Layer3_Switch; //逻辑层3选择界面
    uint8_t  Home_Mov_Gif_Flg;       //逻辑层Gif显示
    uint8_t  Home_Mov_Flag;          //逻辑层界面开始显示
    uint16_t Home_Mov_3S_Count;      //逻辑层界面显示3S之后不动则返回主界面
} Lvgl_Home_Mov_Type;

enum Custom_Uart_Busy_S {
    UART_IDLE,
    UART_BUSY
};

enum Custom_Uart_Modes {
    Uart_Home = 0,
    Uart_Key,
    Uart_Gif,
    Uart_Test
};

enum Language_Mode {
	English = 0,
	Chinese,
};

enum Home_Mov_Layer_E {
    Home_Layer = 0,
    Func_Layer,
    Enter_Layer,
    Lcd_Action_OK
};

enum Home_Layer_E {
    Home_Default = 0,
    Home_Vold,
    Home_Volu
};

enum Func_Layer_E {
    Func_Mode_Change = 0,
    Func_Mode_MW
};

enum Enter_Layer_Mode_Change_E {
    Enter_Mode_Usb = 0,
    Enter_Mode_2P4G,
    Enter_Mode_Ble1,
    Enter_Mode_Ble2,
    Enter_Mode_Ble3,
    Enter_Mode_Win,
    Enter_Mode_Mac
};

enum Custom_Gif_Mode {
    GIF_MODE_1 = 0,
	GIF_MODE_2,
	GIF_MODE_3
};

enum Custom_Uart_Command {
    Command_Get_Error = 0X55,
    Command_Get_Status = 0XAA,
    Command_Ready_Ok = 0XBB,
    Command_Work_Mode_Data = 0XC0,
    Command_Led_Data = 0XC1,
    Command_Batt_Data = 0XC2,
    Command_Mode_Data = 0XC3,
    Command_Key_Mode_Data = 0XC4,
    Command_Home_Mov_Data = 0XC5,
    Command_Gif_Change_Data = 0XC6,
    Command_Test_Colour = 0XC7,
    Command_Usb_Data = 0XC8,
	Command_USB_DISABLE = 0XDD,
};

/**********************************/
/**********************************/
#define INIT_LCD_MODE       (Uart_Home)    // 默认屏幕工作模式

#define LCD_POWER_ON        (0)            // 打开屏幕
#define LCD_POWER_OFF       (1)            // 关闭屏幕
#define INIT_LCD_POWER      (LCD_POWER_ON) // 默认打开屏幕

#define LCD_USB_ON          (1)            // 枚举USB
#define LCD_USB_OFF         (0)            // 关闭USB
#define INIT_USB_POWER      (LCD_USB_OFF)  // 默认关闭USB

#define INIT_LCD_LANGUAGE   (English)      // 默认语言

#define GIF_PIC_MAX         3              // 最大张数

uint8_t Get_Lcd_Usb_Work_Status(void);
void Lcd_Usb_Work_Init(uint8_t Reset, uint8_t Set);
void Lcd_Work_Sync_GIF(uint8_t Gif_Count);
void Reset_Power_Lvgl(void);

/************************LOGO灯灯光**************************/
/************************LOGO灯灯光**************************/
/************************LOGO灯灯光**************************/
#if LOGO_LED_ENABLE
#define LOGO_LED_PLAY_SPEED	        (0)                                                     // 灯光刷新速度
#define LOGO_LED_SIZE	            (6)                                                     // 灯光数量

#define LOGO_LED_ON                 (0)                                                     // 灯光打开
#define LOGO_LED_OFF                (1)                                                     // 灯光关闭

#define LOGO_WAVE_RGB_MODE          (1)                                                     // 彩色波浪
#define LOGO_WAVE_DS_MODE           (2)                                                     // 单色波浪
#define LOGO_SPECTRUM_MODE          (3)                                                     // 光谱
#define LOGO_BREATH_MODE            (4)                                                     // 单色呼吸
#define LOGO_LIGHT_MODE             (5)                                                     // 单色常量
#define LOGO_OFF_MODE               (6)                                                     // 关闭

#define LOGO_MAX_COLOUR             (255)                                                   // 颜色最大
#define LOGO_MIN_COLOUR             (0)                                                     // 颜色最小
#define COLOUR_LEVEL                (15)                                                    // 颜色等级

#define LOGO_MAX_SATURATION         (0)                                                     // 饱和度最大
#define LOGO_MIN_SATURATION         (255)                                                   // 饱和度最小
#define SATURATION_LEVEL            (15)                                                    // 饱和度等级

#define LOGO_MAX_BRIGHTNESS         (RGB_MATRIX_MAXIMUM_BRIGHTNESS)                         // 亮度最大
#define LOGO_MIN_BRIGHTNESS         (0)                                                     // 亮度最小
#define BRIGHTNESS_LEVEL            (15)                                                    // 亮度等级

#define LOGO_MAX_SPEED              (4)                                                     // 速度最大
#define LOGO_MIN_SPEED              (0)                                                     // 速度最小
#define SPEED_LEVEL                 (1)                                                     // 速度等级

#define INIT_LOGO_ON_OFF            (LOGO_LED_ON)                                           // 灯光打开
#define INIT_LOGO_MODE              (LOGO_WAVE_RGB_MODE)                                    // 彩色波浪
#define INIT_LOGO_COLOUR            (LOGO_MIN_COLOUR)                                       // 颜色最小
#define INIT_LOGO_SATURATION        (LOGO_MAX_SATURATION)                                   // 饱和度最大
#define INIT_LOGO_BRIGHTNESS        (LOGO_MAX_BRIGHTNESS)                                   // 亮度最大
#define INIT_LOGO_SPEED             (2)                                                     // 速度居中

void User_Via_Qmk_Logo_Get_Value(uint8_t *data);
void User_Via_Qmk_Logo_Set_Value(uint8_t *data);
void User_Via_Qmk_Logo_Command(uint8_t *data, uint8_t length);

#endif

/**************************侧灯灯光****************************/
/**************************侧灯灯光****************************/
/**************************侧灯灯光****************************/
#if SIDE_LED_ENABLE
#define SIDE_LED_PLAY_SPEED	        (0)                                                     // 灯光刷新速度
#define SIDE_LED_SIZE	            (30)                                                    // 灯光数量

#define SIDE_LED_ON                 (0)                                                     // 灯光打开
#define SIDE_LED_OFF                (1)                                                     // 灯光关闭

#define SIDE_WAVE_RGB_MODE          (1)                                                     // 彩色波浪
#define SIDE_WAVE_DS_MODE           (2)                                                     // 单色波浪
#define SIDE_SPECTRUM_MODE          (3)                                                     // 光谱
#define SIDE_BREATH_MODE            (4)                                                     // 单色呼吸
#define SIDE_LIGHT_MODE             (5)                                                     // 单色常量
#define SIDE_OFF_MODE               (6)                                                     // 关闭

#define SIDE_MAX_COLOUR             (255)                                                   // 颜色最大
#define SIDE_MIN_COLOUR             (0)                                                     // 颜色最小
#define SIDE_COLOUR_LEVEL           (15)                                                    // 颜色等级

#define SIDE_MAX_SATURATION         (0)                                                     // 饱和度最大
#define SIDE_MIN_SATURATION         (255)                                                   // 饱和度最小
#define SIDE_SATURATION_LEVEL       (15)                                                    // 饱和度等级

#define SIDE_MAX_BRIGHTNESS         (RGB_MATRIX_MAXIMUM_BRIGHTNESS)                         // 亮度最大
#define SIDE_MIN_BRIGHTNESS         (0)                                                     // 亮度最小
#define SIDE_BRIGHTNESS_LEVEL       (15)                                                    // 亮度等级

#define SIDE_MAX_SPEED              (4)                                                     // 速度最大
#define SIDE_MIN_SPEED              (0)                                                     // 速度最小
#define SIDE_SPEED_LEVEL            (1)                                                     // 速度等级

#define INIT_SIDE_ON_OFF            (SIDE_LED_ON)                                           // 灯光打开
#define INIT_SIDE_MODE              (SIDE_WAVE_RGB_MODE)                                    // 彩色波浪
#define INIT_SIDE_COLOUR            (SIDE_MIN_COLOUR)                                       // 颜色最小
#define INIT_SIDE_SATURATION        (SIDE_MAX_SATURATION)                                   // 饱和度最大
#define INIT_SIDE_BRIGHTNESS        (SIDE_MAX_BRIGHTNESS)                                   // 亮度最大
#define INIT_SIDE_SPEED             (2)                                                     // 速度居中

void User_Via_Qmk_Side_Get_Value(uint8_t *data);
void User_Via_Qmk_Side_Set_Value(uint8_t *data);
void User_Via_Qmk_Side_Command(uint8_t *data, uint8_t length);

#endif


