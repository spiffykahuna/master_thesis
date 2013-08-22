package ee.ttu.deko.coffee.service.request;

import ee.ttu.deko.coffee.service.Service;
import ee.ttu.deko.coffee.service.message.MessageHandler;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RepeatedRPCRequest extends SingleRPCRequest {
    private final static Logger logger = LoggerFactory.getLogger(SingleRPCRequest.class);

    public static final int DEFAULT_REPEATS = 3;

    private int repeatCount;
    private Object requestObject;

    public RepeatedRPCRequest(MessageHandler messageHandler, Service service) {
        super(messageHandler, service);
        this.repeatCount = DEFAULT_REPEATS;
        this.timeoutMs = service.getTimeoutMs();
    }

    public RepeatedRPCRequest(MessageHandler messageHandler, Service service, int repeatCount) {
        super(messageHandler, service);
        this.timeoutMs = service.getTimeoutMs();
        this.repeatCount = repeatCount;
    }

    @Override
    protected void sendRequest(Object request) {
        this.requestObject = request;
        super.sendRequest(request);
    }

    @Override
    protected Object waitForResponse() {
        Object response = null;
        try {
            response = waiter.getResponseOrNull(this.timeoutMs);
            --repeatCount;

            while(response == null && (repeatCount-- > 0)) {
                sendRequest(this.requestObject);
                response = waiter.getResponseOrNull(this.timeoutMs);
            }

        } catch (InterruptedException e) {
            logger.debug("Request was interrupted ( Request id={})", waiter.getRequest().getID());
        }
        return response;
    }

    public void setRepeatCount(int repeatCount) {
        this.repeatCount = repeatCount;
    }

    @Override
    public RequestProcessor cloneProcessor() {
        RepeatedRPCRequest processor =  new RepeatedRPCRequest(messageHandler, service);
        processor.setTimeoutMs(this.timeoutMs);
        processor.setRepeatCount(this.repeatCount);
        return  processor;
    }
}
