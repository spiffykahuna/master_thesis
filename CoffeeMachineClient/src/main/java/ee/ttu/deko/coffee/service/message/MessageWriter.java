package ee.ttu.deko.coffee.service.message;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Message;
import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.RPCServiceListener;
import ee.ttu.deko.coffee.service.Service;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class MessageWriter implements  Runnable, RPCServiceListener{
    private final static Logger logger = LoggerFactory.getLogger(MessageWriter.class);

    private final Service service;
    private final Writer outputWriter;

    private boolean isStarted;
    private boolean isClosed;
    private volatile Thread writeThread;

    /* TODO think about request priority and PriorityBlockingQueue */
    private BlockingQueue<JSONRPC2Message> requests = new ArrayBlockingQueue<JSONRPC2Message>(100 /* TODO place somewhere this magic constant */, true);


    public MessageWriter(Writer outputWriter, Service service) {
        if(outputWriter == null) throw new IllegalArgumentException("Unable to create writer. Output stream is null");
        if(service == null) throw new IllegalArgumentException("Unable to create writer. Service is null");

        this.outputWriter = outputWriter;
        this.service = service;
    }

    public synchronized boolean isAlive() {
        return writeThread != null && writeThread.isAlive();
    }

    public synchronized void start() {
        if(!isStarted) {
            isStarted = true;
            writeThread = new Thread(this, getClass().getSimpleName());

            service.addListener(this);
            writeThread.start();
        }
    }

    public synchronized void stop() {
        logger.info("Stopping writer");
        isClosed = true;

        if(writeThread != null) {
            Thread thread = writeThread;
            writeThread = null;
            thread.interrupt();
        }
        isStarted = false;
        service.removeListener(this);
    }

    @Override
    public void run() {
        try {
            doRun();
        } catch(InterruptedException iex) {
            logger.warn("Write loop interrupted", iex);
        } catch (Exception e) {
            logger.error("Write loop terminated", e);
        }
        logger.info("Write loop ended successfully");
        requests.clear();
    }

    private void doRun() throws InterruptedException {
        logger.info("Starting write loop");
        List<JSONRPC2Message> newMessages = new ArrayList<JSONRPC2Message>();
        boolean doubleBreak = false;

        for (; !isClosed ;) {
            if (isNeededToStop()) break;

            if(requests.size() > 0) {
                int itemsReceived = requests.drainTo(newMessages);
                if(itemsReceived > 0) {
                    for(JSONRPC2Message message: newMessages) {
                        if (isNeededToStop()) { doubleBreak = true; break;}

                        if(message != null) {
                            logger.debug("Sending new request...");
                            writeNewMessage(message);
                        }
                    }
                    newMessages.clear();

                    if(doubleBreak) break;
                }
            }
//            RPCRequest request = requests.poll(100, TimeUnit.MILLISECONDS);
//            if(request != null) {
//                logger.debug("Sending new request...");
//                writeNewMessage(request);
//            }
        }
        logger.info("Writer loop was stopped.");
    }

    private synchronized boolean isNeededToStop() {
        Thread thisThread = Thread.currentThread();
        if(thisThread.isInterrupted()) return true;
        if(writeThread != thisThread) return true;
        if(isClosed) return true;

        return false;
    }

    private void writeNewMessage(JSONRPC2Message message) {
        String data = null;
        try{
            data = message.toString();
        }catch (Exception e) {
            logger.error("Unable to get request data. ", e);
        }

        if((data != null) && (!data.isEmpty())) {
            try {
                outputWriter.write(toNetString(data));
                outputWriter.flush();
            } catch (IOException e) {
                logger.error(String.format("Unable to write data. Data %s", data) , e);
            }
        }
    }

    /**
     *  This method is converts input string parameter to netstring(<a href="http://cr.yp.to/proto/netstrings.txt">http://cr.yp.to/proto/netstrings.txt</a>)
     *  You cannot just convert string like this: String.format("%d:%s,", data.length(), data);
     *  Usually outputWriter is OutputStreamWriter, that converts characters into bytes.
     *  Java uses two byte encoding for a one char. Output consumer should know that.
     *  UTF-8 is more elegant way how to transmit encoded data.
     *  Therefore we put size of unicode byte array into netstring.
      */
    public static String toNetString(String data) {
        StringBuilder sb = new StringBuilder(data.length() + 4);
        try {
            sb.append(data.getBytes("UTF-8").length)
                .append(':')
                .append(data)
                .append(',');
        } catch (Exception e) {
            logger.error("Unable to convert data to netstring.", e);
            return "";
        }
        return sb.toString();
    }

    @Override
    public void onRequest(RPCRequest request) {
        // TODO think about how message will be added( offer vs put vs add)
        addMessageToWriteQueue(request);

    }

    private void addMessageToWriteQueue(JSONRPC2Message message) {
        if(message != null && isAlive()) {
            try {
                requests.put(message);
            } catch (InterruptedException e) {
                logger.warn("Request addition to request send queue was interrupted");
            }
        }
    }

    @Override
    public void onResponse(RPCResponse response) {
    }

    @Override
    public void onNotification(RPCNotification notification) {
        addMessageToWriteQueue(notification);
    }

    @Override
    public void onUnknownMessage(String messageText) {
    }
}
