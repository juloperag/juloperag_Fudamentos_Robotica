from tkinter import Tk, Label, Button, Frame, Entry, messagebox, Text, Scrollbar, Radiobutton, IntVar, StringVar, PhotoImage
from tkinter.ttk import Combobox
import cv2
from PIL import ImageTk, Image
import serial
import time
import threading

class FrameNavegation(Frame):
    def __init__(self,master = None):
        #Configuracion master
        super().__init__(master, width=700, height=460, bg = "black")
        self.master = master
        
class FrameAStar(Frame):
    #width=700, height=600
    #--------------Constructor----------------
    def __init__(self,master = None, CommunicationSerial = None, txt = None):
        #Configuracion master
        self.color_bg_master = "#EAFFF5"
        super().__init__(master, width=700, height=460, bg = self.color_bg_master)
        self.master = master
        #Referencia comunicacion serial y text
        self.txt = txt
        self.CommunicationSerial = CommunicationSerial
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
        #Creacion de widgets
        self.creat_widgets()

    #----------Funciones para el envio al text------------------
    def writeTextfromAStar(self, megg):
        #Habilitar escritura
        self.txt.config(state="normal")
        #Escribir caracteres
        self.txt.insert("end", megg)
        self.txt.see("end")
        #Desactivar escritura
        self.txt.config(state="disabled")

    #-------Funciones de modificacion del grid map----------
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
        if rows>1 and cols>1 and rows<21 and cols<21:
            self.buttons = [[None for _ in range(cols)] for _ in range(rows)]
            #Creacion y Configuracion del grid map dentro del frame
            for i in range(rows):
                self.frame_grid_map.rowconfigure(i, weight=1)
            for j in range(cols):
                self.frame_grid_map.columnconfigure(j, weight=1)
            #Creacion de los botones asociados a las celdas del grid map
            for i in range(rows):
                for j in range(cols):
                    self.buttons[i][j] = Button(self.frame_grid_map, text=".",bg ="#FAFAFA" , command=lambda i=i, j=j: self.attriute_cell(i, j))
                    self.buttons[i][j].grid(row=i, column=j,sticky="nsew")
            #Levantamos bandera
            self.creategrip_flag = True
            #Cambiamos estado del boton de envio
            self.Button_Send["state"] = "active"
        else:
            #Escribir mensaje en la bandeja de texto
            self.writeTextfromAStar("Uno o ambos valores no validos. \n")

    #-------Funciones de envio datos del A Star----------
    def sendGridMapCommunicationSerial(self): 
        #Guardar valor de variables
        row = self.value_row.get()
        cols = self.value_cols.get()
        sepa = self.value_separation.get()
        if sepa>0 and self.flagStart == True and self.flagGoal == True:
            #Construir string a enviar con la informacion del grid map
            msg = "$"+str(row)+":"+str(cols)+":"+str(sepa)+":"
            for i in range(row):
                for j in range(cols):
                    msg = msg + (self.buttons[i][j])["text"]
                msg = msg + ";"
            msg = msg + "$"
            #Enviar string
            self.CommunicationSerial.write(msg.encode('ascii'))
            #Levantamos bandera
            self.flag_A_Star = True

        else:
            #Escribir mensaje en la bandeja de texto
            self.writeTextfromAStar("Uno o varios Parametros del Grid Map no especificados. \n")

    #----Funciones de modificacion del grid map despues de ejecutar A star----------
    def recieveAStarGridMap(self, msg):
        #Se toma los elementos del grid map
        mesg = msg[1:(len(msg)-1)]
        #Se define variables
        row = self.value_row.get()
        cols = self.value_cols.get()
        charmesg  = "."
        #Se recorre las celdas y el mensaje recibido 
        for i in range(row):
            for j in range(cols):
                #Se bloque el boton
                self.buttons[i][j].config(state= "disabled")
                #Se obtiene el caracter del mensaje
                charmesg = mesg[(i*(cols+1)+j)]
                #Se cambia el color de la celda deacuerdo al caracter
                if(charmesg=="+"):
                    self.buttons[i][j].config(bg= "#0080FF")
                elif(charmesg=="S"):
                    self.buttons[i][j].config(bg= "#01DF3A")    
                elif(charmesg=="G"):
                    self.buttons[i][j].config(bg= "#FFBF00")
        #Modificacion de los estados de los botones
        self.Button_Send.config(state="disabled")
        self.Button_Change.config(state="active") 
              
    def changeGridMap(self):
        #Se define variables
        row = self.value_row.get()
        cols = self.value_cols.get()
        #Se modifica las celdas del grid map
        for i in range(row):
            for j in range(cols):
                #Se cambia el color de la celda y se desbloquea el boton
                self.buttons[i][j].config(bg ="#FAFAFA")
                self.buttons[i][j].config(state = "active")
        #Modificacion de los estados de los botones
        self.Button_Send.config(state = "active")
        self.Button_Change.config(state = "disabled") 

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


