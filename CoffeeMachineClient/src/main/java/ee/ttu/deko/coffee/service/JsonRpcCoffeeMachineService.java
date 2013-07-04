package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.message.JsonRPCMessageHandler;
import ee.ttu.deko.coffee.service.message.MessageReader;
import ee.ttu.deko.coffee.service.message.MessageWriter;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import ee.ttu.deko.coffee.service.request.SingleRPCRequest;

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
        requestProcessor = new SingleRPCRequest(messageHandler, this);
    }

    /**
     *
     * @return  Returns ServiceContract with description of a service. Returns null on timeout or error.
     */
    @Override
    public ServiceContract getServiceContract() {
        logger.debug("Service contract was requested");
        ServiceContract contract = null;

        long id = nextId();
        RequestProcessor processor = requestProcessor.cloneProcessor();

        RPCRequest request = new RPCRequest("system.help", id);

        logger.debug("Processing contract request: {}", request);

        RPCResponse response = processRPCRequest(processor, request);

        logger.debug("Request processor returned new responce {}", response);

        if(response != null && (response.getError() == null)) {
            logger.debug("Creating new service contract using result of response");

            if(!request.getID().equals(response.getID())) {
                logger.warn(
                    "Request and response identificators do not match. Request id: {} Response id: {}"
                    , request.getID(), response.getID()
                );
                return null;
            }

            try {
                contract = new ServiceContract(response.getResult());
            } catch(IllegalArgumentException iae) {
                logger.warn("Cannot create service contract using response result. Result: {}", response.getResult());
            }
        } else {
            logger.warn("Service timeout reached or error occurred. Request: {}", request);
        }


        // TODO each method should handle rpc error ( id may be null)
        // TODO compare error codes (between JSONRPC2Error and my embedded server)

        return contract;
    }

    private RPCResponse processRPCRequest(RequestProcessor processor, RPCRequest request) {
        RPCResponse response = null;

        Object processorResponce = processor.processRequest(request);
        if(processorResponce instanceof RPCResponse) {
            response = (RPCResponse) processorResponce;
        } else {
            logger.warn("Processor returned invalid response: {}", processorResponce);
            response = null;
        }
        return response;
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
        writer = new MessageWriter(outputWriter, this);

        isConnected = true;
    }

    @Override
    public synchronized void disconnect() {
        if(isRunning()) throw new IllegalStateException("Unable to disconnect service. Service is running. Please stop service first.");
        // TODO implement service disconnect
        isConnected = false;
        reader = null;
        writer = null;
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
        if(reader == null) return false;
        if(writer == null) return false;

        return (isConnected && (reader.isAlive() || writer.isAlive()));
    }

    @Override
    public synchronized void stop() {
        if((reader == null)) throw new IllegalStateException("Unable to stop service. Reader is not initialized");
        if((writer == null)) throw new IllegalStateException("Unable to stop service. Writer is not initialized");

        if(reader.isAlive()) reader.stop();
        if(writer.isAlive()) writer.stop();
    }

    private Long nextId() {
        return idCounter.incrementAndGet();
    }
}
