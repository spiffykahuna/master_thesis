package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.RPCServiceListener;
import ee.ttu.deko.coffee.service.Service;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.Writer;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

public class MessageWriter implements  Runnable, RPCServiceListener{
    private final static Logger logger = LoggerFactory.getLogger(MessageWriter.class);

    private final Service service;
    private final Writer outputWriter;

    private boolean isStarted;
    private boolean isClosed;
    private volatile Thread writeThread;

    /* TODO think about request priority and PriorityBlockingQueue */
    private BlockingQueue<RPCRequest> requests = new ArrayBlockingQueue<RPCRequest>(10 /* TODO place somethere this magic constant */, true);


    public MessageWriter(Writer outputWriter, Service service) {
        if(outputWriter == null) throw new IllegalArgumentException("Unable to create writer. Output stream is null");
        if(service == null) throw new IllegalArgumentException("Unable to create writer. Service is null");

        this.outputWriter = outputWriter;
        this.service = service;
    }

    public synchronized boolean isAlive() {
        if(writeThread != null) {
            return writeThread.isAlive();
        } else {
            return false;
        }
    }

    public synchronized void start() {
        if(!isStarted) {
            isStarted = true;
            writeThread = new Thread(this, getClass().getSimpleName());
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
    }

    private void doRun() throws InterruptedException {
        logger.info("Starting write loop");
        for (; !isClosed ;) {
            synchronized (this) {
                Thread thisThread = Thread.currentThread();
                if(thisThread.isInterrupted())  break;
                if(writeThread != thisThread)    break;
                if(isClosed)                    break;
            }

            logger.info("Waiting for new request to send...");
            RPCRequest request = requests.poll(1, TimeUnit.SECONDS);
            if(request != null) {
                logger.debug("Sending new request...");
                writeRequest(request);
            }
        }
        logger.info("Writer loop was stopped.");
    }

    private void writeRequest(RPCRequest request) {
        String data = null;
        try{
            data = request.toString();
        }catch (Exception e) {
            logger.error("Unable to get request data. ", e);
        }

        if((data != null) && (!data.isEmpty())) {
            try {
                outputWriter.write(data);
            } catch (IOException e) {
                logger.error(String.format("Unable to write data. Data %s", data) , e);
            }
        }
    }

    @Override
    public void onRequest(RPCRequest request) {
        // TODO think about how message will be added( offer vs put vs add)
        try {
            requests.put(request);
        } catch (InterruptedException e) {
            logger.warn("Request addition to request send queue was interrupted");
        }
    }

    @Override
    public void onResponse(RPCResponse response) {
    }

    @Override
    public void onNotification(RPCNotification notification) {
    }

    @Override
    public void onUnknownMessage(String messageText) {
    }
}
