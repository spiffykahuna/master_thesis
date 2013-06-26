package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.message.MessageHandler;
import ee.ttu.deko.coffee.service.message.MessageReader;
import ee.ttu.deko.coffee.service.message.MessageWriter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import java.io.Reader;
import java.io.Writer;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public abstract class AbstractCoffeeMachineService implements CoffeeMachineService {

    protected final Logger logger = LoggerFactory.getLogger(this.getClass());

    protected Reader inputReader;
    protected Writer outputWriter;
    protected MessageReader reader;
    protected MessageWriter writer;
    protected MessageHandler messageHandler;

    Set<RPCServiceListener> listeners = new CopyOnWriteArraySet<RPCServiceListener>();
    protected boolean isConnected = false;

    protected long timeoutMs;


    protected AbstractCoffeeMachineService() {
        inputReader = null;
        outputWriter = null;
        reader = null;
        writer = null;
    }

    public abstract ServiceContract getServiceContract();

    public abstract Object getInfo();

    public abstract List<Product> getProducts();

    public abstract void orderProduct(Product product);

    public abstract void cancelProduct(Product product);

    public abstract void connect(Reader inputReader, Writer outputWriter);

    @Override
    public synchronized boolean isConnected() {
        return isConnected;
    }

    public abstract void disconnect();

    @Override
    public synchronized boolean addListener(RPCServiceListener listener) {
        return listeners.add(listener);
    }

    @Override
    public synchronized boolean removeListener(RPCServiceListener listener) {
        return listeners.remove(listener);
    }

    @Override
    public synchronized void setTimeoutMs(long timeoutMs) {
        this.timeoutMs = timeoutMs;
    }

    @Override
    public synchronized long getTimeoutMs() {
        return timeoutMs;
    }

    public abstract void start();

    public abstract boolean isRunning();

    public abstract void stop();
}
