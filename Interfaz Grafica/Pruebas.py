import tkinter as tk

def create_grid(frame, rows, cols):
    for row in range(rows):
        frame.rowconfigure(row, weight=1)
    for col in range(cols):
        frame.columnconfigure(col, weight=1)

def create_buttons(frame, rows, cols):
    for row in range(rows):
        for col in range(cols):
            button = tk.Button(frame, text=f"Button {row*cols+col+1}")
            button.grid(row=row, column=col, sticky="nsew")

def main():
    root = tk.Tk()
    root.geometry("600x400")

    # Especifica los márgenes del frame
    frame = tk.Frame(root, width=300, height=200, padx=10, pady=10, bg="lightgray")
    frame.place(x=50, y=50)  # Posiciona el frame en (50, 50)
    
    # Establece el tamaño fijo del frame
    frame.grid_propagate(False)

    # Especifica el número de filas y columnas
    rows, cols = 5, 5

    # Crear el grid con margenes especificados
    create_grid(frame, rows, cols)
    
    # Crear botones
    create_buttons(frame, rows, cols)

    root.mainloop()

if __name__ == "__main__":
    main()