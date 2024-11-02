#include "epidemics_project.h"
#include "time_function.h"

int TOTAL_SIMULATION_TIME;//numarul de repetari/iteratii
int MAX_X_COORD, MAX_Y_COORD; //maximul coordonatelor
int N; // numărul de persoane
int THREAD_NUMBER;//numarul de thread-uri pe care le folosim la simularea paralela
Person *people; //vectorul de persoane pe care o sa il alocam dinamic in functia de citire a datelor din fisier
Person *people_parallel; //vector in care memoram starea si toate informatiile initiale ale persoanelor pentru a putea apela simularea paralela dupa cea seriala
Person* people_serial;
pthread_barrier_t b1_barrier, b2_barrier, b3_barrier, b4_barrier; //bariere pentru a sincroniza operatiile in simularea paralela
pthread_mutex_t mutex;

// Funcție pentru citirea datelor din fișierul de intrare
void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r"); //deschidem fisierul pentru a citi datele si pentru a construi vectorul de persoane
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului de intrare!\n");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d %d", &MAX_X_COORD, &MAX_Y_COORD); //citim de pe prima linie maximul coordonatelor
    fscanf(file, "%d", &N); //de pe a doua linie citim numarul de persoane 

    people = (Person*)malloc(N * sizeof(Person)); //alocarea dinamica a vectorului
    people_serial = (Person*)malloc(N * sizeof(Person));
    people_parallel = (Person*)malloc(N * sizeof(Person));

    //parcurgem fisierul linie cu linie, construind astfel si vectorul de persoane
    for (int i = 0; i < N; i++) {
        fscanf(file, "%d %d %d %d %d %d", &people[i].id, &people[i].x, &people[i].y,&people[i].current_status, &people[i].direction,&people[i].amplitude);
        people[i].future_x = people[i].future_y = 0; //initializam pozitia viitoare
        if(people[i].current_status == SUSCEPTIBLE){ //daca initial e susceptibil ramane asa
            people[i].future_status = SUSCEPTIBLE;
        }
        else if(people[i].current_status == INFECTED && INFECTED_DURATION == 1){ //daca e initial infectat dar timpul de infectare este de 1, atunci viitorul status este de imun
            people[i].future_status = IMMUNE;
        }
        else if(people[i].current_status == INFECTED && INFECTED_DURATION > 1){ //daca e initial infectat dar timpul de infectare este >0 atunci acesta ramane infectat
            people[i].future_status = INFECTED;
        }
        if(people[i].current_status == INFECTED){ //daca e initial infectat, punem infection count = 1 pentru a tine cont ca a avut loc o infectare
            people[i].infection_count = 1;
        }
        else{
            people[i].infection_count = 0; //altfel initializam cu 0
        }
        people[i].infected_time = INFECTED_DURATION; //initializam timpul de infectare
        people[i].immune_time = IMMUNE_DURATION; //initializam timpul de imunitate

        //copiem starea intr-un alt vector pentru a putea apela si paralel pe pozitiile si pe statusurile initiale pe care le-am citit din fisier
        people_serial[i] = people[i];
        people_parallel[i] = people[i];
    }

    //inchidem fisierul din care am citit datele
    if(fclose(file)  != 0){
        perror("Eroare la inchiderea fisierului de intrare!\n");
        exit(EXIT_FAILURE);
    }
}

// Funcție pentru salvarea rezultatelor într-un fișier de ieșire
void write_output_file(const char *filename) {
    FILE *file = fopen(filename, "w"); //deschidem fisierul pentru a printa  rezultatele
    if (!file) {
        perror("Eroare la deschiderea fișierului de ieșire");
        exit(EXIT_FAILURE);
    }

    //parcurgem vectorul si afisam cateva detalii despre fiecare persoana
    for (int i = 0; i < N; i++) {
        fprintf(file, "Person %d: (%d, %d); Status: %d; Infection Count: %d;\n",people[i].id, people[i].x, people[i].y, people[i].current_status, people[i].infection_count);
    }

    //inchidem fisierul in care am scris rezultatele
    if(fclose(file) != 0){
        perror("Eroare la inchiderea fisierului de iesire!\n");
        exit(EXIT_FAILURE);
    }   
}

