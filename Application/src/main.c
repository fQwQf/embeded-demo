#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "delay.h"
#include "u1.h"
#include "e1.h"
#include "e2.h"
#include "s1.h"
#include "s2.h"
#include "s5.h"
#include "s7.h"

// ================== 全局宏定义 ==================
#define LOOP_DELAY_MS 100
#define FAN_SPEED_LOW 20
#define FAN_SPEED_MEDIUM 40
#define FAN_SPEED_HIGH 60
#define TAP_THRESHOLD_G 1.5f
// #define LOW_LIGHT_THRESHOLD 150 // 定义光照阈值 (单位: Lux)

// --- NFC卡片唯一ID (UID) ---
unsigned char study_card_uid[4] = {0xFE, 0x1F, 0x7F, 0xC2};
unsigned char deep_work_card_uid[4] = {0xB3, 0x35, 0x5E, 0xAC};
unsigned char data_card_uid[4] = {0x9C, 0xAD, 0xBE, 0xCF};

// --- 设置菜单相关的全局变量 ---
// 用于在编辑模式下暂存用户输入的数值
volatile int editing_value = 0;
// 用于记录当前正在编辑的是哪个设置项
typedef enum
{
    SETTING_NONE,
    SETTING_FOCUS_TIME,
    SETTING_REST_TIME,
    SETTING_LONG_REST_TIME,
    SETTING_LOW_LIGHT_THRESHOLD
} SettingType;
volatile SettingType current_setting_type = SETTING_NONE;
// 菜单项索引，用于在主设置菜单中滚动显示或选择
volatile int setting_menu_index = 0; // 0: SetFocusTime, 1: SetRestTime, ...
const char *setting_menu_items[] = {
    "FcsT", // Focus Time
    "RstT", // Rest Time
    "LgRT", // Long Rest Time
    "L_Lt", // Low Light Threshold
};
#define NUM_SETTING_ITEMS (sizeof(setting_menu_items) / sizeof(setting_menu_items[0]))

// ================== 系统状态枚举 ==================
typedef enum
{
    STATE_IDLE,                // 空闲状态
    STATE_LOADING_MODE,        // 加载状态
    STATE_FOCUS,               // 专注状态
    STATE_REST,                // 休息状态
    STATE_LONG_REST,           // 长休息状态
    STATE_PAUSED,              // 暂停状态
    STATE_SHOW_STATS,          // 显示统计数据
    STATE_TEMP_DISPLAY,        // 临时显示
    STATE_NFC_READ,            // 读取NFC卡片
    STATE_NFC_DISPLAY_PART1,   // 用于显示NFC ID的前半部分
    STATE_NFC_DISPLAY_PART2,   // 用于显示NFC ID的后半部分
    STATE_LOW_LIGHT_WARNING,   // 用于低光照闪烁警告
    STATE_BIND_MENU_PROMPT,    // 提示用户选择要绑定的卡类型 (1: Study, 2: Deep, 3: Data)
    STATE_BINDING_STUDY,       // 等待绑定“学习卡”
    STATE_BINDING_DEEP_WORK,   // 等待绑定“深度工作卡”
    STATE_BINDING_DATA,        // 等待绑定“数据卡”
    STATE_BIND_SUCCESS,        // 绑定成功提示
    STATE_BIND_FAILED,         // 绑定失败/超时提示
    STATE_SET_MENU_MAIN,       // 设置主菜单 (例如: "Set---")
    STATE_SET_FOCUS_TIME,      // 设置专注时长 ("FcsT")
    STATE_SET_REST_TIME,       // 设置休息时长 ("RstT")
    STATE_SET_LONG_REST_TIME,  // 设置长休息时长 ("Lgst")
    STATE_SET_LOW_LIGHT_THRES, // 设置低光照阈值 ("L_Lt")
    STATE_SET_FAN_SPEEDS,      // 设置风扇档位 ("FAnS") - 可选，较复杂
    STATE_SET_EDITING_VALUE,   // 编辑当前设置项的值
    STATE_SET_SAVE_SUCCESS,    // 设置保存成功提示
    STATE_SET_SAVE_FAILED,     // 设置保存失败提示
} SystemState;

