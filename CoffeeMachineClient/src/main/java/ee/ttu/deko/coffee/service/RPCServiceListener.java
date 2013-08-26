package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;

public interface RPCServiceListener {
    void onRequest(RPCRequest request);

    void onResponse(RPCResponse response);

    void onNotification(RPCNotification notification);

    // TODO onRpcError or error == response

    void onUnknownMessage(String messageText);
}
