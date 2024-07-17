from tkinter import Tk, Label, Button, Frame, Entry, messagebox, Text, Scrollbar, Radiobutton, IntVar
from tkinter.ttk import Combobox

class MainFrame(Frame):
    #--------Constructor--------
    def __init__(self,master = None, rows = 10, cols = 10):
        #Configuracion master
        super().__init__(master, width=600, height=350, bg = "#EAFFF5")
        self.master = master
        self.pack()
        #Definicion Variables
        self.var_op = IntVar()
        self.rows = rows
        self.cols = cols
        self.buttons = [[None for _ in range(cols)] for _ in range(rows)]
        #Creacion de widgts
        self.creat_widgets()

    #-------Crear Widgets---------
    def creat_widgets(self):
        #----------------
        self.frame_grid_map = Frame(self,width=400,height=200)
        self.frame_grid_map.place(x=20,y=20)
        # Establece el tamaño fijo del frame
        self.frame_grid_map.grid_propagate(False)
        #Creacion grid
        for row in range(self.rows):
            self.frame_grid_map.rowconfigure(row, weight=1)
        for col in range(self.cols):
            self.frame_grid_map.columnconfigure(col, weight=1)
        #creacion de botones
        for i in range(self.rows):
            for j in range(self.cols):
                self.buttons[i][j] = Button(self.frame_grid_map, text=".", command=lambda i=i, j=j: self.toggle_obstacle(i, j))
                self.buttons[i][j].grid(row=i, column=j,sticky="nsew")
        #--------------------



def main():
    root = Tk()
    root.title("A Star")
    app = MainFrame(root)
    app.mainloop()

if __name__ == "__main__": 
    main()
