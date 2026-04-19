#include <Arduino.h>
#include "motor.h"
#include "control.h"
#include "matrix.h"



#define time_push 3000
#define space 100

QueueHandle_t Ong_Truyen_Lenh;

const int led = 1;

const int buzze = 2;

unsigned long push_previous = 0;

enum State{IDLE, RECORD, PLAYING, LOAD, ERROR, ASTAR};

button route[space];
int step = 0;
State current_state = IDLE;

//scan matrixbutton
void read_matrixbutton(){
        button raw_command = scan();
        button command = button::NONE;
        if(raw_command != button::NONE){
            vTaskDelay(50 / portTICK_PERIOD_MS); 
            if(scan() == raw_command){
                command = raw_command; 
            }
        }
        unsigned long push_current = millis();

        if(command != button::NONE){
            if(current_state == RECORD && (push_current - push_previous >= time_push)){
                current_state = ERROR;
                Serial.println("Now: Error (Bam phim qua cham!)");
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            
            push_previous = push_current; 
        }
        switch(current_state){
            
            case IDLE:
                if(command != button::NONE && command == button::OK){
                    current_state = RECORD;
                    Serial.println("Now: RECORD");
                    while(scan() != button::NONE) { 
                        vTaskDelay(20); 
                    }
                }
                break;
                
            case RECORD:
                if(command != button::NONE){
                    if(command == button::OK){
                        current_state = LOAD;
                        Serial.println("Now: LOAD");
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        while(scan() != button::NONE) { 
                            vTaskDelay(20); 
                        }
                    } 
                    else { 
                        if(step > 0){
                            button step_previous = route[step - 1];
                            if( (step_previous == button::TOP && command == button::BACK) ||
                                (step_previous == button::BACK && command == button::TOP) ||
                                (step_previous == button::RIGHT && command == button::LEFT) ||
                                (step_previous == button::LEFT && command == button::RIGHT) )
                            {
                                current_state = ERROR;
                                Serial.println("Now: ERROR");
                                vTaskDelay(100 / portTICK_PERIOD_MS); 
                                break;
                            }
                        }
                        
                        if(step < 100){
                            route[step] = command; 
                            Serial.print("Buoc so "); 
                            Serial.println(step + 1);
                            vTaskDelay(100 / portTICK_PERIOD_MS);
                            Serial.print("Huong: "); Serial.println((int)command);
                            vTaskDelay(100 / portTICK_PERIOD_MS);
                            step++;
                            while(scan() != button::NONE) { 
                                vTaskDelay(20); 
                            }
                        }
                    }
                }
                break; 
                
            case LOAD:
            {
                for(int i = 0; i < step; i++){
                    control command_run;
                    if(route[i] == button::TOP)        command_run = control::TOP;
                    else if(route[i] == button::BACK)  command_run = control::BACK;
                    else if(route[i] == button::LEFT)  command_run = control::LEFT;
                    else if(route[i] == button::RIGHT) command_run = control::RIGHT;
                    else                               command_run = control::STOP;
                    
                    xQueueSend(Ong_Truyen_Lenh, &command_run, portMAX_DELAY);
                }
                
                control command_end = control::FINISH;
                xQueueSend(Ong_Truyen_Lenh, &command_end, portMAX_DELAY);
                
                current_state = PLAYING;
                Serial.println("Now: Playing");
                break;
            }
            case PLAYING:
                if(flag == false) {
                    step = 0;
                    current_state = IDLE;
                    Serial.println("Now: IDLE (San sang cho luot moi)");
                }
                break;
                
            case ERROR:
                if(command != button::NONE && command == button::OK){
                    step = 0;
                    current_state = IDLE;
                    Serial.println("Now: IDLE");
                    while(scan() != button::NONE) { vTaskDelay(20); }
                }
                break;
        }

        vTaskDelay(30 / portTICK_PERIOD_MS);
}
//ring and blink
void finish(){
    pinMode(led, OUTPUT);
    pinMode(buzze, OUTPUT);
    Serial.println(">>> TASK 3: BAT DAU AN MUNG! <<<");
            
    for(int i = 0; i < 3; i++) {
        digitalWrite(buzze, HIGH);
        digitalWrite(led, HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS);
                
        digitalWrite(led, LOW);
        digitalWrite(buzze, LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
            
    flag_goal = false;    
    Serial.println(">>> TASK 3: AN MUNG XONG! NGHỈ! <<<");   
    vTaskDelay(50 / portTICK_PERIOD_MS); 
}
void setup(){
    //open Serial
    Serial.begin(115200);
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
    Serial.println("===========================");
    Serial.println("Start NOW: HE THONG DA MO!");
    Serial.println("===========================");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //Config pinMode
    initEncoder();
    setupMatrix();
    initMotor();
    Serial.println("=::Đã khởi động phần cứng thành công::=");
    
    //Creat QUEUE
    Ong_Truyen_Lenh = xQueueCreate(105, sizeof(MotorCmd));
    if(Ong_Truyen_Lenh == NULL){
        Serial.println("LỖI: Không tạo được Queue!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.println("Khoi tao thanh cong ong truyen lenh");
    delay(1000);
    Serial.println(">>> SETUP HOAN TAT. SAN SANG! <<<");
    Serial.println("Now: IDLE");
    delay(1000);
}
void loop(){
    read_matrixbutton();
    if(flag_goal == true){
        finish();
    }
}