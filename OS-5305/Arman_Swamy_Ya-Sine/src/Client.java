import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Scanner;

public class Client  {
	
	// notification
	private String notif = " *** ";

	// for I/O
	private ObjectInputStream sInput;		// to read from the socket
	private ObjectOutputStream sOutput;		// to write on the socket
	private Socket socket;					// socket object
	
	private String server, username,servername;	// server and username
	private int port;					//port

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	/*
	 *  Constructor to set below things
	 *  server: the server address
	 *  port: the port number
	 *  username: the username
	 */
	Client (){
		//empty constructor
	}
	Client(String server, int port, String username) {
		this.server = server;
		this.port = port;
		this.username = username;
	}
	
	/*
	 * To start the chat
	 */
	public boolean start() {
		// try to connect to the server
		try {
			socket = new Socket(server, port);
		} 
		// exception handler if it failed
		catch(Exception ec) {
			display("Error connectiong to server:" + ec);
			return false;
		}
		
		String msg = "Connection accepted " + socket.getInetAddress() + ":" + socket.getPort();
		display(msg);
	
		/* Creating both Data Stream */
		try
		{
			sInput  = new ObjectInputStream(socket.getInputStream());
			sOutput = new ObjectOutputStream(socket.getOutputStream());
		}
		catch (IOException eIO) {
			display("Exception creating new Input/output Streams: " + eIO);
			return false;
		}

		// creates the Thread to listen from the server 
		new ListenFromServer().start();
		// Send our username to the server this is the only message that we
		// will send as a String. All other messages will be ChatMessage objects
		// try {
		// 	servername = (String) sInput.readObject();
		// } catch (Exception eIO) {
		// 	display("Exception doing login : " + eIO);
		// 	disconnect();
		// 	return false;
		// }
		try
		{
			sOutput.writeObject(username);
		}
		catch (IOException eIO) {
			display("Exception doing login : " + eIO);
			disconnect();
			return false;
		}

		// success we inform the caller that it worked
		return true;
	}

	/*
	 * To send a message to the console
	 */
	private void display(String msg) {

		System.out.println(msg);
		
	}
	
	/*
	 * To send a message to the server
	 */
	void sendMessage(ChatMessage msg) {
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
		// try {
		// 	this.servername = (String) sInput.readObject();
		// 	// System.out.println(servername);
		// } catch (Exception eIO) {
		// 	display("Exception doing login : " + eIO);
		// 	disconnect();
		// }
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
		
		// infinite loop to get the input from the user
		while(true) {
			System.out.print("> ");
			// read message from user
			String msg = scan.nextLine();	
			// logout if message is LOGOUT
			if(msg.equalsIgnoreCase("/logout")) {
				client.sendMessage(new ChatMessage(ChatMessage.LOGOUT, ""));
				break;
			}
			// message to check who are present in chatroom
			else if(msg.equalsIgnoreCase("/activeusers")) {
				client.sendMessage(new ChatMessage(ChatMessage.WHOISIN, ""));				
			}
			// regular text message
			else {
				client.sendMessage(new ChatMessage(ChatMessage.MESSAGE, msg));
			}
		}
		// close resource
		scan.close();
		// client completed its job. disconnect client.
		client.disconnect();	
	}

	/*
	 * a class that waits for the message from the server
	 */
	class ListenFromServer extends Thread {

		public void run() {
			while(true) {
				try {
					// read the message form the input datastream
					String msg = (String) sInput.readObject();
					// print the message
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

