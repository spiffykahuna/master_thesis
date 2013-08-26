package ee.ttu.deko.coffee.service.request;

public abstract class SingleRequestProcessor implements RequestProcessor {
    public long timeoutMs;

    @Override
    public Object processRequest(Object request) {
        sendRequest(request);
        return waitForResponse();
    }

    protected abstract void sendRequest(Object request);

    protected abstract Object waitForResponse();

    @Override
    public synchronized void setTimeoutMs(long timeoutMs) {
        if(timeoutMs < 0) throw new IllegalArgumentException("Timeout should be positive. Wrong value specified");
        this.timeoutMs = timeoutMs;
    }

    @Override
    public synchronized long getTimeoutMs() {
        return timeoutMs;
    }
}
