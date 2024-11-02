#!/bin/bash
thread_number_vector=("2" "4" "8")
total_simulation_time_vector=("50" "100" "150" "200" "500")
input_file_name=("epidemics10K.txt" "epidemics20K.txt" "epidemics50K.txt" "epidemics100K.txt" "epidemics500K.txt")
echo "Începem executia!"
# Parcurgem toate combinațiile și executăm programul secvențial
for input_file in "${input_file_name[@]}"; do
    echo "Procesam fisierul: $input_file"
    for simulation_time in "${total_simulation_time_vector[@]}"; do
        echo "Timp de simulare: $simulation_time"
        for thread_number in "${thread_number_vector[@]}"; do
            echo "Numar de thread-uri: $thread_number"
            echo "Rulam: ./p $simulation_time $input_file $thread_number"
            ./p "$simulation_time" "$input_file" "$thread_number"
        done
    done
done

echo "Executia s-a terminat."