// 状态与计时器
volatile SystemState currentState = STATE_IDLE;
volatile int remaining_seconds = 0;
volatile bool g_second_has_passed = false;
volatile int ui_timer_seconds = 0;   // 新增的UI计时器
volatile int flash_count = 0;        // 用于控制闪烁次数
volatile int tap_cooldown_ticks = 0; // 用于敲击检测的冷却计时器
volatile SystemState previousState = STATE_IDLE;

// 定时器配置
volatile int focus_duration_sec = 25 * 60;
volatile int rest_duration_sec = 5 * 60;
volatile int long_rest_duration_sec = 15 * 60;

// 统计，逻辑与其他
int completed_sessions = 0;
int pomodoro_cycle_count = 0;
int fan_level = 1;
char last_key_pressed = 0;
unsigned char last_read_card_id[4] = {0};
volatile unsigned int low_light_threshold = 150; // 定义光照阈值 (单位: Lux)

// ================== 函数声明 ==================
void hardware_init(void);
void handle_inputs(void);
void update_state_machine(void);
void perform_continuous_checks(void);
void load_task_mode(const unsigned char *card_uid);
void start_focus_mode(void);
void start_rest_mode(void);
void update_display(void);
void handle_keypad_input(char key);
void start_scrolling(const char *text);
void stop_scrolling();
bool apply_setting(void);
void enter_setting_edit_mode(SettingType type, int initial_value);

// ================== 重写的硬件定时器代码 ==================
void hz_timer_init(void)
{
    timer_parameter_struct timer_init_struct;
    rcu_periph_clock_enable(RCU_TIMER0);
    timer_deinit(TIMER0);
    timer_init_struct.prescaler = 20000;
    timer_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    timer_init_struct.counterdirection = TIMER_COUNTER_UP;
    timer_init_struct.clockdivision = TIMER_CKDIV_DIV1;
    timer_init_struct.period = 10000; // 200Mhz / 20k / 10k = 1 Hz
    timer_init_struct.repetitioncounter = 0;
    timer_init(TIMER0, &timer_init_struct);
    timer_counter_value_config(TIMER0, 0);
    timer_enable(TIMER0);
    timer_interrupt_enable(TIMER0, TIMER_INT_UP);
    nvic_irq_enable(TIMER0_UP_TIMER9_IRQn, 1, 1);
}

// <<< 优化后的中断服务程序 (ISR) >>>
void TIMER0_UP_TIMER9_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP) != RESET)
    {
        // 1. 递减主计时器
        if (remaining_seconds > 0)
        {
            remaining_seconds--;
        }

        // 新增：管理UI计时器
        if (ui_timer_seconds > 0)
        {
            ui_timer_seconds--;
        }

        // 2. 设置标志位，通知主循环“一秒钟过去了”
        g_second_has_passed = true;

        timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
    }
}

// ================== 主函数 ==================
int main(void)
{
    hardware_init();
    hz_timer_init();

    while (1)
    {
        // 持续执行的任务
        handle_inputs();
        perform_continuous_checks();

        // 每秒执行的任务
        if (g_second_has_passed)
        {
            g_second_has_passed = false;
            update_state_machine();
            update_display();
        }
    }
}

// ================== 辅助函数实现 ==================
void hardware_init(void)
{
    e1_led_info = e1_led_init();
    e1_tube_info = e1_tube_init();
    e2_fan_info = e2_fan_init();
    s1_key_info = s1_key_init();
    s2_illuminance_info = s2_illuminance_init();
    s2_imu_info = s2_imu_init();
    s5_nfc_info = s5_nfc_init();
    s7_ir_info = s7_ir_init();

    e1_led_rgb_set(e1_led_info, 0, 0, 0);
    e2_fan_speed_set(e2_fan_info, 0);
}