//actualizam coordonatele si statusul pentru iteratia urmatoare
void update_coordinates_and_status(int start, int end) {
    for (int j = start; j < end; j++) { //parcurgem vectorul de la start pana la end(asta in functie daca facem de la serial sau paralel, pentru a putea folosi tot functia asta pentru ambele simulari)
        //printf("update persoana %d\n",j);
        people[j].x = people[j].future_x; //actualizam x
        people[j].y = people[j].future_y; //actualizam y
        people[j].current_status = people[j].future_status; //actualizam statusul
    }
}

//verificam daca sunt persoane la aceleasi pozitii
//daca da, iar una din ele este susceptibila atunci ii actualizam viitorul status si ii incrementam infection count-ul fiind infectat
void check_interactions_and_update_future_statuses_for_new_infections(int start, int end, bool parallel) {
    for (int j = start; j < end; j++) {  //parcurgem de la start la end, pentru a putea folosi functia in ambele simulari
    //printf("check persoana %d\n",j);
        if (people[j].current_status == SUSCEPTIBLE) { //numai persoanele susceptibile se pot infecta
            for (int k = 0; k < N; k++) { //acea persoana susceptibila o verificam cu intregul vector pentru a vedea
                if (j != k && people[k].current_status == INFECTED && people[j].x == people[k].x && people[j].y == people[k].y) {
                    // if(parallel == true){
                    //     pthread_mutex_lock(&mutex);
                    // }
                    if (people[j].future_status != INFECTED) {
                        people[j].future_status = INFECTED;
                        people[j].infection_count++;
                        
                        people[j].infected_time = INFECTED_DURATION;
                        break; //daca a fost susceptibil si va deveni deja infectat, nu mai are rost sa verificam vectorul de persoane pana la sfarsit pentru ca deja e infectat
                    }
                    // if(parallel == true){
                    //     pthread_mutex_unlock(&mutex);
                    // }
                }
            }
        }
    }
}

//calculam viitoarele coordonate ale persoanelor
void calculate_the_future_coordinates_and_check_interactions_and_update_future_statuses_for_new_infections(int start, int end) {
    for (int j = start; j < end; j++) { //mergem tot de la start pana la end
    //printf("future coodinates persoana %d\n",j);
        if (people[j].direction == 0) { //daca merge spre nord(N)
            if ((people[j].y + people[j].amplitude) > MAX_Y_COORD) { //si depaseste xoordonata maxima Y
                people[j].direction = 1; //isi schimba directia inspre sud(S)
                people[j].future_y = MAX_Y_COORD - ((people[j].y + people[j].amplitude) - MAX_Y_COORD); //ii actualizam pozitia y
            } else {
                people[j].future_y = people[j].y + people[j].amplitude; //daca nu depaseste, atunci doar adaugam amplitudinea
            }
            people[j].future_x = people[j].x; //coordonata x ramane la fel
        } else if (people[j].direction == 1) { //daca merge spre sud(S)
            if ((people[j].y - people[j].amplitude) < 0) { //si scade y sub 0
                people[j].direction = 0; //isi schimba directia spre nord(N)
                people[j].future_y = people[j].amplitude - people[j].y; //actualizeaza pozitia y
            } else {
                people[j].future_y = people[j].y - people[j].amplitude;  //daca nu scade sub 0, atunci doar scadem amplitudinea
            }
            people[j].future_x = people[j].x;  //coordonata x ramane la fel
        } else if (people[j].direction == 2) {  //daca merge spre est(E)
            if ((people[j].x + people[j].amplitude) > MAX_X_COORD) {  //si depaseste coordonata maxima X
                people[j].direction = 3;   //isi schimba directia spre vest(V)
                people[j].future_x = MAX_X_COORD - ((people[j].x + people[j].amplitude) - MAX_X_COORD);   //actualizeaza pozitia x
            } else {
                people[j].future_x = people[j].x + people[j].amplitude;   //daca nu depaseste, atunci doar adaugam amplitudinea
            }
            people[j].future_y = people[j].y;    //coordonata y ramane la fel
        } else if (people[j].direction == 3) {   //daca merge spre vest(V)
            if ((people[j].x - people[j].amplitude) < 0) {    //si scade x sub 0
                people[j].direction = 2;    //isi schimba directia spre est(E)
                people[j].future_x = people[j].amplitude - people[j].x;     //actualizeaza pozitia x
            } else {
                people[j].future_x = people[j].x - people[j].amplitude;     //daca nu scade sub 0, atunci doar scadem amplitud  
            }
            people[j].future_y = people[j].y;      //coordonata y ramane la fel
        }

        if (people[j].current_status == INFECTED && people[j].infected_time > 0) { //daca e infectat si mai are timp de infectare atunci decrementam
            people[j].infected_time--;
            if (people[j].infected_time == 0) {  //daca timpul de infectare devine 0, atunci el devine imun
                people[j].future_status = IMMUNE;  // Devine imun după infectare
                people[j].immune_time = IMMUNE_DURATION;
            }
        } else if (people[j].current_status == IMMUNE && people[j].immune_time > 0) { //daca e imun si mai are timp, decrementam timpul de imunitate
            people[j].immune_time--;
            if (people[j].immune_time == 0) { //daca timpul de imunitate devine 0, atunci el devine susceptibil
                people[j].future_status = SUSCEPTIBLE;
            }
        }
    }
}

