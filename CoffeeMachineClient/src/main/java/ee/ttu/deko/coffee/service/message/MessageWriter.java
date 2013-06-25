package ee.ttu.deko.coffee.service.message;

import java.io.Writer;

public class MessageWriter {
    private boolean isStarted;

    public MessageWriter(Writer outputWriter) {
        if(outputWriter == null) throw new IllegalArgumentException("Unable to create writer. Stream is null");
        outputWriter = outputWriter;
    }

    public synchronized boolean isAlive() {
        return isStarted;  //TODO
    }

    public synchronized void start() {

        isStarted = true;
    }

    public synchronized void close() {
        isStarted = false;
    }
}
