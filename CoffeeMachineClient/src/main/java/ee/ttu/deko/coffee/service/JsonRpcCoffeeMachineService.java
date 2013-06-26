package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.message.JsonRPCMessageHandler;
import ee.ttu.deko.coffee.service.message.MessageReader;
import ee.ttu.deko.coffee.service.message.MessageWriter;

import java.io.Reader;
import java.io.Writer;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

public class JsonRpcCoffeeMachineService extends AbstractCoffeeMachineService {
    /*
        System.out.println(new Date(Long.MAX_VALUE));
        Sun Aug 17 09:12:55 EET 292278994
     */
    private static AtomicLong idCounter = new AtomicLong(System.currentTimeMillis());

    public JsonRpcCoffeeMachineService() {
        if(listeners == null) throw new IllegalStateException("Unable to create service. Service listeners collection is null");
        messageHandler = new JsonRPCMessageHandler(listeners);
    }

    @Override
    public ServiceContract getServiceContract() {
        long id = nextId();
        RPCRequest request = new RPCRequest("system.help", id);
        messageHandler.handleMessage(request);

        // TODO each method should handle rpc error ( id may be null)
        // TODO compare error codes (between JSONRPC2Error and my embedded server)
        return null;
    }

    @Override
    public Object getInfo() {
        return null;  
    }

    @Override
    public synchronized List<Product> getProducts() {
        return null;  
    }

    @Override
    public synchronized void orderProduct(Product product) {
        
    }

    @Override
    public synchronized void cancelProduct(Product product) {
        
    }

    @Override
    public synchronized void connect(Reader inputReader, Writer outputWriter) {
        if((inputReader == null)) throw new IllegalArgumentException("Unable to connect service. Reader is null");
        if((outputWriter == null)) throw new IllegalArgumentException("Unable to connect service. Writer is null");

        inputReader = inputReader;
        outputWriter = outputWriter;

        if(messageHandler == null) throw new IllegalStateException("Unable to create reader. Message handler is null");

        reader = new MessageReader(inputReader, messageHandler);
        writer = new MessageWriter(outputWriter);

        isConnected = true;
    }

    @Override
    public synchronized void disconnect() {
        if(isRunning()) throw new IllegalStateException("Unable to disconnect service. Service is running. Please stop service first.");
        // TODO implement service disconnect
        isConnected = false;
    }

    @Override
    public synchronized void start() {
        if((reader == null)) throw new IllegalStateException("Unable to start service. Reader is not connected");
        if((writer == null)) throw new IllegalStateException("Unable to start service. Writer is not connected");

        if(!reader.isAlive()) reader.start();
        if(!writer.isAlive()) writer.start();
    }

    @Override
    public synchronized boolean isRunning() {
        return (reader.isAlive() || writer.isAlive());
    }

    @Override
    public synchronized void stop() {
        if((reader == null)) throw new IllegalStateException("Unable to stop service. Reader is not initialized");
        if((writer == null)) throw new IllegalStateException("Unable to stop service. Writer is not initialized");

        if(reader.isAlive()) reader.close();
        if(writer.isAlive()) writer.close();
    }

    private Long nextId() {
        return idCounter.incrementAndGet();
    }
}