void enter_setting_edit_mode(SettingType type, int initial_value)
{
    char buf[10];
    current_setting_type = type;
    editing_value = 0; // 直接从0开始输入，而不是显示当前值
    currentState = STATE_SET_EDITING_VALUE;

    // 根据设置类型调整显示前缀和单位
    switch (type)
    {
    case SETTING_FOCUS_TIME:
        sprintf(buf, "FcsT%02d", editing_value); // 专注时间显示为分钟
        break;
    case SETTING_REST_TIME:
        sprintf(buf, "RstT%02d", editing_value); // 休息时间显示为分钟
        break;
    case SETTING_LONG_REST_TIME:
        sprintf(buf, "LgRT%02d", editing_value); // 长休息时间显示为分钟
        break;
    case SETTING_LOW_LIGHT_THRESHOLD:
        sprintf(buf, "L_Lt%02d", editing_value); // 光照阈值直接显示
        break;
    default:
        sprintf(buf, "ERR "); // 错误显示
        break;
    }
    e1_tube_str_set(e1_tube_info, buf);
}

bool apply_setting(void)
{
    bool success = true;

    // 检查输入值是否为0（无效输入）
    if (editing_value == 0)
    {
        success = false;
        goto cleanup;
    }

    switch (current_setting_type)
    {
    case SETTING_FOCUS_TIME:
        // 设定范围：1分钟到180分钟
        if (editing_value >= 1 && editing_value <= 180)
        {
            focus_duration_sec = editing_value * 60;
        }
        else
        {
            success = false;
        }
        break;
    case SETTING_REST_TIME:
        // 设定范围：1分钟到60分钟
        if (editing_value >= 1 && editing_value <= 60)
        {
            rest_duration_sec = editing_value * 60;
        }
        else
        {
            success = false;
        }
        break;
    case SETTING_LONG_REST_TIME:
        // 设定范围：1分钟到120分钟
        if (editing_value >= 1 && editing_value <= 120)
        {
            long_rest_duration_sec = editing_value * 60;
        }
        else
        {
            success = false;
        }
        break;
    case SETTING_LOW_LIGHT_THRESHOLD:
        // 设定范围：1到500 Lux
        if (editing_value >= 1 && editing_value <= 500)
        {
            low_light_threshold = (unsigned int)editing_value;
        }
        else
        {
            success = false;
        }
        break;
    default:
        success = false; // 未知设置类型
    }
cleanup:
    editing_value = 0;
    current_setting_type = SETTING_NONE;
    return success;
}

void handle_inputs(void)
{
    // --- 按键输入处理 ---
    char current_key = s1_key_value_get(s1_key_info);
    if (current_key != last_key_pressed && current_key != SWN)
    {
        handle_keypad_input(current_key); // 调用原来的处理函数
    }
    last_key_pressed = current_key;

    // --- NFC输入处理 ---
    // 只有在特定状态下才检测NFC
    if (currentState == STATE_IDLE ||
        currentState == STATE_SHOW_STATS ||
        currentState == STATE_NFC_READ ||
        currentState == STATE_BINDING_STUDY ||
        currentState == STATE_BINDING_DEEP_WORK ||
        currentState == STATE_BINDING_DATA)
    {
        unsigned char card_type[2];
        if (s5_nfc_request(s5_nfc_info, 0x26, card_type) == MI_OK)
        {
            if (s5_nfc_anticoll(s5_nfc_info, last_read_card_id) == MI_OK)
            {
                if (currentState == STATE_BINDING_STUDY)
                {
                    memcpy(study_card_uid, last_read_card_id, 4);
                    currentState = STATE_BIND_SUCCESS;
                    ui_timer_seconds = 2; // 显示成功2秒
                }
                else if (currentState == STATE_BINDING_DEEP_WORK)
                {
                    memcpy(deep_work_card_uid, last_read_card_id, 4);
                    currentState = STATE_BIND_SUCCESS;
                    ui_timer_seconds = 2;
                }
                else if (currentState == STATE_BINDING_DATA)
                {
                    memcpy(data_card_uid, last_read_card_id, 4);
                    currentState = STATE_BIND_SUCCESS;
                    ui_timer_seconds = 2;
                }
                // 现有的NFC读取/显示逻辑
                else if (currentState == STATE_NFC_READ)
                {
                    // 显示前两位
                    char display_buf[10];
                    sprintf(display_buf, "%02x%02x", last_read_card_id[0], last_read_card_id[1]);
                    e1_tube_str_set(e1_tube_info, display_buf);

                    // 2进入第一部分显示状态，并设置计时器
                    currentState = STATE_NFC_DISPLAY_PART1;
                    ui_timer_seconds = 2; // 显示2秒
                }
                else
                {
                    load_task_mode(last_read_card_id);
                }
            }
        }
        else if (currentState >= STATE_BINDING_STUDY && currentState <= STATE_BINDING_DATA && ui_timer_seconds <= 0)
        {
            // 如果处于绑定状态但长时间没有检测到卡，则超时失败
            currentState = STATE_BIND_FAILED;
            ui_timer_seconds = 2;
        }
    }
}

