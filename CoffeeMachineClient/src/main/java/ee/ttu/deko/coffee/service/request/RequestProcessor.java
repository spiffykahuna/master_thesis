package ee.ttu.deko.coffee.service.request;

public interface RequestProcessor {
    Object processRequest(Object request);

    RequestProcessor cloneProcessor();

    void setTimeoutMs(long timeoutMs);
    long getTimeoutMs();
}
