"""Server for multithreaded chat application"""
from socket import AF_INET, socket, SOCK_STREAM
from threading import Thread


def accept_connections():
    """Accepts connection from clients"""
    while True:
        user, user_adr = SERVER.accept()
        print("%s:%s has connected." % user_adr)
        user.send(bytes("Type your username and press enter to begin", "utf8"))
        addresses[user] = user_adr
        Thread(target=handle_user, args=(user,)).start()


def handle_user(user):  # Takes user socket as argument.
    """Handles user connection"""

    username = user.recv(BUFFER_SIZE).decode("utf8")
    welcome = 'Welcome %s! If you ever want to quit, type {quit} to exit.' % username
    user.send(bytes(welcome, "utf8"))
    message = "%s has joined the chat!" % username
    broadcast(bytes(message, "utf8"))
    users[user] = username

    while True:
        message = user.recv(BUFFER_SIZE)
        if message != bytes("{quit}", "utf8"):
            broadcast(message, username+": ")
        else:
            user.send(bytes("{quit}", "utf8"))
            user.close()
            print("%s:%s has disconnected." % addresses[user])
            del users[user]
            broadcast(bytes("%s has left the chat." % username, "utf8"))
            break


def broadcast(message, prefix=""):  # prefix is for username identification.
    """Broadcasts a message to all users"""

    for sock in users:
        sock.send(bytes(prefix, "utf8")+message)


users = {}
addresses = {}

HOST = ''
PORT = 33000
BUFFER_SIZE = 1024
ADDR = (HOST, PORT)

SERVER = socket(AF_INET, SOCK_STREAM)
SERVER.bind(ADDR)

if __name__ == "__main__":
    SERVER.listen(5)
    print("Waiting for connection...")
    ACCEPT_THREAD = Thread(target=accept_connections)
    ACCEPT_THREAD.start()
    ACCEPT_THREAD.join()
    SERVER.close()
