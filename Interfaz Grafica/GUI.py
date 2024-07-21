from tkinter import Tk, Label, Button, Frame, Entry, messagebox, Text, Scrollbar, Radiobutton, IntVar, StringVar
from tkinter.ttk import Combobox
import serial
import time
import threading

class MainFrame(Frame):
    #--------------Constructor----------------
    def __init__(self,master = None):
        #Configuracion master
        self.color_bg_master = "#EAFFF5"
        super().__init__(master, width=700, height=600, bg = self.color_bg_master)
        self.master = master
        self.master.protocol('WM_DELETE_WINDOW',self.askQuit)
        self.pack()
        #Definicion de elementos para la Comunicacion Serial
        self.hilo1 = threading.Thread(target=self.receiveCommunicationSerial,daemon=True)
        self.CommunicationSerial = serial.Serial("COM5",19200,timeout=1.0)
        time.sleep(1)
        #Definicion Variables
        self.creategrip_flag = False
        self.flagStart = False
        self.cellStart = [0,0]
        self.flagGoal = False
        self.cellGoal = [0,0]
        #Definicion Variables asociadas a widgets
        self.value_row = IntVar()
        self.value_cols = IntVar()
        self.value_separation = IntVar()
        self.value_send = StringVar()
        #Creacion de widgets
        self.creat_widgets()
        #Inicializacion hilo 1
        self.isRun=True
        self.hilo1.start()

    #------Funcion para cierre de la ventana---------   
    def askQuit(self):
        self.isRun=False
        time.sleep(1.1)
        self.CommunicationSerial.close()
        self.hilo1.join(0.1)
        self.master.quit()
        self.master.destroy()
        print("*** finalizando...")

    #-------Funciones de Eventos-----------
    def attriute_cell(self, i, j):
        #Verificamos el modo seleccionado
        if self.cmbox.current() == 0:
            #Verificamos si la celda fue seleccionada previamente como Start o Goal
            if self.cellStart[0] == i and self.cellStart[1] == j:
                self.flagStart = False
            elif self.cellGoal[0] == i and self.cellGoal[1] == j:
                self.flagStart = False
            #Especificamos el atributo de la celda
            self.buttons[i][j].config(text=".")
        elif self.cmbox.current() == 1:
            #Verificamos si la celda fue seleccionada previamente como Start o Goal
            if self.cellStart[0] == i and self.cellStart[1] == j:
                self.flagStart = False
            elif self.cellGoal[0] == i and self.cellGoal[1] == j:
                self.flagStart = False
            #Especificamos el atributo de la celda
            self.buttons[i][j].config(text="#")
        elif self.cmbox.current() == 2:
            #Reiniciamos la anterior posicion del Start
            if self.flagStart:
                self.buttons[self.cellStart[0]][self.cellStart[1]].config(text=".")
            #Especificamos el atributo de la celda
            self.buttons[i][j].config(text="S")
            self.cellStart[0] = i
            self.cellStart[1] = j
            #Levantamos bandera
            self.flagStart = True
        else:
            #Reiniciamos la anterior posicion del Goal
            if self.flagGoal:
                self.buttons[self.cellGoal[0]][self.cellGoal[1]].config(text=".")
            #Especificamos el atributo de la celda
            self.buttons[i][j].config(text="G")
            self.cellGoal[0] = i
            self.cellGoal[1] = j
            #Levantamos bandera
            self.flagGoal = True

    def createGridMap(self):
        #--------Eliminacion de los botones presentes en el grid map---------
        if self.creategrip_flag:
            #Destruccion de los widget presentes el frame
            for widget in self.frame_grid_map.winfo_children():
                widget.destroy()
            #Eliminacion del array
            del self.buttons
            #Reinicia la configuración del grid map
            for i in range(20):  # Asumiendo un máximo de 20 filas y columnas
                self.frame_grid_map.rowconfigure(i, weight=0)
            for j in range(20):
                self.frame_grid_map.columnconfigure(j, weight=0)

        #----------Creacion de nuevos botones en el grid map--------------

        rows = self.value_row.get()
        cols = self.value_cols.get()
        #verificacion de valores maximos permitidos
        if rows<21 and cols<21:
            self.buttons = [[None for _ in range(cols)] for _ in range(rows)]
            #Creacion y Configuracion del grid map dentro del frame
            for i in range(rows):
                self.frame_grid_map.rowconfigure(i, weight=1)
            for j in range(cols):
                self.frame_grid_map.columnconfigure(j, weight=1)
            #Creacion de los botones asociados a las celdas del grid map
            for i in range(rows):
                for j in range(cols):
                    self.buttons[i][j] = Button(self.frame_grid_map, text=".", command=lambda i=i, j=j: self.attriute_cell(i, j))
                    self.buttons[i][j].grid(row=i, column=j,sticky="nsew")
            #Levantamos bandera
            self.creategrip_flag = True
            #Cambiamos estado del boton de envio
            self.Button_Send["state"] = "active"
        else:
            print("Numero maximo de filas o columnas superado en la definicion")

    def sendGridMapCommunicationSerial(self):
        #Guardar valor de variables
        row = self.value_row.get()
        cols = self.value_cols.get()
        #Construir string a enviar con la informacion del grid map
        msg = str(row)+";"+str(cols)+";"+str(self.value_separation.get())+";"
        for i in range(row):
            for j in range(cols):
                msg = msg + (self.buttons[i][j])["text"]
            msg = msg + ":"
        msg = msg + " @"
        #Enviar string
        self.CommunicationSerial.write(msg.encode('ascii'))

    def sendCommunicationSerial(self):
        msg = self.value_send.get()
        self.CommunicationSerial.write(msg.encode('ascii'))

    def receiveCommunicationSerial(self):
        while self.isRun:
            mesg = self.CommunicationSerial.readline().decode('ascii')
            if mesg:
                #Habilitar escritura
                self.txt.config(state="normal")
                #Escribir caracteres
                self.txt.insert("end", mesg)
                self.txt.see("end")
                #Desactivar escritura
                self.txt.config(state="disabled")

    def changeGridMap(self):
        b = 0

    #-----------Crear Widgets--------------
    def creat_widgets(self):
        #Creacion y configuracion del frame para contener el grid map
        self.frame_grid_map = Frame(self,width=450,height=400)
        self.frame_grid_map.place(x=20,y=40)
        self.frame_grid_map.grid_propagate(False)
        #Creacion widgets para la formacion del grid map
        Label(self, text="Create Grid Map", font= 18 , bg = self.color_bg_master).place(x=530,y=40)
        Label(self, text="Row: ", bg = self.color_bg_master).place(x=490,y=80)
        Label(self, text="Column: ", bg = self.color_bg_master).place(x=490,y=110)
        Entry(self, textvariable = self.value_row).place(x=550, y=80)
        Entry(self, textvariable = self.value_cols).place(x=550, y=110)
        Button(self, text= "Create", command= self.createGridMap).place(x=560,y=150)
        #Definicion de caracteristicas del grid map
        Label(self, text="Feature Grid Map", font= 18 , bg = self.color_bg_master).place(x=530,y=190)
        Label(self, text="Separation \n between cells:",bg = self.color_bg_master).place(x=490,y=230)
        Label(self, text="Cell attribute \n selection:",bg = self.color_bg_master).place(x=490,y=280)
        Entry(self, textvariable = self.value_separation, width=6).place(x=590, y=240)
        Label(self, text="Cm",bg = self.color_bg_master).place(x=640,y=240)
        self.cmbox = Combobox(self,values=["Free","Obstacle", "Start","Goal"], width=10, state ="readonly")
        self.cmbox.place(x=590,y=290)
        self.cmbox.current(0) 
        #Envio o cambio del grid map
        self.Button_Send = Button(self, text= "Send Grid Map", command= self.sendGridMapCommunicationSerial, state= "disabled")
        self.Button_Send.place(x=580,y=400)
        self.Button_Change = Button(self, text= "Change", command = self.changeGridMap, state= "disabled")
        self.Button_Change.place(x=500,y=400)
        #Visualizacion de comentarios
        self.frame_comments = Frame(self)
        self.frame_comments.place(x=20,y = 460)
        scroll = Scrollbar(self.frame_comments)
        self.txt = Text(self.frame_comments,wrap='word',width="80",height="5",yscrollcommand= scroll.set, state="disabled")
        scroll.config(command=self.txt.yview)
        self.txt.pack(side="left")
        scroll.pack(side="right", fill = "y")
        #Envio de comandos
        Entry(self, textvariable = self.value_send, width=90).place(x=20, y=560)
        Button(self, text= "Send", command = self.sendCommunicationSerial).place(x=580,y=560)



def main():
    root = Tk()
    root.title("A Star")
    app = MainFrame(root)
    app.mainloop()

if __name__ == "__main__": 
    main()
