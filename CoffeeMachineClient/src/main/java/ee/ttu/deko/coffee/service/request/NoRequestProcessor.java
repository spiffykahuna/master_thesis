package ee.ttu.deko.coffee.service.request;

/**
 * Default behaviour class.
 */
public class NoRequestProcessor implements RequestProcessor {
    @Override
    public Object processRequest(Object request) {
        return null;
    }

    @Override
    public RequestProcessor cloneProcessor() {
        return new NoRequestProcessor();
    }

    @Override
    public void setTimeoutMs(long timeoutMs) {
    }

    @Override
    public long getTimeoutMs() {
        return 0;
    }
}
