package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.request.RequestProcessor;

import java.io.Reader;
import java.io.Writer;

public interface Service {
    // connect methods
    void connect(Reader inputReader, Writer outputWriter);
    boolean isConnected();
    void disconnect();

    boolean addListener(RPCServiceListener listener);
    boolean removeListener(RPCServiceListener listener);

    void setTimeoutMs(long timeoutMs);
    long getTimeoutMs();

    void setRequestProcessor(RequestProcessor requestProcessor);
}
