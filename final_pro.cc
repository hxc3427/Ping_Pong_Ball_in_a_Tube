

#include <cstdlib>
#include <iostream>
#include <sys/neutrino.h>//for privity
#include<stdio.h>
#include <unistd.h>//delay
#include<stdlib.h>
#include<pthread.h>//for thread
#include <time.h>//for clock()
#include <sys/mman.h> //for mmap
#include <stdint.h> //for uintptr_t
#include <hw/inout.h>
#include <sys/timeb.h>
#include <vector>

#define Port_Length 1
#define Data_Base 0x280

using namespace std;

void * fan_control(void* );
/*******************************************************

                  Ultra-Sonic Sensor

*******************************************************/

std::vector <int> signalWidth;

void * ball_level( void* )
{
        int i=0; //vector index

        if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1)   // root permission
        {
            perror("Failed to get I/O access permission");
            return NULL;
        }

        clock_t start;
        clock_t end;

        uintptr_t dir_ptr=mmap_device_io(Port_Length,Data_Base +0xB);
        if(dir_ptr == MAP_DEVICE_FAILED)  // test for root permission
        {
            perror("Failed to map control register");
            return NULL;
        }
        out8(dir_ptr,0x02);

        ///////Setting up port A
        uintptr_t da_ptr = mmap_device_io( Port_Length,Data_Base + 0x8 );// 280 --base + 8(B)
        if(da_ptr == MAP_DEVICE_FAILED) // test for root permission
        {
            perror("Failed to map control register");
            return NULL;
        }
        ///////Setting up port B
        uintptr_t db_ptr = mmap_device_io( Port_Length,Data_Base +0x9 );// 280 --base + 9(B)
        if(db_ptr == MAP_DEVICE_FAILED) // test for root permission
        {
            perror("Failed to map control register");
            return NULL;
        }
                                while(1){
                                /////trigger sensor through port A
                                out8(da_ptr,0xFF);//set A to 1
                                delay(100);//10 readings per sec are required
                                out8(da_ptr,0x00);//stop triggereing set A to 0

                                while(in8(db_ptr)!=0xFF){
                                        //waiting for echo/portB to go high i.e. trigger reaches back to the sensor
                                }
                                start=clock(); //record start of signal
                                while(in8(db_ptr)!=0xFE){
                                //waiting for echo/portB to go low i.e. signal ends
                                }
                                end=clock(); //record end of signal

                                int signal= (end-start)/2;// get the actual signal width= roundTripTime/2
                                int d=0;
                                d= (signal*13)/1000;//convert to distance
                                if(d<235){//within threshold max threshold given is 18ms---which is 234 inches
                                        signalWidth.push_back(signal);
                                        //printf("signal Width%d\n",signalWidth[i]);
                                        int d= (signalWidth.at(i)*13)/1000;//convert to distance
                                        printf("Distance is :%d\n", d);
                                        i++;//increment vector index
                                        pthread_exit(NULL);
                                }else{
                                        printf("////////////////////////\n");
                                }
                                }

}
/*******************************************************

                        main

*******************************************************/
int main(int argc, char *argv[]) {
    pthread_t fan;
    pthread_t ball_height;//Thread that will range
    while(1)
    {
    pthread_create( &ball_height, NULL,&ball_level , NULL );//directing thread to sonicRanging function
    delay(80);
    pthread_create( &fan, NULL, &fan_control, NULL );
    delay(30);
}
    //while(1);
    //pthread_join( ball_height, NULL );}//wait for Ranger Thread to join up with main---to avoid premature termination of the program
   // pthread_join( fan, NULL );}
    return EXIT_SUCCESS;
}
/*******************************************************

                        PWM CODE

*******************************************************/
double Duty=20;
void * fan_control(void* )
{
    uintptr_t dir_ptr=mmap_device_io(Port_Length,Data_Base +0xB); //pointer for setting direction of io ports
    uintptr_t dc_ptr=mmap_device_io(Port_Length,Data_Base +0xA);  //pointer to port B
    useconds_t on_time=0;
    if ( ThreadCtl(_NTO_TCTL_IO, NULL) == -1)   // root permission
        {
            perror("Failed to get I/O access permission");
            return NULL;
        }
    if(dir_ptr == MAP_DEVICE_FAILED)  // test for root permission
        {
            perror("Failed to map control register");
            return NULL;
        }
    if(dc_ptr == MAP_DEVICE_FAILED) // test for root permission
        {
            perror("Failed to map control register");
            return NULL;
        }
    out8(dir_ptr,0x00);
    while(1)
    {
        out8(dc_ptr,0x03);  //output a high
        on_time=(20000*(Duty/100));    //Motor_pos1 hold a percent value. For example 15/100 =.15.  15% of 20000 or 20ms
        printf("+ve Cycle is for %f;Duty Cycle is %f\n",on_time,Duty);
        usleep(on_time);
        out8(dc_ptr,0x00);  //output a low
        usleep(20000-on_time);      // Keep 20ms period
        pthread_exit(NULL);
    }
return NULL;
}
