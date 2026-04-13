#include <Arduino.h>
#include <vector>
#include <algorithm>
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

std::vector<button> route[space];
int step = 0;
State current_state = IDLE;

//scan matrixbutton
void Task_1(void *parameter){
    while(1){
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
            else if(current_state == IDLE && (push_current - push_previous >= time_push)){
                current_state = ASTAR;
                Serial.println("Now: Thuat Toan A* Da Sang Sang");
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
                        //stupid
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
                        if(step < space){
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
                //fill input
                Serial.println("Dang toi uu hoa loi di  : ----- :");
                int cnt = 1;
                for(int i = 0; i < step; i++){
                    if( i < step - 1 && route[i] == route[i + 1]){
                        cnt++;
                    }
                    else{
                        MotorCmd package;
                        package.multip = cnt;
                        if(route[i] == button::TOP)        package.type = control::TOP;
                        else if(route[i] == button::BACK)  package.type = control::BACK;
                        else if(route[i] == button::LEFT)  package.type = control::LEFT;
                        else if(route[i] == button::RIGHT) package.type = control::RIGHT;
                        else                               package.type = control::STOP;
                        xQueueSend(Ong_Truyen_Lenh, &package, portMAX_DELAY);
                    }
                    cnt = 1;
                }
                /*for(int i = 0; i < step; i++){
                    control command_run;
                    if(route[i] == button::TOP)        command_run = control::TOP;
                    else if(route[i] == button::BACK)  command_run = control::BACK;
                    else if(route[i] == button::LEFT)  command_run = control::LEFT;
                    else if(route[i] == button::RIGHT) command_run = control::RIGHT;
                    else                               command_run = control::STOP;
                    
                    //xQueueSend(Ong_Truyen_Lenh, &command_run, portMAX_DELAY);
                }*/
                
                MotorCmd command_end = {control::FINISH, 1};
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
            case ASTAR:
                break;
        }

        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}
//ring and blink
void Task_3(void *parameter){
    pinMode(led, OUTPUT);
    pinMode(buzze, OUTPUT);
    while(1) {
        // chinh sua flag_goal
        if (flag_goal == true) {
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
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS); 
    }
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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Create task_1
    xTaskCreatePinnedToCore(
        Task_1,
        "Matrix_Button_Scan", 
        2048,
        NULL,
        1,
        NULL,
        1
    );
    Serial.println("khoi tao xong task_1");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //Create task_2
    xTaskCreatePinnedToCore(
        Task_2,
        "Control_PID", 
        18160, 
        NULL,
        1, 
        NULL, 
        1
    );
    Serial.println("Khoi tao thanh cong task_2");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //Create task_3
    xTaskCreatePinnedToCore(
        Task_3,
        "Led and Buzzer",
        1024,
        NULL, 
        0,
        NULL,
        0  
    );
    Serial.println("Khoi tao thanh cong Task_3");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    Serial.println(">>> SETUP HOAN TAT. SAN SANG! <<<");
    Serial.println("Now: IDLE");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
void loop(){
    
}