package ee.ttu.deko.coffee.service.message;

import com.thetransactioncompany.jsonrpc2.*;
import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.RPCServiceListener;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Set;

public class JsonRPCMessageHandler implements MessageHandler {
    private final static Logger logger = LoggerFactory.getLogger(MessageReader.class);

    public void setListeners(Set<RPCServiceListener> listeners) {
        this.listeners = listeners;
    }

    Set<RPCServiceListener> listeners;

    public JsonRPCMessageHandler(Set<RPCServiceListener> listeners) {
        if(listeners == null) throw new IllegalArgumentException("Listeners should not be null");
        this.listeners = listeners;
    }

    @Override
    public synchronized void handleMessage(String msg) {
        checkListeners();
        JSONRPC2Message incomeMsg = null;
        try {
            incomeMsg = JSONRPC2Message.parse(msg);
        } catch (JSONRPC2ParseException e) {
            logger.error("Unable to parse message {}", e);
        }

        if(incomeMsg instanceof JSONRPC2Request) {
            RPCRequest request = new RPCRequest((JSONRPC2Request) incomeMsg);
            handleMessage(request);
            return;
        }

        if(incomeMsg instanceof JSONRPC2Response) {
            RPCResponse response = null;
            JSONRPC2Response resp = (JSONRPC2Response) incomeMsg;
            if(resp.getError() == null) {
                response = new RPCResponse(resp);
            } else {
                response = new RPCResponse(resp.getError(), resp.getID());
            }

            handleMessage(response);
            return;
        }

        if(incomeMsg instanceof JSONRPC2Notification) {
            RPCNotification notification = new RPCNotification((JSONRPC2Notification) incomeMsg);
            handleMessage(notification);
            return;
        }

        logger.debug("UnknownMessage received: {}", msg);
        for(RPCServiceListener listener: listeners) {
            listener.onUnknownMessage(msg);
        }
    }

    public void handleMessage(RPCNotification notification) {
        checkListeners();
        logger.debug("RPCNotification received: {}", notification);
        synchronized (listeners) {
            for(RPCServiceListener listener: listeners) {
                listener.onNotification(notification);
            }
        }
    }

    public void handleMessage(RPCResponse response) {
        checkListeners();
        logger.debug("RPCResponse received: {}", response);
        synchronized (listeners) {
            for(RPCServiceListener listener: listeners) {
                listener.onResponse(response);
            }
        }
    }

    public void handleMessage(RPCRequest request) {
        checkListeners();
        logger.debug("RPCRequest received: {}", request);
        synchronized (listeners) {
            /* TODO there may be such case, when listener method blocks whole process. It might be transport delay or smth similar.
               One solution is to start a new thread for each listener
               Another is to start a new thread for all listeners.
               It depends on what will listeners do and what does handleMessage caller expect
              */
            for(RPCServiceListener listener: listeners) {
                listener.onRequest(request);
            }
        }
    }

    private synchronized void checkListeners() {
        if(listeners == null) throw new IllegalStateException("Service listeners are not initialized");
    }
}
