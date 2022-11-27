import java.io.*;
import java.net.*;
import java.text.SimpleDateFormat;
import java.util.*;

public class Server{
	private static int uniqueId;
	private ArrayList<ClientThread> al;
	private SimpleDateFormat sdf;
	private int port;
	private boolean serverActiveStatus;
	private String notif = " *** ";
	private String serverName = "Temp server";
	History history = new History(serverName);
	public Server(int port) {
		this.port = port;
		sdf = new SimpleDateFormat("HH:mm:ss");
		al = new ArrayList<ClientThread>();
	}
	public String getServerName(){
		return serverName;
	}
	public void start() throws IOException{
		serverActiveStatus = true;
		try 
		{
			Scanner sc = new Scanner(System.in);
			System.out.println("Give a name to the chat server:");
			serverName = sc.nextLine();
			sc.close();
			ServerSocket serverSocket = new ServerSocket(port);
			String ipAddr = InetAddress.getLocalHost().getHostAddress();
			display("Server waiting for Clients on port " +ipAddr+":"+port + ".");
			history.setFileName(serverName);
			history.write_to("Hello this is a test line");

			while(serverActiveStatus) 
			{
				Socket socket = serverSocket.accept();
				// break if server stoped
				if(!serverActiveStatus)
					break;
				ClientThread t = new ClientThread(socket,history);
				al.add(t);
				
				t.start();
			}
			// try to stop the server
			try {
				serverSocket.close();
				for(int i = 0; i < al.size(); ++i) {
					ClientThread tc = al.get(i);
					try {
					// close all data streams and socket
					tc.sInput.close();
					tc.sOutput.close();
					tc.socket.close();
					}
					catch(IOException ioE) {
					}
				}
			}
			catch(Exception e) {
				display("Exception closing the server and clients: " + e);
			}
		}
		catch (IOException e) {
            String msg = sdf.format(new Date()) + " Exception on new ServerSocket: " + e + "\n";
			display(msg);
		}
	}
	
	// to stop the server
	// protected void stop() {
	// 	serverActiveStatus = false;
	// 	try {
	// 		new Socket("localhost", port);
	// 	}
	// 	catch(Exception e) {
	// 	}
	// }
	
	// Display an event to the console
	private void display(String msg) {
		String time = sdf.format(new Date()) + " " + msg;
		System.out.println(time);
	}
	
	// to broadcast a message to all Clients
	private synchronized boolean broadcast(String message) {
		// add timestamp to the message
		String time = sdf.format(new Date());
		
		// to check if message is private i.e. client to client message
		String[] w = message.split(" ",3);
		
		boolean isPrivate = false;
		if(w[1].charAt(0)=='@') 
			isPrivate=true;
		
		
		// if private message, send message to mentioned username only
		if(isPrivate==true)
		{
			String tocheck=w[1].substring(1, w[1].length());
			history.write_to(time +" "+message);

			
			message=w[0]+w[2];
			String messageLf = time + " " + message + "\n";
			boolean found=false;
			// we loop in reverse order to find the mentioned username
			for(int y=al.size(); --y>=0;)
			{
				ClientThread ct1=al.get(y);
				String check=ct1.getUsername();
				if(check.equals(tocheck))
				{
					// try to write to the Client if it fails remove it from the list
					if(!ct1.writeMsg(messageLf)) {
						al.remove(y);
						display("Disconnected Client " + ct1.username + " removed from list.");
					}
					// username found and delivered the message
					found=true;
					break;
				}
				
				
				
			}
			// mentioned user not found, return false
			if(found!=true)
			{
				return false; 
			}
		}
		// if message is a broadcast message
		else
		{
			String messageLf = time + " " + message + "\n";
			// display message
			System.out.print(messageLf);
			history.write_to(messageLf);
			
			// we loop in reverse order in case we would have to remove a Client
			// because it has disconnected
			for(int i = al.size(); --i >= 0;) {
				ClientThread ct = al.get(i);
				// try to write to the Client if it fails remove it from the list
				if(!ct.writeMsg(messageLf)) {
					al.remove(i);
					display("Disconnected Client " + ct.username + " removed from list.");
				}
			}
		}
		return true;
		
		
	}

