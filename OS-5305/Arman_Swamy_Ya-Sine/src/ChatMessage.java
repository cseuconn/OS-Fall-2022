//chat message class to handle structures of message input
import java.io.*;

public class ChatMessage implements Serializable {
	//ACTIVEUSERS - lists all active users
	//MESSAGE - is the format of message
	//LOGOUT- logsout from the server
	static final int ACTIVEUSERS = 0, MESSAGE = 1, LOGOUT = 2;
	private int type;
	private String message;
	
	ChatMessage(int type, String message) {
		this.type = type;
		this.message = message;
	}
	
	int getType() {
		return type;
	}

	String getMessage() {
		return message;
	}
}
