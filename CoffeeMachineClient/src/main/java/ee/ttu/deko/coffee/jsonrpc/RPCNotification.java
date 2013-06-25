package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Notification;

import java.util.List;
import java.util.Map;

public class RPCNotification extends JSONRPC2Notification {

    public RPCNotification(String method) {
        super(method);
    }

    public RPCNotification(String method, List<Object> positionalParams) {
        super(method, positionalParams);
    }

    public RPCNotification(String method, Map<String, Object> namedParams) {
        super(method, namedParams);
    }

    public RPCNotification(JSONRPC2Notification notification) {
        super(notification.getMethod());
        setPositionalParams(notification.getPositionalParams());
        setNamedParams(notification.getNamedParams());

        if(notification.getNonStdAttributes() != null && notification.getNonStdAttributes().size() > 0) {
            for(Map.Entry<String, Object> attribute: notification.getNonStdAttributes().entrySet()) {
                appendNonStdAttribute(attribute.getKey(), attribute.getValue());
            }
        }
    }
}
