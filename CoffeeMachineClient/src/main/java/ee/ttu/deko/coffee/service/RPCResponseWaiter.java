package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;

public class RPCResponseWaiter implements RPCServiceListener {
    private final Object lock = new Object();
    private RPCRequest request;
    private RPCResponse response;
    private Service service;

    public RPCResponseWaiter(RPCRequest request, Service service) {
        if(request == null) throw new IllegalArgumentException("Unable to create responce waiter. Specified request is null");
        if(service == null) throw new IllegalArgumentException("Unable to create responce waiter. Specified service is null");

        this.service = service;
        this.request = request;
        service.addListener(this);

    }

    public RPCResponse getResponseOrNull(long timeout) throws InterruptedException {
        synchronized (lock) {
            if (response != null) {
                service.removeListener(this);
                return response;
            }

            lock.wait(timeout);
        }
        service.removeListener(this);
        return response;
    }

    public RPCResponse getResponseOrNull() throws InterruptedException {
        return getResponseOrNull(service.getTimeoutMs());
    }

    @Override
    public void onResponse(RPCResponse response) {
        if(response == null) return;

        // TODO each method should handle rpc error ( id may be null)
        synchronized (lock) {
            Object requestId = request.getID();
            Object responseId = response.getID();

            // Receiver should decide what to do with responce/error messages that don't have id.
            if( responseId == null) {
                this.response = response;
                lock.notifyAll();
            }

            if (requestId.equals(responseId)) {
                this.response = response;
                lock.notifyAll();
            }
        }
    }

    @Override
    public void onRequest(RPCRequest request) {
    }

    @Override
    public void onNotification(RPCNotification notification) {
    }

    @Override
    public void onUnknownMessage(String messageText) {
    }

    public RPCRequest getRequest() {
        return request;
    }
}
