package ee.ttu.deko.coffee.service.message;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.util.Iterator;

public class MessageReader implements Runnable {
    private final static Logger logger = LoggerFactory.getLogger(MessageReader.class);

    protected final Reader inputReader;
    protected final MessageHandler messageHandler;

    private volatile Thread readThread;
    private boolean isClosed;

    private StringBuilder messageBuffer = new StringBuilder();
    private boolean isStarted;

    private enum ReaderState{
        WAITING_FOR_INPUT,
        READING_MESSAGE_SIZE,
        READING_MESSAGE_VALUE, READING_MESSAGE_TERMINATOR, HANDLING_NEW_MESSAGE,
    }
    private ReaderState state;

    public MessageReader(Reader inputReader, MessageHandler messageHandler) {
        if(inputReader == null) throw new IllegalArgumentException("Unable to create reader. Stream is null");
        if(messageHandler == null) throw new IllegalArgumentException("Unable to create reader. Message handler is null");

        this.inputReader = inputReader;
        this.messageHandler = messageHandler;
        this.state = ReaderState.WAITING_FOR_INPUT;
    }

    public synchronized void start() {
        if(!isStarted) {
            isStarted = true;
            readThread = new Thread(this, getClass().getSimpleName());
            readThread.start();
        }
    }

    @Override
    public void run() {
        try {
            doRun();
        } catch (Exception e) {
            logger.error("Read loop terminated", e);
        }
        logger.info("Reader loop ended successfully");
    }

    private void doRun() throws IOException, InterruptedException  {
        logger.info("Starting read loop");
        char newChar;
        int messageLength = -1;
        for (; !isClosed ;) {

            synchronized (this) {
                Thread thisThread = Thread.currentThread();
                if(thisThread.isInterrupted())  break;
                if(readThread != thisThread)    break;
                if(isClosed)                    break;
            }


            // State machine for receiving netstrings
            switch(state) {
                case WAITING_FOR_INPUT:
                    newChar = readChar();
                    if(Character.isDigit(newChar)) {
                        messageBuffer.append(newChar);
                        state = ReaderState.READING_MESSAGE_SIZE;
                    }
                    break;

                case READING_MESSAGE_SIZE:
                    newChar = readChar();
                    if(Character.isDigit(newChar)) {
                        messageBuffer.append(newChar);
                        state = ReaderState.READING_MESSAGE_SIZE;
                    } else if(Character.valueOf(newChar).equals(':')) {
                        messageLength = -1;
                        try{
                            messageLength = Integer.parseInt(messageBuffer.toString());
                        } catch(NumberFormatException nfe) {
                            logger.warn("Unable to parse message size: value = {}", messageBuffer.toString());
                        }
                        if(messageLength > -1) {
                            messageBuffer.setLength(0);
                            state = ReaderState.READING_MESSAGE_VALUE;
                        }
                    }
                    break;

                case READING_MESSAGE_VALUE:
                    if(messageLength > 0) {
                        newChar = readChar();
                        messageBuffer.append(newChar);
                        --messageLength;
                    } else if(messageLength == 0) {
                        state = ReaderState.READING_MESSAGE_TERMINATOR;
                    } else {
                        logger.warn("Reader is bound of message. Current position = {}", messageLength );
                        state =  ReaderState.WAITING_FOR_INPUT;
                    }
                    break;

                case READING_MESSAGE_TERMINATOR:
                    newChar = readChar();
                    if(Character.valueOf(newChar).equals(',')) {
                        state = ReaderState.HANDLING_NEW_MESSAGE;
                    } else {
                        logger.warn("Unable to get message terminator. Got {} instead of ',' ", newChar);
                        state = ReaderState.WAITING_FOR_INPUT;
                    }
                    break;

                case HANDLING_NEW_MESSAGE:
                    String msg = messageBuffer.toString().replace("\r", "");
                    if(msg != null && !msg.isEmpty()) {
                        logger.info("Received new message: {}", msg);
                        handleMessage(msg);

                        if(messageBuffer.capacity() > 1024) {
                            messageBuffer = new StringBuilder();
                        } else {
                            messageBuffer.setLength(0);
                        }
                    }
                    state = ReaderState.WAITING_FOR_INPUT;

                    break;

                default:
                    logger.warn("Unreachable reader state: {}", state.name());
                    continue;
            }
        }
        logger.info("Reader loop was stopped.");
    }

    private char readChar() throws IOException {
        int newChar = inputReader.read();
        if(newChar == -1) throw new IOException("End of stream occurred. (User generated exception)");
        return (char) newChar;
    }

    private void handleMessage(String msg) {
        messageHandler.handleMessage(msg);
    }

    public synchronized void stop() {
        logger.info("Stopping reader");
        isClosed = true;

        if(readThread != null) {
            Thread thread = readThread;
            readThread = null;
            thread.interrupt();
        }
    }

    public boolean isAlive() {
        return readThread != null && readThread.isAlive();
    }
}
