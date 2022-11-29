from socket import AF_INET, socket, SOCK_STREAM
from threading import Thread
import tkinter


# Recieve Messages
def receive():
    while True:
        try:
            val = server_connection.recv(BUFSIZ).decode("utf8") # Recieve message from server
            all_msg.insert(tkinter.END, val) # Insert new message into message list all_msg
        except OSError:
            break

# Send Messages
def send(event=None):
    val = msg_value.get() # Get value from entry field
    msg_value.set("") # Reset it to nothing
    server_connection.send(bytes(val, "utf8")) # Send the message to the server
    if val == "{quit}": # If the message is 'quit' close the connection with the server
        server_connection.close()
        top.quit()

#When you quit
def quitting(event=None): # Uses the send function with the message value
    msg_value.set("{quit}") # Sets the msg_value to 'quit'
    send() # Uses send function to bring it to if statement in send()

#GUI


top = tkinter.Tk()
top.title("Chat Server")

messages = tkinter.Frame(top)
msg_value = tkinter.StringVar()  # For the messages to be sent
msg_value.set(" ")
scrollbar = tkinter.Scrollbar(messages)  # To navigate through past messages


all_msg = tkinter.Listbox(messages, height=25, width=50, yscrollcommand=scrollbar.set) # List of all prior messages
scrollbar.pack(side=tkinter.RIGHT, fill=tkinter.Y)
all_msg.pack(side=tkinter.LEFT, fill=tkinter.BOTH)
all_msg.pack()
messages.pack()

message_field = tkinter.Entry(top, textvariable=msg_value)
message_field.bind("<Return>", send)
message_field.pack()


send_button = tkinter.Button(top, text="Send", command=send)
send_button.pack()

quit_button = tkinter.Button(top, text="QUIT", command=quitting)
quit_button.pack()

top.protocol("WM_DELETE_WINDOW", quitting)

#Sockets


HOST = input('Enter host: ')
PORT = input('Enter port: ')

if not HOST:
    HOST = '127.0.0.1'
if not PORT:
    PORT = 33000
else:
    PORT = int(PORT)

BUFSIZ = 1024
ADDR = (HOST, PORT)

server_connection = socket(AF_INET, SOCK_STREAM)
server_connection.connect(ADDR)

begin_recieving = Thread(target=receive)
begin_recieving.start()
tkinter.mainloop()  # Starts GUI execution.
