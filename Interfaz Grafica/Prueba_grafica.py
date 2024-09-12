import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import threading

class PlotApp:
    def __init__(self, root):
        self.root = root
        self.root.geometry("600x400")
        
        # Inicializar listas de coordenadas
        self.x_values = []
        self.y_values = []
        self.max_points = 10  # Número máximo de puntos
        
        # Crear la figura de Matplotlib
        self.fig, self.ax = plt.subplots()
        self.ax.set_xlabel("Coordenada X",fontsize=6)
        self.ax.set_ylabel("Coordenada Y",fontsize=6)
        self.ax.tick_params(axis='both', which='major', labelsize=6)
        self.ax.grid(True)  
        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        # Iniciar el hilo para leer desde la consola
        self.start_console_input_thread()

    # Función para agregar puntos a la gráfica o eliminarlos
    def add_point(self, x, y):
        # Agregar coordenadas
        self.x_values.append(x)
        self.y_values.append(y)          
        # Verificar si se ha superado el número máximo de puntos
        if len(self.x_values) > self.max_points:
            self.x_values.pop(0)  # Eliminar el primer punto      
            self.y_values.pop(0)
            self.ax.lines[0].remove()  # Eliminar la primera línea añadida
            self.ax.relim()           # Recalcular los límites
            self.ax.autoscale_view()  # Ajustar la vista
        # Conectar el último punto con el penúltimo solo si hay más de 1 punto
        if len(self.x_values) > 1:
            self.ax.plot(self.x_values[-2:], self.y_values[-2:], linestyle='-', color='b')
        # Redibujar la gráfica en el canvas
        self.canvas.draw()

    # Función que se ejecuta en un hilo para leer desde la consola
    def read_from_console(self):
        while True:
            try:
                # Leer las coordenadas desde la terminal
                mesg = input("Ingresa las coordenadas (x y): ")
                values_num = mesg.split()
                x = float(values_num[0])
                y = float(values_num[1])
                self.add_point(x, y)
            except ValueError:
                print("Error: Ingresa dos números separados por un espacio.")
            except IndexError:
                print("Error: Ingresa ambos valores x y y.")
    
    # Iniciar un hilo separado para la entrada de la consola
    def start_console_input_thread(self):
        console_thread = threading.Thread(target=self.read_from_console, daemon=True)
        console_thread.start()

# Crear la ventana principal
root = tk.Tk()
app = PlotApp(root)
root.mainloop()