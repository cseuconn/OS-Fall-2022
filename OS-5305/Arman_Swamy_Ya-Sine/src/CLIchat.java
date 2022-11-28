// Main file to start both server and client
// To start server
// java CLIchat server [port] [ipaddr] port and ip are optional
// To start client
// java CLIchat client [port] [ipaddr] port and ip are optional

import java.io.IOException;

public class CLIchat {

    public static void main(String[] args) throws IOException {
        if(args.length<1){
            System.out.println("Too few arguments");
            System.exit(0);
        }
        int serverPort=1234;//default serverport
        if(args[0].equals("server")){
            if(args.length>1){
                serverPort = Integer.parseInt(args[1]);
            }
            Server server = new Server(serverPort); 
            server.start();
        }
        int clientPort = 1234; //default for port to connect
        String clientServerAddress = "localhost";//default server for client to connect
        if(args[0].equals("client")){
            if(args.length>1){
                clientPort=Integer.parseInt(args[1]);
            }
            if(args.length>2){
                clientServerAddress=args[2];
            }
            Client client = new Client();
            client.createClient(clientPort,clientServerAddress);
        }
    }
}
