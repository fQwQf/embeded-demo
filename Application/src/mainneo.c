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
#define LOW_LIGHT_THRESHOLD 150 // 定义光照阈值 (单位: Lux)

// --- NFC卡片唯一ID (UID) ---
const unsigned char STUDY_CARD_UID[4] = {0xFE, 0x1F, 0x7F, 0xC2};
const unsigned char DEEP_WORK_CARD_UID[4] = {0x5E, 0x6F, 0x7A, 0x8B};
const unsigned char DATA_CARD_UID[4] = {0x9C, 0xAD, 0xBE, 0xCF};

// ================== 系统状态枚举 ==================
typedef enum
{
    STATE_IDLE,
    STATE_FOCUS,
    STATE_REST,
    STATE_LONG_REST,
    STATE_PAUSED,
    STATE_SNOOZE,
    STATE_SHOW_STATS,
    STATE_NFC_READ
} SystemState;

// --- 状态与计时器 ---
volatile SystemState currentState = STATE_IDLE;
volatile int remaining_seconds = 0;
volatile bool g_second_has_passed = false;

// --- 定时器配置 ---
int focus_duration_sec = 25 * 60;
int rest_duration_sec = 5 * 60;
int long_rest_duration_sec = 15 * 60;

// --- 统计与逻辑 ---
int completed_sessions = 0;
int pomodoro_cycle_count = 0;
int fan_level = 1;
char last_key_pressed = 0;

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

// ================== <<< 重写的硬件定时器代码 >>> ==================
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
    if (currentState == STATE_IDLE || currentState == STATE_SHOW_STATS || currentState == STATE_NFC_READ)
    {
        unsigned char card_id[4];
        unsigned char card_type[2];
        if (s5_nfc_request(s5_nfc_info, 0x26, card_type) == MI_OK)
        {
            if (s5_nfc_anticoll(s5_nfc_info, card_id) == MI_OK)
            {
                if (currentState == STATE_NFC_READ)
                {
                    char display_buf[10];

                    // 成功读取ID，开始分段显示

                    // 显示前2个字节
                    sprintf(display_buf, "%02x%02x", card_id[0], card_id[1]);
                    e1_tube_str_set(e1_tube_info, display_buf);
                    delay_ms(2000); // 停留2秒

                    // 显示后2个字节
                    sprintf(display_buf, "%02x%02x", card_id[2], card_id[3]);
                    e1_tube_str_set(e1_tube_info, display_buf);
                    delay_ms(2000); // 停留2秒

                    // 显示完毕，返回空闲状态
                    currentState = STATE_IDLE;
                }
                else
                {
                    load_task_mode(card_id);
                }
            }
        }
    }
}

void load_task_mode(const unsigned char *card_uid)
{
    if (memcmp(card_uid, STUDY_CARD_UID, 4) == 0)
    {
        focus_duration_sec = 25 * 60;
        rest_duration_sec = 5 * 60;
        long_rest_duration_sec = 15 * 60;
        e1_tube_str_set(e1_tube_info, "StdY");
    }
    else if (memcmp(card_uid, DEEP_WORK_CARD_UID, 4) == 0)
    {
        focus_duration_sec = 50 * 60;
        rest_duration_sec = 10 * 60;
        long_rest_duration_sec = 10 * 60; // 深度工作模式无长休息
        e1_tube_str_set(e1_tube_info, "dEEP");
    }
    else if (memcmp(card_uid, DATA_CARD_UID, 4) == 0)
    {
        currentState = STATE_SHOW_STATS;
        return; // 不改变计时器设置，仅切换状态
    }
    else
    {
        e1_tube_str_set(e1_tube_info, "Err"); // 未知卡
        delay_ms(1000);
        return;
    }

    delay_ms(1500);           // 显示模式后稍作停留
    pomodoro_cycle_count = 0; // 重置循环计数
    start_focus_mode();
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
    case STATE_REST:
    case STATE_LONG_REST:
        if (remaining_seconds <= 0)
            start_focus_mode();
        break;
    case STATE_SNOOZE:
        if (remaining_seconds <= 0)
            start_rest_mode();
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

    // --- IMU 轻拍检测 ---
    // 这个需要高频检测，所以留在这里持续运行
    if (currentState == STATE_FOCUS && remaining_seconds <= 0)
    {
        s2_imu_t imu = s2_imu_value_get(s2_imu_info);
        float magnitude = sqrtf(imu.acc_x * imu.acc_x + imu.acc_y * imu.acc_y + imu.acc_z * imu.acc_z);
        if (magnitude > TAP_THRESHOLD_G)
        {
            currentState = STATE_SNOOZE;
            remaining_seconds = 5 * 60;
            delay_ms(500); // 保留短暂延时用于按键/IMU防抖是可接受的
        }
    }
}

void start_focus_mode(void)
{
    unsigned int current_illuminance = s2_illuminance_value_get(s2_illuminance_info);
    if (current_illuminance < LOW_LIGHT_THRESHOLD)
    {
        e1_tube_str_set(e1_tube_info, "LItE Lo");
        // 闪烁黄色灯光以示提醒
        for (int i = 0; i < 3; i++)
        {
            e1_led_rgb_set(e1_led_info, 100, 100, 0); // 黄
            delay_ms(300);                            // 注意：这里的短延时是可接受的，因为它只在开始时执行一次
            e1_led_rgb_set(e1_led_info, 0, 0, 0);
            delay_ms(300);
        }
    }

    currentState = STATE_FOCUS;
    remaining_seconds = focus_duration_sec;
    e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
    fan_level = 1;
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
    case STATE_SNOOZE:
        e1_led_rgb_set(e1_led_info, 0, 50, 50); // 青色
        e1_tube_str_set(e1_tube_info, "Snoo");
        break;
    case STATE_SHOW_STATS:
        e1_led_rgb_set(e1_led_info, 100, 100, 100); // 白
        sprintf(buf, "donE%02d", completed_sessions);
        e1_tube_str_set(e1_tube_info, buf);
        break;
    case STATE_NFC_READ:                          // <-- 新增显示逻辑
        e1_led_rgb_set(e1_led_info, 0, 100, 100); // 青色 (Cyan)
        // 数码管的显示在主循环中直接处理了，这里只负责灯光
        break;
    }
}

void handle_keypad_input(char key)
{
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
            fan_level = (fan_level % 3) + 1;
            if (fan_level == 1)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_LOW);
            if (fan_level == 2)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_MEDIUM);
            if (fan_level == 3)
                e2_fan_speed_set(e2_fan_info, FAN_SPEED_HIGH);
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
        { // 从统计或空闲返回
            currentState = STATE_IDLE;
            completed_sessions = 0; // 可选：按#重置统计
        }
        else if (key == '5')
        { // <-- 新增逻辑
            currentState = STATE_NFC_READ;
            e1_tube_str_set(e1_tube_info, "rEAd"); // 立即更新显示，提供即时反馈
            delay_ms(500);                         // 避免按键抖动
        }
        break;

    default:
        break;
    }
}