class MainFrame(Frame):
    def __init__(self,master = None):
        #Configuracion master
        self.color_bg_master = "#EAFFF5"
        super().__init__(master, bg = self.color_bg_master)
        self.master = master
        self.master.protocol('WM_DELETE_WINDOW',self.askQuit)
        self.pack()
        #Definicion de elementos para la Comunicacion Serial
        self.hilo1 = threading.Thread(target=self.receiveCommunicationSerial,daemon=True)
        self.CommunicationSerial = serial.Serial("COM5",19200,timeout=1.0)
        time.sleep(1)
        #Definicion Variables
        self.flag_A_Star = False
        self.flag_Navegation = False
        #Definicion Variables asociadas a widgets
        self.value_send = StringVar()
        #Creacion de widgets
        self.creat_widgets()
        #Inicializacion hilo 1
        self.isRun=True
        self.hilo1.start()
        #Contruccion de las frame de los modos
        self.Navegation = FrameNavegation(master = self.frame_principal)
        self.Navegation.pack_forget()
        self.A_Star = FrameAStar(master = self.frame_principal, CommunicationSerial = self.CommunicationSerial, txt = self.txt)
        self.A_Star.pack_forget()
    
    #------Funcion para cierre de la ventana---------   
    def askQuit(self):
        self.isRun=False
        time.sleep(1.1)
        self.CommunicationSerial.close()
        self.hilo1.join(0.1)
        self.master.quit()
        self.master.destroy()
        print("*** finalizando...")

    #-------Funciones de envio y recepcion----------
    #Recepcion
    def receiveCommunicationSerial(self):
        while self.isRun:
            mesg = self.CommunicationSerial.readline().decode('latin-1')
            if mesg:
                #Escribir mensaje en la bandeja de texto
                self.writeText(mesg +'\n')

            #Si tiene un modo activo se realiza algo con el mensaje
            if self.flag_A_Star:
                if mesg[0] == '$':
                    self.recieveAStarGridMap(mesg)
            if self.flag_Navegation:
                a = 1
    def writeText(self, megg):
        #Habilitar escritura
        self.txt.config(state="normal")
        #Escribir caracteres
        self.txt.insert("end", megg)
        self.txt.see("end")
        #Desactivar escritura
        self.txt.config(state="disabled")
    #Envio
    def sendfromEntryofCS(self):
        msg = self.value_send.get()   
        self.sendCommunicationSerial(msg)
    def sendCommunicationSerial(self, msg):
        self.CommunicationSerial.write(msg.encode('ascii'))

    #--------Funciones para seleccionar el frame del modo respectivo--------------
    def selection_Navegation(self):
        #Cambio de banderas
        self.flag_A_Star = False
        self.flag_Navegation = True
        #Cambio de vizualizacion de frame
        self.A_Star.pack_forget() 
        self.Navegation.pack()
    
    def selection_A_Star(self):
        #Cambio de banderas
        self.flag_A_Star = True
        self.flag_Navegation = False
        #Cambio de vizualizacion de frame
        self.Navegation.pack_forget() 
        self.A_Star.pack()

    #-----------Crear Widgets--------------   
    def creat_widgets(self):
        #----------------------Creacion de paneles------------------------        
        self.frame_superior = Frame(self,bd=3, relief="ridge", bg="#1f2329", width=820, height=65)
        self.frame_superior.grid(row=0,column=0, columnspan=2, sticky= "nsew")      
        self.frame_lateral = Frame(self, bd=3, relief="raised", bg="#2a3138", width=120, height=600)
        self.frame_lateral.grid(row=1,column=0, rowspan=2, sticky= "nsew")        
        self.frame_principal = Frame(self, bg="#f1faff", width=700, height=460)
        self.frame_principal.grid(row=1,column=1)  
        self.frame_inferior = Frame(self, bg="#f1faff", width=700, height=140)
        self.frame_inferior.grid(row=2,column=1) 
        #--------------Creacion de Widgets en los diferentes paneles------------------
        #-----------Frame superior----------
        #Marca de inicio
        imagen_rgb = cv2.resize(cv2.cvtColor(cv2.imread("Imagenes/RobotMovil_2.png"), cv2.COLOR_BGR2RGB), (50, 50)) 
        self.logo = ImageTk.PhotoImage(Image.fromarray(imagen_rgb))
        Label(self.frame_superior, image = self.logo, bg = "#1f2329").place(x=2, y=2)
        Label(self.frame_superior, font= 100 , fg = "white" ,text="Fundamentos de Robótica", bg = "#1f2329").place(x = 60, y = 15)

        #-----------Frame Lateral----------
        #Cambio entre modos de la interfaz grafica
        Label(self.frame_lateral, font= 18 , fg = "white" ,text="Modos Interfaz", bg = "#2a3138").pack(pady = 10)
        Button(self.frame_lateral,text= "A-Star",command=self.selection_A_Star).pack(pady = 13)
        Button(self.frame_lateral,text= "Navegación",command=self.selection_Navegation).pack(pady = 26)

        #---------Frame inferior------------
        #Visualizacion de comentarios
        self.frame_comments = Frame(self.frame_inferior)
        self.frame_comments.place(x=20,y=5)
        scroll = Scrollbar(self.frame_comments)
        self.txt = Text(self.frame_comments,wrap='word',width="80",height="5",yscrollcommand= scroll.set, state="disabled")
        scroll.config(command=self.txt.yview)
        self.txt.pack(side="left")
        scroll.pack(side="right", fill = "y")
        #Envio de comandos
        Entry(self.frame_inferior, textvariable = self.value_send, width=90).place(x=20, y=100)
        Button(self.frame_inferior, text= "Send", command=self.sendfromEntryofCS).place(x=580,y=100)

#---------Inicializacion ventana---------
def main():
    root = Tk()
    root.title("Terminal Robótica")
    app = MainFrame(root)
    app.mainloop()

if __name__ == "__main__": 
    main()