//functia in care are loc simularea seriala
void serial_simulate() {
    printf("serial\n");
    int start = 0; //pornim de la 0 fiind vorba de simulare seriala
    int end = N; //terinam cu end la N(dar vom folosi <end deci vom lucra in intervalul 0,N-1) fiind vorba de simulare seriala
    for (int i = 0; i < TOTAL_SIMULATION_TIME; i++) { //repetem pentru  fiecare moment din timpul simulatiei
        // printf("Iterația = %d\n", i);
        calculate_the_future_coordinates_and_check_interactions_and_update_future_statuses_for_new_infections(start, end);
        check_interactions_and_update_future_statuses_for_new_infections(start, end, false);
        update_coordinates_and_status(start, end);
    }
}

//functia de care avem nevoie in simularea paralela pentru a putea fi data thread-urilor fiind void*
void* thread_function(void* rank) {
    long my_rank = *(int*)rank; //numarul thread-ului
    int local_n = N / THREAD_NUMBER; //marimea locala pe care lucreaza fiecare thread, cunoastem ca N se imparte la THREAD_NUMBER
    int start = my_rank * local_n; //pornim de la  numarul thread-ului * marimea locala
    int end = (my_rank + 1) * local_n;  //terminam cu numarul thread-ului + 1 * marimea locala

    for (int i = 0; i < TOTAL_SIMULATION_TIME; i++) { //repetam de  fiecare data pentru fiecare moment din timpul simularii
        // printf("iteratia paralela %d\n", i);
        pthread_barrier_wait(&b1_barrier); //prima bariera pentru a aduce toate thread-urile la acleasi moment la inceputul fiecarei iteratii

        calculate_the_future_coordinates_and_check_interactions_and_update_future_statuses_for_new_infections(start, end);
        pthread_barrier_wait(&b2_barrier); //a doua bariera pentru ca toate thread-urile sa calculeze coordonatele viitoare

        check_interactions_and_update_future_statuses_for_new_infections(start, end, true);
        pthread_barrier_wait(&b3_barrier); // a patra bariera pentru ca toate thread-urile sa verifice interactiunile si  sa actualizeze statutul pentru viitorii infectati

        update_coordinates_and_status(start, end);
        pthread_barrier_wait(&b4_barrier); // a cincea bariera pentru ca toate thread-urile sa actualizeze coordonate si statusul
    }
    return NULL;
}

