package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.service.ServiceListener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.Reader;
import java.util.Set;

public class MessageReader implements Runnable {
    private final static Logger logger = LoggerFactory.getLogger(MessageReader.class);

    protected final Reader inputReader;
    protected final MessageHandler messageHandler;

    private volatile Thread readThread;
    private boolean isClosed;

    private StringBuilder messageBuffer = new StringBuilder();

    public MessageReader(Reader inputReader, MessageHandler messageHandler) {
        this.inputReader = inputReader;
        this.messageHandler = messageHandler;
    }

    public void start() {
        readThread = new Thread(this, getClass().getSimpleName());
        readThread.start();
    }

    @Override
    public void run() {
        try {
            doRun();
        } catch (Exception e) {
            logger.error("Read loop terminated", e);
        }
        logger.info("Reader was closed");
    }

    private void doRun() throws IOException, InterruptedException  {
        logger.info("Starting read loop");
        for (; !isClosed ;) {

            synchronized (this) {
                Thread thisThread = Thread.currentThread();
                if(thisThread.isInterrupted())  return;
                if(readThread != thisThread)    return;
                if(isClosed)                    return;
            }

            int newChar = inputReader.read();
            if (newChar < 0) {
                // EOF
                logger.debug("End of stream appeared");
                return;
            }

            messageBuffer.append((char) newChar);
            if(hasNewLine(messageBuffer)) {
                logger.debug("Processing new line...");
                processNewMessage(messageBuffer);
            }
        }
    }

    private boolean hasNewLine(StringBuilder messageBuffer) {
        return  messageBuffer.indexOf("\n") > -1;
    }

    private void processNewMessage(StringBuilder messageBuffer) {
        int idx;
        while ((idx = messageBuffer.indexOf("\n")) > -1) {
            String msg = messageBuffer.substring(0, idx).replace("\r", "");
            handleMessage(msg);
            messageBuffer.delete(0, idx + 1);
        }
    }

    private void handleMessage(String msg) {
        messageHandler.handleMessage(msg);
    }

    public synchronized void close() {
        logger.info("Closing reader");
        isClosed = true;

        if(readThread != null) {
            Thread thread = readThread;
            readThread = null;
            thread.interrupt();
        }
    }
}
