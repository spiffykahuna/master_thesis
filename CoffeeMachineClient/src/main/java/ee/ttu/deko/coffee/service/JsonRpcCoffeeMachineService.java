package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.domain.MachineConfig;
import ee.ttu.deko.coffee.service.domain.Price;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.message.JsonRPCMessageHandler;
import ee.ttu.deko.coffee.service.message.MessageReader;
import ee.ttu.deko.coffee.service.message.MessageWriter;
import ee.ttu.deko.coffee.service.request.RepeatedRPCRequest;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import ee.ttu.deko.coffee.service.request.SingleRPCRequest;

import java.io.Reader;
import java.io.Writer;
import java.math.BigDecimal;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

public class JsonRpcCoffeeMachineService extends AbstractCoffeeMachineService {
    /*
        System.out.println(new Date(Long.MAX_VALUE));
        Sun Aug 17 09:12:55 EET 292278994
     */
//    private static AtomicLong idCounter = new AtomicLong(System.currentTimeMillis());
    private static AtomicLong idCounter = new AtomicLong(1);

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
        logger.info("Service contract was requested");
        ServiceContract contract = null;

        RPCResponse response = callRpcMethod("system.help", null);
        if(response != null) {
            try {
                contract = new ServiceContract(response.getResult());
            } catch(IllegalArgumentException iae) {
                String msg = String.format("Cannot create service contract using response result. Result: %s", response.getResult());
                logger.warn(msg, iae);
            }
        } else {
            logger.warn("Cannot get response for service contract request");
        }

        // TODO each method should handle rpc error ( id may be null)
        // TODO compare error codes (between JSONRPC2Error and my embedded server)

