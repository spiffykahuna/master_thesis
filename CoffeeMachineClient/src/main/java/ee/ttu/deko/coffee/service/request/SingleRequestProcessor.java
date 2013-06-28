package ee.ttu.deko.coffee.service.request;

public abstract class SingleRequestProcessor implements RequestProcessor {
    @Override
    public Object processRequest(Object request) {
        sendRequest(request);
        return waitForResponse();
    }

    protected abstract void sendRequest(Object request);

    protected abstract Object waitForResponse();
}
