package ee.ttu.deko.coffee.service.request;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.service.JsonRpcCoffeeMachineService;
import ee.ttu.deko.coffee.service.RPCResponseWaiter;
import ee.ttu.deko.coffee.service.Service;
import ee.ttu.deko.coffee.service.message.MessageHandler;
import ee.ttu.deko.coffee.service.message.MessageReader;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SingleRPCRequest extends SingleRequestProcessor {
    private final static Logger logger = LoggerFactory.getLogger(SingleRPCRequest.class);

    protected MessageHandler messageHandler;
    protected Service service;
    protected RPCResponseWaiter waiter;

    public SingleRPCRequest(MessageHandler messageHandler, Service service) {
        if(messageHandler == null) throw new IllegalArgumentException("Unable to create new request provessor. Message handler is null");
        if(service == null) throw new IllegalArgumentException("Unable to create new request provessor. Service is null");

        this.service = service;
        this.messageHandler = messageHandler;
    }

    @Override
    protected void sendRequest(Object request) {
        logger.debug("Sending request: {}", request.toString());

        if(request != null) {
            RPCRequest rpcRequest = (RPCRequest) request;

            messageHandler.handleMessage(rpcRequest);

            waiter = new RPCResponseWaiter(rpcRequest, service);
        }
        logger.debug("Request was sent");
    }

    @Override
    protected Object waitForResponse() {
        Object response = null;
        try {
            response = waiter.getResponseOrNull();
        } catch (InterruptedException e) {
            logger.debug("Request was interrupted ( Request id={})", waiter.getRequest().getID());
        }
        return response;
    }
}
