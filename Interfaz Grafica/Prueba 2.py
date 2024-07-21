import tkinter as tk

# Tu string con caracteres "\n" para saltos de línea
help_menu = "Help Menu: \n 1) help  ---Imprime lista de comandos. \n 2) frequency #  ---Definir los 3 valores de las frecuencias para el study en Hz. \n 3) motor # ---Elige el tipo de motor para el test, 1:Derecho, 2:Izquierdo \n 4) on --- Enciende el motor seleccionado en la prueba estatica \n"

# Crear una ventana de Tkinter
root = tk.Tk()
root.title("Help Menu")

# Crear un widget de Text
text_widget = tk.Text(root, wrap='word')
text_widget.pack(expand=True, fill='both')

# Insertar el string en el widget de Text
text_widget.insert(tk.END, help_menu)

# Desactivar la edición del widget de Text
text_widget.config(state=tk.DISABLED)

# Iniciar el bucle principal de la interfaz gráfica
root.mainloop()