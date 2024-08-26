import tkinter as tk
from tkinter import messagebox
import serial

class SerialApp:
    def __init__(self, master):
        self.master = master
        self.master.title("Conexión Serial")
        
        # Configuración inicial del puerto
        self.puerto = 'COM5'
        self.velocidad = 9600
        self.ser = serial.Serial(None)

        # Botón para conectar
        self.connect_button = tk.Button(master, text="Conectar", command=self.conectar_puerto)
        self.connect_button.pack(pady=10)

        # Botón para desconectar
        self.disconnect_button = tk.Button(master, text="Desconectar", command=self.desconectar_puerto, state=tk.DISABLED)
        self.disconnect_button.pack(pady=10)

    def conectar_puerto(self):
        try:
            if self.ser is not self.ser.is_open:
                self.ser = serial.Serial(port=self.puerto, baudrate=self.velocidad, timeout=1)
                messagebox.showinfo("Conexión Serial", f"Conectado al puerto {self.puerto}")
                self.connect_button.config(state=tk.DISABLED)
                self.disconnect_button.config(state=tk.NORMAL)
        except serial.SerialException as e:
            messagebox.showerror("Error", f"No se pudo conectar al puerto {self.puerto}\nError: {str(e)}")

    def desconectar_puerto(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            messagebox.showinfo("Conexión Serial", f"Desconectado del puerto {self.puerto}")
            self.connect_button.config(state=tk.NORMAL)
            self.disconnect_button.config(state=tk.DISABLED)

    def on_closing(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.master.destroy()

# Crear la ventana principal de Tkinter
root = tk.Tk()
app = SerialApp(root)

# Manejar el cierre de la ventana
root.protocol("WM_DELETE_WINDOW", app.on_closing)

# Ejecutar la aplicación
root.mainloop()