        return contract;
    }

    private RPCResponse callRpcMethod(String methodName, Object params) {
        long id = nextId();
        RPCResponse response = null;

        RPCRequest request = new RPCRequest(methodName, id);

        if(params != null) {
            if(params instanceof Map) {
                request.setNamedParams(getStringObjectMap((Map) params));
            } else if(params instanceof List) {
                request.setPositionalParams(getObjectList((List) params));
            } else {
                logger.error(
                        "Unknown request parameters object was specified. This should be a List<Object> or Map<String, Object>. Got {} {}"
                        , params.getClass(), params
                );
            }
        }

        logger.debug("Processing new request: {}", request);

        RequestProcessor processor = requestProcessor.cloneProcessor();
        response = processRPCRequest(processor, request);

        logger.debug("Request processor returned new responce {}", response);

        if(response != null) {
            logger.debug("Checking response...");

            if(response.getError() != null) {
                logger.warn("Response has error: {}", response.getError());
                return null;
            }

            if(!request.getID().equals(response.getID())) {
                logger.warn(
                        "Request and response identificators do not match. Request id: {} Response id: {}"
                        , request.getID(), response.getID()
                );
                return null;
            }
        } else {
            logger.warn("Service timeout reached or error occurred. Request: {}", request);
        }
        return response;
    }

    private List<Object> getObjectList(List objects) {
        List<Object> paramObjects = new ArrayList<Object>(((List) objects).size());
        for(Object o: (List) objects) {
            paramObjects.add(o);
        }
        return paramObjects;
    }

    private RPCResponse processRPCRequest(RequestProcessor processor, RPCRequest request) {
        RPCResponse response = null;

        Object processorResponse = processor.processRequest(request);
        if(processorResponse instanceof RPCResponse) {
            response = (RPCResponse) processorResponse;
        } else {
            logger.warn("Processor returned invalid response: {}", processorResponse);
            response = null;
        }
        return response;
    }

    @Override
    public Map<String, Object> getInfo() {
        logger.info("Service information was requested");
        Object info = null;

        RPCResponse response = callRpcMethod("machine.getInfo", null);
        if(response != null) {
            info = response.getResult();
        } else {
            logger.warn("Cannot get response for service contract request");
            return null;
        }

        return getStringObjectMap((Map) info);
    }

    @Override
    public Object getServiceInfo() {
        return null;
    }

    @Override
    public void configCoffeeMachine(MachineConfig config) {
    }

    @Override
    public MachineConfig getCoffeeMachineConfig() {
        return null;
    }

    @Override
    public synchronized List<Product> getProducts() {
        logger.info("List of products was requested");

        List<Product>  productList = null;

        RequestProcessor processor = requestProcessor.cloneProcessor();

        RPCResponse response = callRpcMethod("machine.getProducts", null);
        if(response != null) {
            Object result = response.getResult();
            if(result instanceof List) {
                productList = createProductListFromResponse((List) result);
            } else {
                logger.warn("Received object is not a list. Valid list of products should be returned");
                return null;
            }
        } else {
            logger.warn("Cannot get response for service contract request");
        }
        return productList;
    }

    private List<Product> createProductListFromResponse(List products) {
        if(products == null)  throw new IllegalArgumentException("Received products list object is null");
        List<Product> result = new ArrayList<Product>(products.size());

        for(Object prod: products) {
            Product product = createProductFromResponseResult(prod);
            result.add(product);
        }
        return result;
    }

    private Product createProductFromResponseResult(Object productObj) {
        Product newProduct = null;
        if(!(productObj instanceof Map)) throw new IllegalArgumentException("Unable to create new newProduct object. Received responseObj is not Map");


        Map<String, Object> newProductObj = getStringObjectMap((Map) productObj);

        try {
            int productId = ((Long) newProductObj.get("id")).intValue();
            String name = (String) newProductObj.get("name");
            Map<String, Object> priceMap = getStringObjectMap((Map) newProductObj.get("price"));
            Number amount = (Number) priceMap.get("amount");
            String currencyText = (String) priceMap.get("currency");
            Price price = new Price(new BigDecimal(amount.toString()), Currency.getInstance(currencyText));
            newProduct = new Product(productId, name, price);

        } catch(IllegalArgumentException iae) {
            logger.warn("Unable to create new product. Received product: {}. Error: {}", newProductObj, iae.getMessage());
        } catch(ClassCastException cce) {
            logger.warn("Unable to create new product. Unable to extract product parameters.", cce);
        }

        return newProduct;
    }

    private Map<String, Object> getStringObjectMap(Map productObj) {
        if(productObj == null) throw new IllegalArgumentException("Received map object is null");
        Map<String, Object> newProductObj = new HashMap<String, Object>();
        Map temp =  (Map) productObj;
        Iterator props = temp.entrySet().iterator();
        while(props.hasNext()) {
            Map.Entry property = (Map.Entry) props.next();
            newProductObj.put((String) property.getKey(), property.getValue());
        }
        return newProductObj;
    }

    private synchronized Product.Status makeProductRequest(String method, int productId) {
        Product.Status status = null;

        Map<String, Object> params = new HashMap<String, Object>();
        params.put("id", Integer.valueOf(productId));


        RPCResponse response = callRpcMethod(method, params);
        if(response != null) {
            Object result = response.getResult();
            if(result != null && result instanceof String && !((String) result).isEmpty() ) {
                try{
                    status = Product.Status.valueOf((String) result);
                } catch(IllegalArgumentException iae) {
                    logger.warn("Server returned wrong status code {}", result);
                    return null;
                }
            } else {
                logger.warn("Received object is not a valid product status string.");
                return null;
            }
        } else {
            logger.warn("Cannot get response for product request");
        }
        return status;
    }


    @Override
    public synchronized Product.Status orderProduct(int productId) {
        logger.info("Requested order for product: {}", productId);
        return makeProductRequest("machine.orderProduct", productId);
    }

    @Override
    public synchronized Product.Status cancelProduct(int productId) {
        logger.info("Requested cancel for product: {}", productId);
        return makeProductRequest("machine.cancelProduct", productId);
    }

    @Override
    public synchronized Product.Status getProductStatus(int productId) {
        logger.info("Requested product status for product: {}", productId);
        return makeProductRequest("machine.getProductStatus", productId);
    }

    @Override
    public void login(String password) {
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


    public RequestProcessor getRequestProcessorByType(Class requestProcessorClass, Object... params) {
        RequestProcessor processor = null;

        if(SingleRPCRequest.class.equals(requestProcessorClass)) {
            processor = new SingleRPCRequest(messageHandler, this);
            if(params[0] != null) {
                processor.setTimeoutMs((Integer) params[0]);
            } else {
                processor.setTimeoutMs(this.timeoutMs);
            }

        }

        if(RepeatedRPCRequest.class.equals(requestProcessorClass)) {
            RepeatedRPCRequest repeatedRPCRequest =  new RepeatedRPCRequest(messageHandler, this);



            if(params[0] != null) {
                repeatedRPCRequest.setTimeoutMs((Integer) params[0]);
            } else {
                repeatedRPCRequest.setTimeoutMs(this.timeoutMs);
            }

            if(params[1] != null) {
                repeatedRPCRequest.setRepeatCount((Integer) params[1]);
            }

            processor = repeatedRPCRequest;
        }

        return processor;
    }
}