void load_task_mode(const unsigned char *card_uid)
{
    if (memcmp(card_uid, study_card_uid, 4) == 0)
    {
        focus_duration_sec = 25 * 60;
        rest_duration_sec = 5 * 60;
        long_rest_duration_sec = 15 * 60;
        e1_tube_str_set(e1_tube_info, "StdY");
    }
    else if (memcmp(card_uid, deep_work_card_uid, 4) == 0)
    {
        focus_duration_sec = 50 * 60;
        rest_duration_sec = 10 * 60;
        long_rest_duration_sec = 10 * 60; // 深度工作模式无长休息
        e1_tube_str_set(e1_tube_info, "dEEP");
    }
    else if (memcmp(card_uid, data_card_uid, 4) == 0)
    {
        currentState = STATE_SHOW_STATS;
        return; // 不改变计时器设置，仅切换状态
    }
    else
    {
        e1_tube_str_set(e1_tube_info, "Err");   // 未知卡
        ui_timer_seconds = 1;                   // 短暂显示错误
        currentState = STATE_NFC_DISPLAY_PART2; // 借用STATE_NFC_DISPLAY_PART2，显示完错误后返回IDLE
        return;
    }

    currentState = STATE_LOADING_MODE;
    ui_timer_seconds = 2;
}

void update_state_machine(void)
{
    switch (currentState)
    {
    case STATE_FOCUS:
        if (remaining_seconds <= 0)
        {
            completed_sessions++;
            pomodoro_cycle_count++;
            if (pomodoro_cycle_count >= 4)
            {
                pomodoro_cycle_count = 0;
                remaining_seconds = long_rest_duration_sec;
                currentState = STATE_LONG_REST;
            }
            else
            {
                start_rest_mode();
            }
        }
        break;
    case STATE_TEMP_DISPLAY:
        if (ui_timer_seconds <= 0)
        {
            // 临时显示结束，返回之前的状态
            currentState = previousState;
        }
        break;
    case STATE_REST:
    case STATE_LONG_REST:
        if (remaining_seconds <= 0)
            start_focus_mode();
        break;
    case STATE_NFC_DISPLAY_PART1:
        if (ui_timer_seconds <= 0)
        {
            char display_buf[10];
            sprintf(display_buf, "%02x%02x", last_read_card_id[2], last_read_card_id[3]);
            e1_tube_str_set(e1_tube_info, display_buf);

            // 进入第二部分显示状态
            currentState = STATE_NFC_DISPLAY_PART2;
            ui_timer_seconds = 2; // 再显示2秒
        }
        break;
    case STATE_NFC_DISPLAY_PART2:
        if (ui_timer_seconds <= 0)
        {
            // 全部显示完毕，返回空闲状态
            currentState = STATE_IDLE;
        }
        break;
    case STATE_LOW_LIGHT_WARNING:
        if (ui_timer_seconds <= 0 && flash_count > 0)
        {
            if (flash_count % 2 == 0)
            {
                e1_led_rgb_set(e1_led_info, 100, 100, 0); // 黄灯亮
            }
            else
            {
                e1_led_rgb_set(e1_led_info, 0, 0, 0); // 黄灯灭
            }
            flash_count--;
            ui_timer_seconds = 1; // 重置1秒计时器
        }

        if (flash_count <= 0)
        {
            // 闪烁结束，正式进入专注模式
            currentState = STATE_FOCUS;
            remaining_seconds = focus_duration_sec;
            e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
            fan_level = 1;
        }
        break;
    case STATE_LOADING_MODE:
        if (ui_timer_seconds <= 0)
        {
            pomodoro_cycle_count = 0;
            start_focus_mode(); // 延时结束，切换到专注模式
        }
        break;
    case STATE_BIND_SUCCESS:
        if (ui_timer_seconds <= 0)
        {
            e1_led_rgb_set(e1_led_info, 0, 100, 0); // 绿色表示成功
            currentState = STATE_BIND_MENU_PROMPT;  // 成功后返回绑定菜单，等待选择下一个
        }
        break;
    case STATE_BIND_MENU_PROMPT: // 如果在绑定菜单超时未选择
        if (ui_timer_seconds <= 0) {
            currentState = STATE_IDLE;
        }
        break;
    case STATE_BIND_FAILED:
        if (ui_timer_seconds <= 0)
        {
            e1_led_rgb_set(e1_led_info, 100, 0, 0); // 红色表示失败
            currentState = STATE_BIND_MENU_PROMPT;  // 失败后返回绑定菜单
        }
        break;
    case STATE_SET_SAVE_SUCCESS:
    case STATE_SET_SAVE_FAILED:
        if (ui_timer_seconds <= 0)
        {
            currentState = STATE_SET_MENU_MAIN;      // 显示完毕后返回设置主菜单
            e1_led_rgb_set(e1_led_info, 50, 0, 100); // 设置菜单紫色
        }
        break;

    default:
        break;
    }
}

