import matplotlib.pyplot as plt

# Inicialización de listas para X e Y
X = []
Y = []

# Crear la figura y el eje
plt.ion()  # Modo interactivo activado
fig, ax = plt.subplots()

# Función para agregar un punto y actualizar la gráfica
def agregar_punto(x, y):
    X.append(x)
    Y.append(y)
    ax.clear()  # Limpiar la gráfica anterior
    ax.plot(X, Y, linestyle='-', color='b')  # Graficar los puntos y conectarlos con una línea
    plt.draw()  # Dibujar la gráfica actualizada
    plt.pause(0.01)  # Pausa breve para que la gráfica se actualice

# Ejemplo de cómo agregar puntos desde la consola
while True:
    try:
        x = float(input("Ingresa la coordenada X: "))
        y = float(input("Ingresa la coordenada Y: "))
        agregar_punto(x, y)
    except ValueError:
        print("Por favor, ingresa un número válido.")
    except KeyboardInterrupt:
        print("\nFinalizando la gráfica.")
        break

plt.ioff()  # Desactivar el modo interactivo
plt.show()  # Mostrar la gráfica final
