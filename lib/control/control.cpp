#include <Arduino.h>
#include "control.h"
#include "RobotConfig.h"
#include "Kinematics.h"
#include "motor.h"

int Target_pos_A = 0; //setpoint speed
int Target_pos_B = 0;
unsigned long time_pre = 0;
//sai số A
float er_A = 0;
float er_A_pre = 0;

float er_A_v = 0;
float sum_er_A = 0;
int posA_pre = 0;
//sai số B
float er_B = 0;
float er_B_pre = 0;

float er_B_v = 0;
float sum_er_B = 0;
int posB_pre = 0;

//state machine
volatile bool flag = false;
volatile bool flag_goal = false;
volatile bool flag_running = false;

void Task_2(void *parameters){
    control command_receive;
    control current_command = control::STOP;

    while(1){
        if(flag_running == false){
            if(xQueueReceive(Ong_Truyen_Lenh, &command_receive, 0) == pdTRUE){
                current_command = command_receive; 
                
                if(command_receive == control::FINISH){
                    flag_goal = true;
                    flag = false;
                    go(control::STOP, 0, 0); 
                    Serial.println("______HOÀN THÀNH TOÀN BỘ CHUỖI LỆNH______");
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                else {
                    switch(current_command) {
                        case control::TOP:
                            Target_pos_A = xung_top::target_pos; 
                            Target_pos_B = xung_top::target_pos;
                            break;
                
                        case control::BACK:
                            Target_pos_A = -xung_top::target_pos; 
                            Target_pos_B = -xung_top::target_pos;
                            break;
                
                        case control::RIGHT:
                            Target_pos_A = -xung_re::target_pivot; 
                            Target_pos_B = xung_re::target_pivot;
                            break;
                
                        case control::LEFT:
                            Target_pos_A = xung_re::target_pivot; 
                            Target_pos_B = -xung_re::target_pivot;
                            break;
                    }
                    flag_running = true;
                    flag = true; 
                    flag_goal = false;
                    reset(); 
                    time_pre = millis(); 
                    
                    encoderA_values = 0; 
                    encoderB_values = 0;

                    posA_pre = 0;
                    posB_pre = 0;
                    er_A_pre = 0;
                    er_B_pre = 0;
                    sum_er_A = 0; 
                    sum_er_B = 0;

                    Serial.print(">> Xe bat dau chay lenh: ");
                    Serial.println((int)current_command);
                    vTaskDelay(1000 / portTICK_PERIOD_MS); 
                }
            }
        } 

        if(flag == true && flag_goal == false && flag_running == true){
            unsigned long time_now = millis();
            if(time_now - time_pre >= 50){
                float dt = (time_now - time_pre) / 1000.0;
                
                if (dt > 0) {
                    time_pre = time_now;
                    int pos_A = encoderA_values;
                    int pos_B = encoderB_values;
                    //velocity current
                    int ds_A = pos_A - posA_pre;
                    int ds_B = pos_B - posB_pre;
                    posA_pre = pos_A;
                    posB_pre = pos_B;

                    //PD a & b
                    er_A = Target_pos_A - pos_A;
                    er_B = Target_pos_B - pos_B;
                    float P_posA = CONTR::Kp_A_P * er_A;
                    float p_posB = CONTR::Kp_B_P * er_B;
                    float D_posA = CONTR::Kd_A_P * (er_A - er_A_pre) / dt;
                    er_A_pre = er_A;
                    float D_posB = CONTR::Kd_B_P * (er_B - er_B_pre) / dt;
                    er_B_pre = er_B;
                    int OP_A_P = (int)(P_posA + D_posA); //output a position = target_velocity_A
                    int OP_B_P = (int)(p_posB + D_posB); //output b position = target_velocity_B

                    OP_A_P = constrain(OP_A_P, -600, 600);
                    OP_B_P = constrain(OP_B_P, -600, 600);

                    er_A_v = OP_A_P - ds_A;
                    er_B_v = OP_B_P - ds_B;
                    sum_er_A += er_A_v * dt;
                    sum_er_B += er_B_v * dt;

                    sum_er_A = constrain(sum_er_A, -1000, 1000);
                    sum_er_B = constrain(sum_er_B, -1000, 1000);

                    float P_velA = CONTR::Kp_A_V*er_A_v;
                    float P_velB = CONTR::Kp_B_V*er_B_v;
                    float I_velA = CONTR::Ki_A_V * sum_er_A;
                    float I_velB = CONTR::Ki_B_V * sum_er_B;
                    int OP_A_V = (int)(P_velA + I_velA);
                    int OP_B_V = (int)(P_velB + I_velB);
                    
                    OP_A_V = constrain(OP_A_V, -1023, 1023);
                    OP_B_V = constrain(OP_B_V, -1023, 1023);
// 📈 Đồ thị 1: Vòng Vị trí - Bánh A (Kéo D_posA đè lên P_posA)
                    Serial.printf(">P_posA:%.2f\n", P_posA);
                    Serial.printf(">D_posA:%.2f\n", D_posA);

// 📈 Đồ thị 2: Vòng Vị trí - Bánh B (Kéo D_posB đè lên P_posB)
                    Serial.printf(">P_posB:%.2f\n", p_posB); // Trong code sếp viết chữ 'p' thường
                    Serial.printf(">D_posB:%.2f\n", D_posB);

// 📈 Đồ thị 3: Vòng Vận tốc - Bánh A (Kéo I_velA đè lên P_velA)
                    Serial.printf(">P_velA:%.2f\n", P_velA);
                    Serial.printf(">I_velA:%.2f\n", I_velA);

// 📈 Đồ thị 4: Vòng Vận tốc - Bánh B (Kéo I_velB đè lên P_velB)
                    Serial.printf(">P_velB:%.2f\n", P_velB);
                    Serial.printf(">I_velB:%.2f\n", I_velB);
                    go(current_command, OP_A_V,  OP_B_V);

                    if(abs(er_A) <= 10 && abs(er_B) <= 10){
                        go(control::STOP, 0, 0);
                        flag_running = false;
                        
                        Serial.println(">> Done 1 buoc! Chuan bi chay buoc tiep theo...");
                        vTaskDelay(500 / portTICK_PERIOD_MS); 
                    }
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}