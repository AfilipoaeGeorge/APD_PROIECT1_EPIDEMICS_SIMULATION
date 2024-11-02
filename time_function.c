#include "time_function.h"

void start(struct timespec* start_time){
    clock_gettime(CLOCK_MONOTONIC, start_time);
}

void end(struct timespec* finish_time){
    clock_gettime(CLOCK_MONOTONIC, finish_time);
}

double calculate_time(struct timespec start_time,struct timespec end_time){
    double elapsed_time;
    elapsed_time = (end_time.tv_sec - start_time.tv_sec);
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    return elapsed_time;
}

double calculate_speedup(double serial_time,double parallel_time){
    return  serial_time / parallel_time;
}

double calculate_diff(double serial_time,double parallel_time){
    return  serial_time - parallel_time;
}