void perform_continuous_checks(void)
{
    // --- PIR 检测 ---
    if (currentState == STATE_FOCUS)
    {
        // PIR检测
        if (s7_ir_status_get(s7_ir_info) == 0)
        {

            currentState = STATE_PAUSED;
            e2_fan_speed_set(e2_fan_info, 0);
        }
    }
    else if (currentState == STATE_PAUSED)
    {
        if (s7_ir_status_get(s7_ir_info) == 1)
        {
            currentState = STATE_FOCUS; // 恢复专注
            // 恢复风扇
            if (fan_level == 1)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
            if (fan_level == 2)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_MEDIUM);
            if (fan_level == 3)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_HIGH);
        }
    }

    // 敲击检测
    if (currentState == STATE_FOCUS || currentState == STATE_PAUSED)
    {
        // 1. 更新冷却计时器
        if (tap_cooldown_ticks > 0)
        {
            tap_cooldown_ticks--;
        }

        // 2. 读取IMU数据
        s2_imu_t imu_value = s2_imu_value_get(s2_imu_info);

        // 3. 检测Z轴上的加速度冲击 (敲击)
        if (fabsf(imu_value.acc_z) > TAP_THRESHOLD_G && tap_cooldown_ticks == 0)
        {
            // 4. 设置冷却时间，防止连续触发 (例如1秒)
            tap_cooldown_ticks = 10; // 假设主循环延时100ms，10次即1秒

            // 5. 根据当前状态执行操作
            if (currentState == STATE_FOCUS)
            {
                currentState = STATE_PAUSED;
                e2_fan_speed_set(e2_fan_info, 0); // 暂停时关闭风扇
            }
            else if (currentState == STATE_PAUSED)
            {
                currentState = STATE_FOCUS;
                // 恢复风扇，具体速度取决于fan_level
                if (fan_level == 1)
                    e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
                if (fan_level == 2)
                    e2_fan_speed_set(e2_fan_info, FAN_SPEED_MEDIUM);
                if (fan_level == 3)
                    e2_fan_speed_set(e2_fan_info, FAN_SPEED_HIGH);
            }
        }
    }
}

void start_focus_mode(void)
{
    unsigned int current_illuminance = s2_illuminance_value_get(s2_illuminance_info);
    if (current_illuminance < low_light_threshold)
    {
        e1_tube_str_set(e1_tube_info, "LItE Lo");
        currentState = STATE_LOW_LIGHT_WARNING;
        flash_count = 6;      // 闪烁3次（亮+灭=2，所以3*2=6）
        ui_timer_seconds = 1; // 每秒切换一次亮/灭
    }
    else
    {
        currentState = STATE_FOCUS;
        remaining_seconds = focus_duration_sec;
        e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
        fan_level = 1;
    }
}

void start_rest_mode(void)
{
    currentState = STATE_REST;
    remaining_seconds = rest_duration_sec;
    e2_fan_speed_set(e2_fan_info, 0);
}

