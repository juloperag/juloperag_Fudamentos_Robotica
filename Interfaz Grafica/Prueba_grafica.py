import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt

# Inicializar las listas de coordenadas X y Y
x_values = []
y_values = []

# Función para agregar un punto a la gráfica
def agregar_punto(x, y):
    x_values.append(x)
    y_values.append(y)
    plt.scatter(x, y, color="blue")  # Agregar el nuevo punto sin limpiar
    canvas.draw()  # Redibujar la gráfica en el canvas

# Configuración de la interfaz gráfica
root = tk.Tk()
root.title("Interfaz gráfica con matplotlib")

# Crear una figura de matplotlib
fig, ax = plt.subplots()
ax.set_xlabel("Coordenada X")
ax.set_ylabel("Coordenada Y")
ax.set_title("Gráfico de puntos")

# Crear un lienzo para dibujar la gráfica en la interfaz de tkinter
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()

# Simular la entrada de puntos desde la consola
try:
    while True:
        # Solicitar las coordenadas X y Y desde la consola
        x = float(input("Ingrese la coordenada X: "))
        y = float(input("Ingrese la coordenada Y: "))
        agregar_punto(x, y)
except KeyboardInterrupt:
    print("Aplicación finalizada.")

root.mainloop()
