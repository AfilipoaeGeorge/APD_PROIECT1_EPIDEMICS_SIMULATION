# Epidemics Simulation (Serial & Parallel)

This project simulates the spread of an epidemic in a moving population on a 2D grid. It runs the simulation in two modes:
- **Serial** execution
- **Parallel** execution using **pthreads**

The program measures execution time for both versions and prints the **speedup** (serial_time / parallel_time). It also writes results to output files.

## 🧩 Simulation Model

Each person is in one of the following states:
- **INFECTED (0)**: infected, can transmit the disease
- **SUSCEPTIBLE (1)**: healthy, can become infected
- **IMMUNE (2)**: temporarily immune after infection

The simulation runs for a number of iterations (`simulation_time`) and applies these rules:
- **Movement**: people move (N/S/E/W) with a certain amplitude on the 2D grid.
- **Infection**: if two people meet at the same position and one is INFECTED while the other is SUSCEPTIBLE, the SUSCEPTIBLE person becomes INFECTED.
- **Infection duration**: a person stays INFECTED for `INFECTED_DURATION` iterations (e.g., 2).
- **Immunity**: after infection, a person becomes IMMUNE for `IMMUNE_DURATION` iterations (e.g., 5).
- **Border collisions**: at grid borders, the direction is reversed/adjusted.

## Build
- gcc -O2 -Wall -o p main_epidemics_project.c epidemics_project.c time_function.c -lpthread -lm
## Run
- ./p <simulation_time> <input_file> <thread_number>

Example:
./p 100 epidemics10K.txt 4

Outputs:
- <input>_serial_out_<simulation_time>_<thread_number>.txt
- <input>_parallel_out_<simulation_time>_<thread_number>.txt
- speedup_result.txt
