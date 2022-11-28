//server functions
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
		try {
			Scanner sc = new Scanner(System.in);
			System.out.println("Give a name to the chat server:");
			serverName = sc.nextLine();
			sc.close();
			ServerSocket serverSocket = new ServerSocket(port);
			String ipAddr = InetAddress.getLocalHost().getHostAddress();
			display("Server waiting for Clients on port " +ipAddr+":"+port + ".");
			history.setFileName(serverName);

			while(serverActiveStatus) {
				Socket socket = serverSocket.accept();

				if(!serverActiveStatus)
					break;
				ClientThread t = new ClientThread(socket,history);
				al.add(t);
				
				t.start();
			}

			try {
				serverSocket.close();
				for(int i = 0; i < al.size(); ++i) {
					ClientThread tc = al.get(i);
					try {
						tc.sInput.close();
						tc.sOutput.close();
						tc.socket.close();
					}
					catch(IOException ioE) {
						display("Exception closing IO streams");
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
	
	//display helper function
	private void display(String msg) {
		String time = sdf.format(new Date()) + " " + msg;
		System.out.println(time);
	}
	
	// to broadcast a message to all Clients
	private synchronized boolean broadcast(String message) {
		String time = sdf.format(new Date());
		// to check if message is private
		String[] w = message.split(" ",3);
		
		boolean isPrivate = false;
		if(w[1].charAt(0)=='@') 
			isPrivate=true;
		//private message
		if(isPrivate==true){
			String tocheck=w[1].substring(1, w[1].length());
			history.write_to(time +" "+message);

			
			message=w[0]+w[2];
			String messageLf = time + " " + message + "\n";
			boolean found=false;
			
			for(int y=al.size(); --y>=0;){ // looping through all users to find the correct user in msg
				ClientThread ct1=al.get(y);
				String check=ct1.getUsername();
				if(check.equals(tocheck)){
					if(!ct1.writeMsg(messageLf)) {
						al.remove(y);
						display("Disconnected Client " + ct1.username + " removed from list.");
					}
					found=true;
					break;
				}				
			}
			if(found!=true){
				return false; 
			}
		}
		//broadcast message
		else{
			String messageLf = time + " " + message + "\n";
			
			System.out.print(messageLf); // display message
			history.write_to(messageLf); //write it to history
			
			for(int i = al.size(); --i >= 0;) { //sending messages to all clients
				ClientThread ct = al.get(i);
				if(!ct.writeMsg(messageLf)) {
					al.remove(i);
					display("Disconnected Client " + ct.username + " removed from list.");
				}
			}
		}
		return true;
		
		
	}

	synchronized void remove(int id) {
		
		String disconnectedClient = "";
		for(int i = 0; i < al.size(); ++i) {
			ClientThread ct = al.get(i);
			if(ct.id == id) {
				disconnectedClient = ct.getUsername();
				al.remove(i);
				break;
			}
		}
		broadcast(notif + disconnectedClient + " has left the chat room." + notif);
	}
	

	// one instance of thread for every client
	class ClientThread extends Thread {

		Socket socket;
		History history;
		ObjectInputStream sInput;
		ObjectOutputStream sOutput;
		int id;
		String username;
		MessageObject cm;
		String date;
		ClientThread(Socket socket,History history) {

			id = ++uniqueId;
			this.socket = socket;
			this.history = history;
			try{
				sOutput = new ObjectOutputStream(socket.getOutputStream());
				sInput  = new ObjectInputStream(socket.getInputStream());
			}
			catch (IOException e) {
				display("Exception creating new Input/output Streams: " + e);
				return;
			}
			try {
				username = (String) sInput.readObject();//get username from client
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

		//loop to read and forward message
		public void run() {
			boolean serverActiveStatus = true;
			while(serverActiveStatus) {
				try {
					cm = (MessageObject) sInput.readObject();
				}
				catch (IOException e) {
					display(username + " Exception reading Streams: " + e);
					break;				
				}
				catch(ClassNotFoundException e2) {
					break;
				}
				String message = cm.getMessage();
				switch(cm.getType()) {
					case MessageObject.MESSAGE:{
						boolean confirmation =  broadcast(username + ": " + message);
						if(confirmation==false){
							String msg = notif + "Sorry. No such user exists." + notif;
							writeMsg(msg);
						}
					}
					break;
					case MessageObject.LOGOUT:{
						display(username + " disconnected with a LOGOUT message.");
						serverActiveStatus = false;
					}
					break;
					case MessageObject.ACTIVEUSERS:{
						writeMsg("List of the users connected at " + sdf.format(new Date()) + "\n");
						for(int i = 0; i < al.size(); ++i) {
							ClientThread ct = al.get(i);
							writeMsg((i+1) + ") " + ct.username + " since " + ct.date);
						}
					}
					break;
				}
			}
			remove(id);
			close();
		}
		

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

		//write string to output stream
		private boolean writeMsg(String msg) {
			if(!socket.isConnected()) {
				close();
				return false;
			}
			try {
				sOutput.writeObject(msg);
			}
			catch(IOException e) {
				display(notif + "Error sending message to " + username + notif);
				display(e.toString());
			}
			return true;
		}
	}
}