//functia specifica simularii paralele
void parallel_simulate() {
    printf("paralel\n");
    pthread_t* thread_handles = malloc(THREAD_NUMBER * sizeof(pthread_t)); //vector de thread-uri
    int* tid = malloc(THREAD_NUMBER * sizeof(int)); //vector in care tinem minte id-urile thread-urilor
    //initializam barierele
    pthread_barrier_init(&b1_barrier, NULL, THREAD_NUMBER);
    pthread_barrier_init(&b2_barrier, NULL, THREAD_NUMBER);
    pthread_barrier_init(&b3_barrier, NULL, THREAD_NUMBER);
    pthread_barrier_init(&b4_barrier, NULL, THREAD_NUMBER);
    pthread_mutex_init(&mutex, NULL);

    for (int thread = 0; thread < THREAD_NUMBER; thread++) { //cream thread-urile 
        tid[thread] = thread;
        pthread_create(&thread_handles[thread], NULL, thread_function, &tid[thread]);
    }

    for (int thread = 0; thread < THREAD_NUMBER; thread++) {// dam join thread-urilor
        pthread_join(thread_handles[thread], NULL);
    }
    //eliberam memoria de la cei 2 vectori
    free(thread_handles);
    free(tid);
    //distrugem barierele
    pthread_barrier_destroy(&b1_barrier);
    pthread_barrier_destroy(&b2_barrier);
    pthread_barrier_destroy(&b3_barrier);
    pthread_barrier_destroy(&b4_barrier);
    pthread_mutex_destroy(&mutex);
}

void print_vector(){
    for(int i = 0;i < N; i++){
        printf("persoana %d, x=%d, y=%d, status=%d\n",people[i].id, people[i].x,people[i].y,people[i].current_status);
    }
}

//in cazul in care nu exista fisierul -> filePath il cream cu drepturi si cu permisiuni de 0777
void check_or_create_file(char *filePath)
{
    int foFilePath = open(filePath, O_WRONLY);
    if (foFilePath == -1)
    {
        creat(filePath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
        chmod(filePath, 0777);
    }
    else{
        if(close(foFilePath) == -1){
            perror("Error: close in check_or_create_file()\n");
            exit(1);
        }
    }
}

bool are_equal(Person a, Person b) {
    return (a.id == b.id &&
            a.x == b.x &&
            a.y == b.y &&
            a.future_x == b.future_x &&
            a.future_y == b.future_y &&
            a.infected_time == b.infected_time &&
            a.immune_time == b.immune_time &&
            a.current_status == b.current_status &&
            a.future_status == b.future_status &&
            a.direction == b.direction &&
            a.amplitude == b.amplitude &&
            a.infection_count == b.infection_count);
}


bool same_result(){
    bool same = true;
    for( int i = 0; i < N; i++){
        if(are_equal(people_serial[i],people_parallel[i]) == false){
            printf("Step= %d, people information are not equal\n",i);
            same = false;
        }
    }
    return same;
}

void print_same_result(){
    if(same_result() == false){
        printf("The serial and parallel versions not produce the same result.\n");
    }
    else{
        printf("The serial and parallel versions produce the same result.\n");
    }
}

void print_speedup_and_time_in_output_file(double speedup,double serial_time, double parallel_time){
    FILE* result;
    check_or_create_file("speedup_result.txt");
    if((result = fopen("speedup_result.txt","a")) == NULL){
        perror("Error at open result speedup file\n");
        exit(-1);
    }

    fprintf(result,"speedup=%f, number of people=%d, number thread=%d, total simulation time=%d, max_x=%d, max_y=%d\t serial time=%f, parallel time=%f\n", speedup, N, THREAD_NUMBER,TOTAL_SIMULATION_TIME,MAX_X_COORD,MAX_Y_COORD,serial_time,parallel_time);

    if(fclose(result) != 0){
        perror("Error at close result speedup file\n");
        exit(-1);
    }
}

void print_diff_time(double serial_time, double parallel_time){
    printf("diferenta dintre timpul serial si cel paralel este=%f\n",calculate_diff(serial_time, parallel_time));
}

void free_memory(){
    free(people);
    free(people_serial);
    free(people_parallel);
}