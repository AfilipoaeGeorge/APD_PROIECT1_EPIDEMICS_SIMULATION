import matplotlib.pyplot as plt
import numpy as np

# Datele pentru numărul de persoane, numărul de threaduri, timpul de simulare, și valorile de speedup
data = [
    (10000, 50, 2, 1.862031), (10000, 50, 4, 3.438894), (10000, 50, 8, 5.271473),
    (10000, 100, 2, 1.921702), (10000, 100, 4, 3.317712), (10000, 100, 8, 5.336082),
    (10000, 150, 2, 1.915232), (10000, 150, 4, 3.124526), (10000, 150, 8, 4.825254),
    (10000, 200, 2, 1.788575), (10000, 200, 4, 3.199102), (10000, 200, 8, 4.551129),
    (10000, 500, 2, 1.809839), (10000, 500, 4, 3.175704), (10000, 500, 8, 4.339918),
    (20000, 50, 2, 1.916028), (20000, 50, 4, 3.472632), (20000, 50, 8, 4.586383),
    (20000, 100, 2, 1.856400), (20000, 100, 4, 3.175041), (20000, 100, 8, 4.607970),
    (20000, 150, 2, 1.863808), (20000, 150, 4, 3.320318), (20000, 150, 8, 4.493193),
    (20000, 200, 2, 1.862305), (20000, 200, 4, 3.170480), (20000, 200, 8, 4.429472),
    (20000, 500, 2, 1.836479), (20000, 500, 4, 3.130659), (20000, 500, 8, 4.505608),
    (50000, 50, 2, 1.762614), (50000, 50, 4, 3.118968), (50000, 50, 8, 4.474842),
    (50000, 100, 2, 1.848257), (50000, 100, 4, 3.031768), (50000, 100, 8, 4.300880),
    (50000, 150, 2, 1.805416), (50000, 150, 4, 2.772834), (50000, 150, 8, 4.407634),
    (50000, 200, 2, 1.697311), (50000, 200, 4, 3.041225), (50000, 200, 8, 4.125431),
    (50000, 500, 2, 1.820890), (50000, 500, 4, 2.875818), (50000, 500, 8, 4.261512),
    (100000, 50, 2, 1.805642), (100000, 50, 4, 2.896602), (100000, 50, 8, 4.354933),
    (100000, 100, 2, 1.796712), (100000, 100, 4, 2.908014), (100000, 100, 8, 4.239175),
    (100000, 150, 2, 1.782243), (100000, 150, 4, 2.847447), (100000, 150, 8, 4.240773),
    (100000, 200, 2, 1.787641), (100000, 200, 4, 2.934778), (100000, 200, 8, 4.226317),
    (100000, 500, 2, 1.764665), (100000, 500, 4, 2.826229), (100000, 500, 8, 4.356712),
    (500000, 50, 2, 1.764298), (500000, 50, 4, 3.156328), (500000, 50, 8, 4.467213),
    (500000, 100, 2, 1.879251), (500000, 100, 4, 3.295412), (500000, 100, 8, 4.502871),
    (500000, 150, 2, 1.843625), (500000, 150, 4, 3.278941), (500000, 150, 8, 4.438725),
    (500000, 200, 2, 1.891405), (500000, 200, 4, 3.287564), (500000, 200, 8, 4.491052),
    (500000, 500, 2, 1.921374), (500000, 500, 4, 3.305897), (500000, 500, 8, 4.523764)
]

# Configurații de culori pentru diferitele combinații
colors = {
    50: {2: 'red', 4: 'blue', 8: 'yellow'},
    100: {2: 'green', 4: 'orange', 8: 'purple'},
    150: {2: 'cyan', 4: 'magenta', 8: 'brown'},
    200: {2: 'gray', 4: 'pink', 8: 'lime'},
    500: {2: 'lightblue', 4: 'lightgreen', 8: 'lightcoral'}
}

# Funcția pentru a desena graficul
def plot_speedup(data):
    # Organizează datele în funcție de numărul de persoane
    groups = {}
    for entry in data:
        num_people = entry[0]
        if num_people not in groups:
            groups[num_people] = []
        groups[num_people].append(entry)

    # Creează un grafic pentru fiecare grup
    for num_people, group_data in groups.items():
        plt.figure(figsize=(10, 6))  # Ajustarea dimensiunii graficului

        # Organizează datele în funcție de numărul de iterații
        for sim_time in sorted(set(d[1] for d in group_data)):
            thread_speedup = {d[2]: d[3] for d in group_data if d[1] == sim_time}
            threads = sorted(thread_speedup.keys())
            speedups = [thread_speedup[t] for t in threads]

            # Desenează linia pentru această combinație
            plt.plot(threads, speedups, marker='o', color=colors[sim_time][threads[0]], label=f"{num_people} persoane, {sim_time} iterații")

        plt.title(f"Speedup în funcție de numărul de threaduri (pentru {num_people} persoane)")
        plt.xlabel("Număr de Threaduri")
        plt.ylabel("Speedup")
        plt.xticks([2, 4, 8])
        plt.grid(visible=True, linestyle='--', alpha=0.5)
        plt.legend()

        # Salvează graficul ca fișier PNG
        plt.savefig(f"speedup_{num_people}_persoane.png", format='png')

        # Arată graficul
        plt.show()

# Desenează graficele
plot_speedup(data)
