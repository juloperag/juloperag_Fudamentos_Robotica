import tkinter as tk

class App:
    def __init__(self, root):
        self.root = root
        self.root.geometry("600x400")

        # Frame principal con tamaño fijo y posicionado
        self.frame = tk.Frame(root, width=300, height=200, padx=10, pady=10, bg="lightgray")
        self.frame.place(x=50, y=50)
        self.frame.grid_propagate(False)

        # Entradas para filas y columnas
        self.rows_entry = tk.Entry(root)
        self.rows_entry.place(x=50, y=300)
        self.rows_entry.insert(0, "3")

        self.cols_entry = tk.Entry(root)
        self.cols_entry.place(x=150, y=300)
        self.cols_entry.insert(0, "3")

        # Botón para actualizar filas y columnas
        self.update_button = tk.Button(root, text="Actualizar", command=self.update_grid)
        self.update_button.place(x=250, y=295)

        # Inicializa el grid con el valor por defecto
        self.rows = 3
        self.cols = 3
        self.create_grid()
        self.create_buttons()

    def create_grid(self):
        for row in range(self.rows):
            self.frame.rowconfigure(row, weight=1)
        for col in range(self.cols):
            self.frame.columnconfigure(col, weight=1)

    def create_buttons(self):
        for widget in self.frame.winfo_children():
            widget.destroy()
        
        for row in range(self.rows):
            for col in range(self.cols):
                button = tk.Button(self.frame, text=f"Button {row*self.cols+col+1}")
                button.grid(row=row, column=col, sticky="nsew", padx=5, pady=5)

    def update_grid(self):
        try:
            self.rows = int(self.rows_entry.get())
            self.cols = int(self.cols_entry.get())

            # Reinicia la configuración del grid
            for row in range(10):  # Asumiendo un máximo de 10 filas y columnas
                self.frame.rowconfigure(row, weight=0)
            for col in range(10):
                self.frame.columnconfigure(col, weight=0)

            self.create_grid()
            self.create_buttons()
        except ValueError:
            print("Please enter valid integers for rows and columns")

if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()