	// if client sent LOGOUT message to exit
	synchronized void remove(int id) {
		
		String disconnectedClient = "";
		// scan the array list until we found the Id
		for(int i = 0; i < al.size(); ++i) {
			ClientThread ct = al.get(i);
			// if found remove it
			if(ct.id == id) {
				disconnectedClient = ct.getUsername();
				al.remove(i);
				break;
			}
		}
		broadcast(notif + disconnectedClient + " has left the chat room." + notif);
	}
	

	// One instance of this thread will run for each client
	class ClientThread extends Thread {
		// the socket to get messages from client
		Socket socket;
		History history;
		ObjectInputStream sInput;
		ObjectOutputStream sOutput;
		// my unique id (easier for deconnection)
		int id;
		// the Username of the Client
		String username;
		// message object to recieve message and its type
		ChatMessage cm;
		// timestamp
		String date;

		// Constructor
		ClientThread(Socket socket,History history) {
			// a unique id
			id = ++uniqueId;
			this.socket = socket;
			this.history = history;
			//Creating both Data Stream
			// System.out.println("Thread trying to create Object Input/Output Streams");
			try
			{
				sOutput = new ObjectOutputStream(socket.getOutputStream());
				sInput  = new ObjectInputStream(socket.getInputStream());
				// read the username
				
				
				
				
				
			}
			catch (IOException e) {
				display("Exception creating new Input/output Streams: " + e);
				return;
			}
			// catch (ClassNotFoundException e) {
			// }
			// try {
			// 	sOutput.writeObject(serverName);
			// } catch (Exception e) {
			// 	display("Exception sending servername");
			// }
			try {
				username = (String) sInput.readObject();
				broadcast(notif + username + " has joined the chat room." + notif);
			} catch (Exception e) {
				display("exception reading username");
			}
            date = new Date().toString() + "\n";
		}
		
		public String getUsername() {
			return username;
		}

		public void setUsername(String username) {
			this.username = username;
		}

		// infinite loop to read and forward message
		public void run() {
			// to loop until LOGOUT
			boolean serverActiveStatus = true;
			while(serverActiveStatus) {
				// read a String (which is an object)
				try {
					cm = (ChatMessage) sInput.readObject();
				}
				catch (IOException e) {
					display(username + " Exception reading Streams: " + e);
					break;				
				}
				catch(ClassNotFoundException e2) {
					break;
				}
				// get the message from the ChatMessage object received
				String message = cm.getMessage();

				// different actions based on type message
				switch(cm.getType()) {

				case ChatMessage.MESSAGE:
					boolean confirmation =  broadcast(username + ": " + message);
					if(confirmation==false){
						String msg = notif + "Sorry. No such user exists." + notif;
						writeMsg(msg);
					}
					break;
				case ChatMessage.LOGOUT:
					display(username + " disconnected with a LOGOUT message.");
					serverActiveStatus = false;
					break;
				case ChatMessage.WHOISIN:
					writeMsg("List of the users connected at " + sdf.format(new Date()) + "\n");
					// send list of active clients
					for(int i = 0; i < al.size(); ++i) {
						ClientThread ct = al.get(i);
						writeMsg((i+1) + ") " + ct.username + " since " + ct.date);
					}
					break;
				}
			}
			// if out of the loop then disconnected and remove from client list
			remove(id);
			close();
		}
		
		// close everything
		private void close() {
			try {
				if(sOutput != null) sOutput.close();
			}
			catch(Exception e) {}
			try {
				if(sInput != null) sInput.close();
			}
			catch(Exception e) {};
			try {
				if(socket != null) socket.close();
			}
			catch (Exception e) {}
		}

		// write a String to the Client output stream
		private boolean writeMsg(String msg) {
			// if Client is still connected send the message to it
			if(!socket.isConnected()) {
				close();
				return false;
			}
			// write the message to the stream
			try {
				sOutput.writeObject(msg);
			}
			// if an error occurs, do not abort just inform the user
			catch(IOException e) {
				display(notif + "Error sending message to " + username + notif);
				display(e.toString());
			}
			return true;
		}
	}
}

