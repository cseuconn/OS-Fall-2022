import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.Scanner; 

public class History {
    private String name = "";

    public History(){};

    public History(String fName){
        this.name = fName + ".txt";
        make_history();
    }

    public String getFileName(){
        return this.name;
    }
    
    public void setFileName(String f){
        String path = this.name;
        File fp = new File(path);
        if(fp.exists() && !fp.isDirectory()) { 
            path = f + ".txt";
            File rename = new File(path);
            boolean flag = fp.renameTo(rename);
            if (flag == true) {
                System.out.println("File Successfully Rename");
                this.write_to("file is renamed\n");
            }
            this.name = path;
        }else{
            System.out.println("No History File Exists!");
        }
    }

    public void write_to(String ln){
        try {
            FileWriter wObj = new FileWriter(this.name,true);
            wObj.write(ln+"\n");
            wObj.close();
            // System.out.println("Successfully wrote to the file.");
          } catch (IOException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
          }
    }

    public String read_from(int nums){
        String msg = "";
        System.out.println(nums);
        try {
            File fObj = new File(this.name);
            Scanner rObj = new Scanner(fObj);
            while (rObj.hasNextLine()) {
              String data = rObj.nextLine();
              msg += data +"\n";
            }
            rObj.close();
        } catch (FileNotFoundException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }
        return msg;
    }

    private void make_history(){
        try {
            File fObj = new File(this.name);
            if (fObj.createNewFile()) {
              // System.out.println("File created: " + fObj.getName());
              System.out.println("History file is created");
            } else {
              System.out.println("File already exists.");
            }
          } catch (IOException e) {
            System.out.println("An error occurred.");
            e.printStackTrace();
          }
    }
}