void update_display(void)
{
    char buf[10];

    switch (currentState)
    {
    case STATE_IDLE:
        e1_led_rgb_set(e1_led_info, 5, 5, 5);
        e1_tube_str_set(e1_tube_info, "----");
        break;
    case STATE_FOCUS:
        e1_led_rgb_set(e1_led_info, 0, 100, 0); // 绿
        sprintf(buf, "%02d.%02d", remaining_seconds / 60, remaining_seconds % 60);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_REST:
        e1_led_rgb_set(e1_led_info, 0, 0, 100); // 蓝
        sprintf(buf, "%02d.%02d", remaining_seconds / 60, remaining_seconds % 60);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_LONG_REST:
        e1_led_rgb_set(e1_led_info, 100, 0, 100); // 紫
        sprintf(buf, "%02d.%02d", remaining_seconds / 60, remaining_seconds % 60);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_PAUSED:
        e1_led_rgb_set(e1_led_info, 100, 100, 0); // 黄
        e1_tube_str_set(e1_tube_info, "PAUS");
        break;
    case STATE_SHOW_STATS:
        e1_led_rgb_set(e1_led_info, 100, 100, 100); // 白
        sprintf(buf, "donE%02d", completed_sessions);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_NFC_READ:
        e1_led_rgb_set(e1_led_info, 0, 100, 100); // 青
        // 数码管的显示在主循环中直接处理了，这里只负责灯光
        break;
    case STATE_BIND_MENU_PROMPT:
        e1_led_rgb_set(e1_led_info, 100, 50, 0); // 橙色
        e1_tube_str_set(e1_tube_info, "bnd?");   // 提示选择绑定类型
        break;
    case STATE_BINDING_STUDY:
        e1_led_rgb_set(e1_led_info, 100, 50, 0);
        e1_tube_str_set(e1_tube_info, "bnd1"); // 等待刷学习卡
        break;
    case STATE_BINDING_DEEP_WORK:
        e1_led_rgb_set(e1_led_info, 100, 50, 0);
        e1_tube_str_set(e1_tube_info, "bnd2"); // 等待刷深度工作卡
        break;
    case STATE_BINDING_DATA:
        e1_led_rgb_set(e1_led_info, 100, 50, 0);
        e1_tube_str_set(e1_tube_info, "bnd3"); // 等待刷数据卡
        break;
    case STATE_BIND_SUCCESS:
        e1_led_rgb_set(e1_led_info, 0, 100, 0); // 绿色
        e1_tube_str_set(e1_tube_info, " OK ");
        break;
    case STATE_BIND_FAILED:
        e1_led_rgb_set(e1_led_info, 100, 0, 0); // 红色
        e1_tube_str_set(e1_tube_info, "Fail");
        break;
    case STATE_TEMP_DISPLAY:
        // 数码管内容已由 handle_keypad_input 或其他调用者设置，此处不覆盖
        e1_led_rgb_set(e1_led_info, 100, 100, 0); // 黄色表示临时显示
        break;
    case STATE_SET_MENU_MAIN:
        e1_led_rgb_set(e1_led_info, 50, 0, 100); // 紫色
        // 显示当前菜单项名称而不是编号
        sprintf(buf, "%s", setting_menu_items[setting_menu_index]);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_SET_FOCUS_TIME:
    case STATE_SET_REST_TIME:
    case STATE_SET_LONG_REST_TIME:
    case STATE_SET_LOW_LIGHT_THRES:
        // 这些状态会立即进入 STATE_SET_EDITING_VALUE，所以这里不需要特别显示
        // 实际上，enter_setting_edit_mode 会直接设置 currentState
        break;
    case STATE_SET_EDITING_VALUE:
        e1_led_rgb_set(e1_led_info, 100, 50, 0); // 橙色
        // 显示会在handle_keypad_input中实时更新
        break;
    case STATE_SET_SAVE_SUCCESS:
        e1_led_rgb_set(e1_led_info, 0, 100, 0); // 绿色
        e1_tube_str_set(e1_tube_info, " OK ");
        break;
        
    case STATE_SET_SAVE_FAILED:
        e1_led_rgb_set(e1_led_info, 100, 0, 0); // 红色
        e1_tube_str_set(e1_tube_info, "FAIL");
        break;
    }
}

