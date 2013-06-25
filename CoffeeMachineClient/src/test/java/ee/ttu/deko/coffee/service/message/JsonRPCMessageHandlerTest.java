package ee.ttu.deko.coffee.service.message;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Message;
import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.ServiceListener;
import org.junit.Before;
import org.junit.Test;

import java.util.*;
import java.util.concurrent.CopyOnWriteArraySet;

import static org.junit.Assert.assertEquals;

public class JsonRPCMessageHandlerTest {
    JsonRPCMessageHandler handler;
    TestListener listener;
    Set<ServiceListener> listeners;

    public class TestListener implements ServiceListener {
        public List<RPCRequest> requests = new ArrayList<RPCRequest>();
        public List<RPCResponse> responses = new ArrayList<RPCResponse>();
        public List<RPCNotification> notifications = new ArrayList<RPCNotification>();
        public List<String> unknownMessages = new ArrayList<String>();
        @Override
        public void onRequest(RPCRequest request) {
            requests.add(request);
        }

        @Override
        public void onResponse(RPCResponse response) {
            responses.add(response);
        }

        @Override
        public void onNotification(RPCNotification notification) {
            notifications.add(notification);
        }

        @Override
        public void onUnknownMessage(String messageText) {
            unknownMessages.add(messageText);
        }
    }

    private Map.Entry<String, Object> field(String key, Object value) {
        return  new AbstractMap.SimpleEntry<String, Object>(key, value);
    }

    @Before
    public void setUp() throws Exception {
        listener = new TestListener();
        listeners = new CopyOnWriteArraySet<ServiceListener>() {{
            add(listener);
        }};
        handler = new JsonRPCMessageHandler(listeners);
    }

    private void assertContainsValue(Collection<? extends JSONRPC2Message> messages,
                                     Map.Entry<String, Object> field, boolean isPresent ) {
        boolean valueFound = false;
        for(JSONRPC2Message msg: messages) {
            for(Map.Entry<String, Object> property: msg.toJSONObject().entrySet()) {
                if(field.getKey().equals(property.getKey())
                        && field.getValue().equals(property.getValue())) {
                    valueFound = true;
                }
            }
        }
        assertEquals(String.format("Value  %s is %s found in %s", field, (isPresent == true ? "not": ""),  messages), isPresent, valueFound);
    }

    private void assertContainsKey(Collection<? extends JSONRPC2Message> messages, String key, boolean isPresent) {
        boolean keyFound = false;
        for(JSONRPC2Message msg: messages) {
            for(Map.Entry<String, Object> property: msg.toJSONObject().entrySet()) {
                if(property.getKey().equals(key)) {
                    keyFound = true;
                }
            }
        }
        assertEquals(String.format("Key  %s is %s found in %s", key, (isPresent == true ? "not": ""), messages), isPresent, keyFound);
    }

    @Test
    public void handlesRequest() throws Exception {
        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42, 23], \"id\": 1}");

        assertEquals(1, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.requests, field("id", Long.valueOf(1)), true);
        assertContainsValue(listener.requests, field("method", "subtract"), true);
        assertContainsValue(listener.requests, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.requests, field("params", new ArrayList<Long>() {{
            add(Long.valueOf(42));
            add(Long.valueOf(23));
        }}), true);

        assertContainsKey(listener.requests, "result", false);
        assertContainsKey(listener.requests, "error", false);

        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": {\"subtrahend\": 23, \"minuend\": 42}, \"id\": 3}");
        assertEquals(2, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.requests, field("id", Long.valueOf(3)), true);
        assertContainsValue(listener.requests, field("method", "subtract"), true);
        assertContainsValue(listener.requests, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.requests, field("params", new HashMap<String, Long>() {{
            put("minuend", Long.valueOf(42));
            put("subtrahend", Long.valueOf(23));
        }}), true);

        assertContainsKey(listener.requests, "result", false);
        assertContainsKey(listener.requests, "error", false);
    }

    @Test
    public void handlesResponse() throws Exception {
        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"result\": 19, \"id\": 1}");

        assertEquals(0, listener.requests.size());
        assertEquals(1, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.responses, field("id", Long.valueOf(1)), true);

        assertContainsValue(listener.responses, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.responses, field("result", Long.valueOf(19)), true);

        assertContainsKey(listener.requests, "method", false);

        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32601, \"message\": \"Method not found\"}, \"id\": \"1\"}");
        assertEquals(0, listener.requests.size());
        assertEquals(2, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.responses, field("id", "1"), true);

        assertContainsValue(listener.responses, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.responses, field("error", new HashMap<String, Object>() {{
            /* code
                A Number that indicates the error type that occurred.
                This MUST be an integer.   JSON-RPC 2.0 Specification
            */
            put("code", Integer.valueOf(-32601));
            put("message", "Method not found");
        }}), true);

        assertContainsKey(listener.requests, "method", false);
    }

    @Test
    public void handlesNotification() throws Exception {
        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"method\": \"foobar\"}");

        assertEquals(0, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(1, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.notifications, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.notifications, field("method", "foobar"), true);

        assertContainsKey(listener.notifications, "id", false);
        assertContainsKey(listener.notifications, "result", false);
        assertContainsKey(listener.notifications, "error", false);

        handler.handleMessage("{\"jsonrpc\": \"2.0\", \"method\": \"notify_sum\", \"params\": [1,2,4]}");
        assertEquals(0, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(2, listener.notifications.size());
        assertEquals(0, listener.unknownMessages.size());

        assertContainsValue(listener.notifications, field("method", "notify_sum"), true);
        assertContainsValue(listener.notifications, field("jsonrpc", "2.0"), true);
        assertContainsValue(listener.notifications, field("params", new ArrayList<Long>() {{
            add(Long.valueOf(1));
            add(Long.valueOf(2));
            add(Long.valueOf(4));
        }}), true);

        assertContainsKey(listener.notifications, "id", false);
        assertContainsKey(listener.notifications, "result", false);
        assertContainsKey(listener.notifications, "error", false);
    }

    @Test
    public void handleInvalidJson() throws Exception {
        String invalidJson = "{\"jsonrpc\": \"2.0\", \"method\": \"foobar, \"params\": \"bar\", \"baz]";
        handler.handleMessage(invalidJson);

        assertEquals(0, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(1, listener.unknownMessages.size());

        assertEquals(invalidJson,listener.unknownMessages.get(0));

        invalidJson = "{\"jsonrpc\": \"2.0\", \"method\"";
        handler.handleMessage(invalidJson);

        assertEquals(0, listener.requests.size());
        assertEquals(0, listener.responses.size());
        assertEquals(0, listener.notifications.size());
        assertEquals(2, listener.unknownMessages.size());

        assertEquals(invalidJson,listener.unknownMessages.get(1));
    }
}
