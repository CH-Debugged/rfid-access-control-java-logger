import com.fazecast.jSerialComm.SerialPort;
import java.util.Scanner;

public class RFIDLogger {
    public static void main(String[] args) {
        SerialPort port = SerialPort.getCommPorts()[0]; // Automatically selects the first COM port
        port.setBaudRate(9600);

        if (port.openPort()) {
            System.out.println("Connected to Arduino...");
        } else {
            System.out.println("Failed to open port.");
            return;
        }

        Scanner scanner = new Scanner(port.getInputStream());
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            System.out.println("Arduino: " + line);
        }

        // Optional: close port when finished (not needed in this loop)
        // port.closePort();
    }
}