void handle_keypad_input(char key)
{
    char buf[10];

    switch (currentState)
    {
    case STATE_FOCUS:
        if (key == '*')
            currentState = STATE_PAUSED;
        else if (key == '#')
            currentState = STATE_IDLE;
        else if (key == '1')
            remaining_seconds += 5 * 60;
        else if (key == '2')
        {
            char buf[10];
            fan_level = (fan_level % 3) + 1;
            if (fan_level == 1)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
            if (fan_level == 2)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_MEDIUM);
            if (fan_level == 3)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_HIGH);

            previousState = STATE_FOCUS;        // 保存当前状态
            currentState = STATE_TEMP_DISPLAY;  // 切换到临时显示状态
            ui_timer_seconds = 2;               // 设置显示时长为2秒
            sprintf(buf, "FAn %d", fan_level);  // 准备要显示的内容
            e1_tube_str_set(e1_tube_info, buf); // 立即更新数码管
        }else if (key == '9') // 开发者功能：跳过当前阶段
        {
            previousState = currentState; // 保存当前状态
            currentState = STATE_TEMP_DISPLAY; // 临时显示 "SKIP"
            e1_tube_str_set(e1_tube_info, "SKIP");
            ui_timer_seconds = 1; // 显示1秒
            remaining_seconds = 0; // 强制结束当前阶段
        }
        else if (key == '8') // 开发者功能：加速当前阶段
        {
            previousState = currentState; // 保存当前状态
            currentState = STATE_TEMP_DISPLAY; // 临时显示 "FAST"
            e1_tube_str_set(e1_tube_info, "FAST");
            ui_timer_seconds = 1; // 显示1秒
            if (remaining_seconds > 5) // 如果剩余时间大于5秒，则设置为5秒
            {
                remaining_seconds = 5;
            }
        }
        break;
    case STATE_REST:
    case STATE_LONG_REST:
        if (key == '*' || key == '0')
            start_focus_mode(); // 跳过休息
        else if (key == '1')
            remaining_seconds += 5 * 60;
        else if (key == '#')
            currentState = STATE_IDLE;
        else if (key == '9') // 开发者功能：跳过当前阶段
        {
            previousState = currentState; // 保存当前状态
            currentState = STATE_TEMP_DISPLAY; // 临时显示 "SKIP"
            e1_tube_str_set(e1_tube_info, "SKIP");
            ui_timer_seconds = 1; // 显示1秒
            remaining_seconds = 0; // 强制结束当前阶段
        }
        else if (key == '8') // 开发者功能：加速当前阶段
        {
            previousState = currentState; // 保存当前状态
            currentState = STATE_TEMP_DISPLAY; // 临时显示 "FAST"
            e1_tube_str_set(e1_tube_info, "FAST");
            ui_timer_seconds = 1; // 显示1秒
            if (remaining_seconds > 5) // 如果剩余时间大于5秒，则设置为5秒
            {
                remaining_seconds = 5;
            }
        }
        break;
    case STATE_PAUSED:
        if (key == '*')
        {
            currentState = STATE_FOCUS; // 恢复
        }
        break;
    case STATE_IDLE:
    case STATE_SHOW_STATS:
        if (key == '#')
        { // 从统计返回
            currentState = STATE_IDLE;
            completed_sessions = 0; // 可选：按#重置统计
        }
        else if (key == '5')
        {
            currentState = STATE_NFC_READ;
            e1_tube_str_set(e1_tube_info, "rEAd"); // 立即更新显示，提供即时反馈
            ui_timer_seconds = 1;                  // 短暂显示
        }
        else if (key == '6') // 按 6 进入设置主菜单
        {
            currentState = STATE_SET_MENU_MAIN;
            setting_menu_index = 0;                  // 默认显示第一个菜单项
            e1_tube_str_set(e1_tube_info, "SET---"); // 初始显示
        }
        else if (key == '0')
        { // 按0进入绑定模式
            currentState = STATE_BIND_MENU_PROMPT;
            e1_tube_str_set(e1_tube_info, "bnd?");
            ui_timer_seconds = 5; // 如果5秒内没选择，自动退出
        }
        break;
    case STATE_BIND_MENU_PROMPT:
        if (key == '1')
        { // 绑定学习卡
            currentState = STATE_BINDING_STUDY;
            e1_tube_str_set(e1_tube_info, "bnd1");
            ui_timer_seconds = 10; // 等待刷卡10秒超时
        }
        else if (key == '2')
        { // 绑定深度工作卡
            currentState = STATE_BINDING_DEEP_WORK;
            e1_tube_str_set(e1_tube_info, "bnd2");
            ui_timer_seconds = 10;
        }
        else if (key == '3')
        { // 绑定数据卡
            currentState = STATE_BINDING_DATA;
            e1_tube_str_set(e1_tube_info, "bnd3");
            ui_timer_seconds = 10;
        }
        else if (key == '#')
        { // 退出绑定菜单
            currentState = STATE_IDLE;
        }
        break;

    case STATE_BINDING_STUDY:
    case STATE_BINDING_DEEP_WORK:
    case STATE_BINDING_DATA:
        if (key == '#')
        { // 强制退出当前绑定操作，返回绑定菜单
            currentState = STATE_BIND_MENU_PROMPT;
            ui_timer_seconds = 0; // 清除超时计时器
        }
        break;
    case STATE_SET_MENU_MAIN:
        if (key >= '1' && key <= '4') // 限制为有效的菜单项数量
        {
            int selected_index = key - '1';
            setting_menu_index = selected_index;

            // 进入对应的设置编辑状态
            switch (selected_index)
            {
            case 0: // Focus Time
                enter_setting_edit_mode(SETTING_FOCUS_TIME, focus_duration_sec / 60);
                break;
            case 1: // Rest Time
                enter_setting_edit_mode(SETTING_REST_TIME, rest_duration_sec / 60);
                break;
            case 2: // Long Rest Time
                enter_setting_edit_mode(SETTING_LONG_REST_TIME, long_rest_duration_sec / 60);
                break;
            case 3: // Low Light Threshold
                enter_setting_edit_mode(SETTING_LOW_LIGHT_THRESHOLD, low_light_threshold);
                break;
            }
        }
        else if (key == '*') // 滚动到下一个菜单项
        {
            setting_menu_index = (setting_menu_index + 1) % NUM_SETTING_ITEMS;
        }
        else if (key == '#') // 退出设置菜单
        {
            currentState = STATE_IDLE;
            setting_menu_index = 0;
        }
        break;

    case STATE_SET_EDITING_VALUE:
        if (key >= '0' && key <= '9')
        {
            int new_value = editing_value * 10 + (key - '0');

            // 根据当前设置类型进行范围检查
            bool valid_input = false;
            switch (current_setting_type)
            {
            case SETTING_FOCUS_TIME:
                valid_input = (new_value <= 180);
                break;
            case SETTING_REST_TIME:
                valid_input = (new_value <= 60);
                break;
            case SETTING_LONG_REST_TIME:
                valid_input = (new_value <= 120);
                break;
            case SETTING_LOW_LIGHT_THRESHOLD:
                valid_input = (new_value <= 500);
                break;
            default:
                valid_input = false;
            }

            if (valid_input && new_value < 1000)
            {
                editing_value = new_value;
                sprintf(buf, " %03d", editing_value);
                e1_tube_str_set(e1_tube_info, buf);
            }
            else
            {
                // 显示错误提示
                sprintf(buf, "MAX!");
                e1_tube_str_set(e1_tube_info, buf);
                ui_timer_seconds = 1;
                currentState = STATE_TEMP_DISPLAY;
                previousState = STATE_SET_EDITING_VALUE;
            }
        }
        else if (key == '*') // 确认并保存
        {
            if (apply_setting())
            {
                currentState = STATE_SET_SAVE_SUCCESS;
                ui_timer_seconds = 2;
            }
            else
            {
                currentState = STATE_SET_SAVE_FAILED;
                ui_timer_seconds = 2;
            }
        }
        else if (key == '#') // 取消编辑
        {
            editing_value = 0;
            current_setting_type = SETTING_NONE;
            currentState = STATE_SET_MENU_MAIN;
        }
        break;

    // SetSaveSuccess 和 SetSaveFailed 会在 update_state_machine 中处理超时
    case STATE_SET_SAVE_SUCCESS:
    case STATE_SET_SAVE_FAILED:
        if (key == '#')
        {
            currentState = STATE_SET_MENU_MAIN;
            ui_timer_seconds = 0;
        }
        break;

    default:
        break;
    }
}
