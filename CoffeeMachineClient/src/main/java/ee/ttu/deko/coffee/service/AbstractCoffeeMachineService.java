package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
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

    Set<ServiceListener> listeners = new CopyOnWriteArraySet<ServiceListener>();
    protected boolean isConnected = false;

    protected AbstractCoffeeMachineService() {
        inputReader = null;
        outputWriter = null;
        reader = null;
        writer = null;
    }

    @Override
    public synchronized ServiceContract getServiceContract() {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized Object getInfo() {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized List<Product> getProducts() {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized void orderProduct(Product product) {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized void cancelProduct(Product product) {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized void connect(Reader inputReader, Writer outputWriter) {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized boolean isConnected() {
        return isConnected;
    }

    @Override
    public synchronized void disconnect() {
        throw new UnsupportedOperationException();
    }

    @Override
    public synchronized void addListener(ServiceListener listener) {
        listeners.add(listener);
    }

    @Override
    public synchronized void removeListener(ServiceListener listener) {
        listeners.remove(listener);
    }
}
