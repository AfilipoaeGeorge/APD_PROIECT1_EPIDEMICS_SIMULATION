#include "epidemics_project.h"
#include "time_function.h"
// #define DEBUG 1

int main(int argc, char** argv){

#ifdef DEBUG
    printf("number of arg = %d\n",argc);
#endif
    if(argc != 4){ //verificam daca a fost dat numarul corect de argumente in linia de comanda
        perror("Numar gresit de argumente! Se utilizeaza: <./p> <simulation time> <input file> <thread number>\n");
        exit(EXIT_FAILURE);
    }

    TOTAL_SIMULATION_TIME = atoi(argv[1]); // luam  timpul de simulare
    
#ifdef DEBUG
    printf("total simulation time = %d\n",TOTAL_SIMULATION_TIME);
#endif

    char* input_file = argv[2]; //luam  numele fisierului de intrare
#ifdef DEBUG
    printf("input file = %s\n",input_file);
#endif

    THREAD_NUMBER = atoi(argv[3]); // luam numarul de thread-uri
#ifdef DEBUG
    printf("thread number = %d\n",THREAD_NUMBER);
#endif

    struct timespec start_time, finish_time;
	double elapsed_serial,elapsed_parallel;
    char output_serial_file[512];
    char output_parallel_file[512];

    read_input_file(input_file); //apelam functia de citire a datelor

#ifdef DEBUG
    print_vector();
#endif
    start(&start_time); //pornim cronometrul
    serial_simulate(); //apelam simularea seriala

#ifdef DEBUG
    printf("people vector after serial simulation\n");
    print_vector();
#endif
    end(&finish_time); //oprim cronometrul
    elapsed_serial = calculate_time(start_time,finish_time); //calculam timpul in care s-a facut simularea seriala
	printf("time serial =%f, simulation time=%d, no of persons=%d, max_x=%d, max_y=%d\n", elapsed_serial,TOTAL_SIMULATION_TIME,N,MAX_X_COORD,MAX_Y_COORD); //afisam timpul  de simulare seriala si alte informatii
    
#ifdef DEBUG
    printf("people for print in output_serial_file\n");
    print_vector();
#endif
    char* input = strtok(input_file,".");
#ifdef DEBUG
    printf("input file name without .txt = %s\n",input);
#endif
    snprintf(output_serial_file, sizeof(output_serial_file), "%s_serial_out_%d_%d.txt", input, TOTAL_SIMULATION_TIME,THREAD_NUMBER);
#ifdef DEBUG
    printf("output serial file name = %s\n",output_serial_file);
#endif
    check_or_create_file(output_serial_file);
    write_output_file(output_serial_file); //apelam functia de afisare a datelor pentru simularea seriala

#ifdef DEBUG
    printf("people before memcpy\n");
    print_vector();
#endif
    memcpy(people_serial, people, N * sizeof(Person)); //copiem in vectorul people_serial starile starile din vectorul people dupa ce a avut loc simularea seriala
    memcpy(people,people_parallel, N * sizeof(Person)); //schimbam la starile initiale vectorul people pentru a putea porni simularea paralela

#ifdef DEBUG
    printf("people after memcpy\n");
    print_vector();
#endif

    start(&start_time); //pornim cronometrul
    parallel_simulate(); //apela simularea paralela
#ifdef DEBUG
    printf("people vector after parallel simulation\n");
    print_vector();
#endif
    end(&finish_time);//oprim cronometrul
    elapsed_parallel = calculate_time(start_time,finish_time); //calculam timpul in care s-a facut simularea paralela
	printf("time parallel=%f, simulation time=%d, no of persons=%d, max_x=%d, max_y=%d\n", elapsed_parallel,TOTAL_SIMULATION_TIME,N,MAX_X_COORD,MAX_Y_COORD); //afisam timpul de simulare paralela si alte informatii

#ifdef DEBUG
    printf("people for print in output_parallel_file\n");
    print_vector();
#endif

    snprintf(output_parallel_file, sizeof(output_parallel_file), "%s_parallel_out_%d_%d.txt", input, TOTAL_SIMULATION_TIME,THREAD_NUMBER);
#ifdef DEBUG
    printf("output parallel file name = %s\n",output_parallel_file);
#endif
    check_or_create_file(output_parallel_file);
    write_output_file(output_parallel_file); //apelam functia de afisare a datelor pentru simularea paralela

    printf("speedup=%f\n",elapsed_serial/elapsed_parallel); //afisam speedup-ul

    print_speedup_and_time_in_output_file(calculate_speedup(elapsed_serial,elapsed_parallel),elapsed_serial,elapsed_parallel);

    memcpy(people_parallel, people, N * sizeof(Person)); //copiem din people in people_parallel starile dupa executia simularii paralele

    print_same_result();

    print_diff_time(elapsed_serial, elapsed_parallel);

    //eliberam memoria ocupata de vectorii de persoane
    free_memory();

    return 0;
}