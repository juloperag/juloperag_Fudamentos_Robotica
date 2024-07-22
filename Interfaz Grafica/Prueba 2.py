import tkinter as tk

# Función para restablecer el color de fondo al valor predeterminado
def reset_background():
    button.config(background=default_bg)

# Crear la ventana principal
root = tk.Tk()

# Crear un botón y almacenar su color de fondo predeterminado
button = tk.Button(root, text="Botón")
default_bg = button.cget("background")
button.pack()

# Cambiar el color de fondo del botón a rojo
button.config(background="red")

# Crear un botón para restablecer el color de fondo al valor predeterminado
reset_button = tk.Button(root, text="Restablecer color", command=reset_background)
reset_button.pack()

root.mainloop()