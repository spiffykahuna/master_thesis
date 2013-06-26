package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;

public interface MessageHandler {
    void handleMessage(String msg);

    void handleMessage(RPCRequest request);

    void handleMessage(RPCNotification notification);

    void handleMessage(RPCResponse response);
}
