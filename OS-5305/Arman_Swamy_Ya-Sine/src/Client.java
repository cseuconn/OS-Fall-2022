//client functions
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Scanner;

public class Client  {	
	private String notif = " *** ";

	private ObjectInputStream sInput;		// to read from the socket
	private ObjectOutputStream sOutput;		// to write on the socket
	private Socket socket;					// socket object
	
	private String server, username,servername;	// server,username,servername
	private int port;// variable for port number

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	Client (){}//empty constructor
	

	Client(String server, int port, String username) {
		this.server = server;
		this.port = port;
		this.username = username;
	}
	

	public boolean start() {
		//connect to the server
		try {
			socket = new Socket(server, port);
		} 
		catch(Exception ec) {
			display("Error connectiong to server:" + ec);
			return false;
		}
		
		String msg = "Connection accepted " + socket.getInetAddress() + ":" + socket.getPort();
		display(msg);

		//creating data streams
		try
		{
			sInput  = new ObjectInputStream(socket.getInputStream());
			sOutput = new ObjectOutputStream(socket.getOutputStream());
		}
		catch (IOException eIO) {
			display("Exception creating new Input/output Streams: " + eIO);
			return false;
		}

		new ListenFromServer().start();
		try
		{
			sOutput.writeObject(username);
		}
		catch (IOException eIO) {
			display("Exception doing login : " + eIO);
			disconnect();
			return false;
		}

		return true;
	}


	private void display(String msg) {
		System.out.println(msg);
	}

	void sendMessage(MessageObject msg) {
		try {
			sOutput.writeObject(msg);
		}
		catch(IOException e) {
			display("Exception writing to server: " + e);
		}
	}


	private void disconnect() {
		try { 
			if(sInput != null) sInput.close();
		}
		catch(Exception e) {}
		try {
			if(sOutput != null) sOutput.close();
		}
		catch(Exception e) {}
        try{
			if(socket != null) socket.close();
		}
		catch(Exception e) {}
			
	}

	public void createClient(int portNumber,String serverAddress) {
		String userName = "Anonymous";
		Scanner scan = new Scanner(System.in);
		
		System.out.println("Enter the username: ");
		userName = scan.nextLine();
		Client client = new Client(serverAddress, portNumber, userName);

		if(!client.start()){
			scan.close();
			return;
		}
			
		
		System.out.println("\nHello "+userName+"! Welcome to the chatroom "+servername);
		System.out.println("Instructions:");
		System.out.println("1. Just type messages to send it to all active clients");
		System.out.println("2. Type '@username<space><message>' to send a personal message to desired client");
		System.out.println("3. Type '/activeusers' to see list of active clients");
		System.out.println("4. Type '/logout' to exit the chat room");
		
		// loop to get the input from the user
		while(true) {
			System.out.print("> ");
			String msg = scan.nextLine();	
			if(msg.equalsIgnoreCase("/logout")) {//to logout from server
				client.sendMessage(new MessageObject(MessageObject.LOGOUT, ""));
				break;
			}
			else if(msg.equalsIgnoreCase("/activeusers")) {//to list all users
				client.sendMessage(new MessageObject(MessageObject.ACTIVEUSERS, ""));				
			}
			else {// message string
				client.sendMessage(new MessageObject(MessageObject.MESSAGE, msg));
			}
		}
		scan.close();
		client.disconnect();	
	}

	class ListenFromServer extends Thread {

		public void run() {
			while(true) {
				try {
					String msg = (String) sInput.readObject();
					System.out.println(msg);
					System.out.print("> ");
				}
				catch(IOException e) {
					display(notif + "Server has closed the connection: " + e + notif);
					break;
				}
				catch(ClassNotFoundException e2) {
				}
			}
		}
	}
}

