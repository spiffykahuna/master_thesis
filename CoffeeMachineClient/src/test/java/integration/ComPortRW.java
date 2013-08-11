package integration;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;

import java.io.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class ComPortRW implements Runnable {
    private static final int MAX_DELAY = 3500;
    public static final String DEFAULT_COM_PORT = "COM25";
    Reader portReader;
    Writer portWriter;
    CommPort commPort = null;

    StringBuilder commandBuffer;
    Random rand = new Random();

    public ComPortRW(String comPortName){

        CommPortIdentifier portIdentifier = null;


        try {
            portIdentifier = CommPortIdentifier.getPortIdentifier(comPortName);
            if (portIdentifier.isCurrentlyOwned() ) {
                throw new IllegalStateException("Error: Port is currently in use");
            }
            commPort = portIdentifier.open(this.getClass().getName(),2000);
            if(!( commPort instanceof SerialPort)) throw new IllegalStateException("Error: Only serial ports are handled by this example.");

            SerialPort serialPort = (SerialPort) commPort;
            serialPort.enableReceiveTimeout(100000000);
            serialPort.enableReceiveThreshold(1);

            serialPort.setSerialPortParams(115200,SerialPort.DATABITS_8,SerialPort.STOPBITS_1,SerialPort.PARITY_NONE);

            InputStream in = serialPort.getInputStream();
            OutputStream out = serialPort.getOutputStream();

            portReader = new InputStreamReader(in, "UTF-8");
            portWriter = new OutputStreamWriter(out, "UTF-8");

            commandBuffer = new StringBuilder();

        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    @Override
    public void run() {
        int ch = -1;

        while(true) {
            if(Thread.currentThread().isInterrupted()) return;
            try {
                while ( ( ch = portReader.read()) > -1 ){
                    if(Thread.currentThread().isInterrupted()) return;

                    processChar((char) ch);
                }
            }
            catch ( IOException e ){

            }
        }
    }

    private void processChar(char ch) throws IOException {

        commandBuffer.append(ch);

        int idx;
        while( (idx = commandBuffer.indexOf("\r\n")) > -1) {
            newCommand(commandBuffer.substring(0, idx ));
            commandBuffer.delete(0, idx + 2);
        }
    }

    private void newCommand(String cmdText) throws IOException {
        System.out.println("<-- " + cmdText);
    }

    public void writeCommand(String cmdText) throws IOException {
//        randomDelay();
        System.out.print("--> " + cmdText);
        portWriter.write(cmdText);
        portWriter.flush();
    }

    private void randomDelay() {
        try {
            Thread.sleep(rand.nextInt(MAX_DELAY));
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static void main ( String[] args ) {
        new Thread(new ComPortRW(args[0]), ComPortRW.class.getSimpleName() ).start();
    }

    public void close() throws IOException {
        portReader.close();
        portWriter.close();
        commPort.close();

    }
}
