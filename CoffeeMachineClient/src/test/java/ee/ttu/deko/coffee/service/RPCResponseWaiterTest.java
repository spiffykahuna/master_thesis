package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import org.hamcrest.MatcherAssert;
import org.junit.Before;
import org.junit.Test;

import java.io.Reader;
import java.io.Writer;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import static org.hamcrest.CoreMatchers.hasItem;
import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.*;

public class RPCResponseWaiterTest {
    class TestService implements Service {
        public Set<RPCServiceListener> listeners = new HashSet<RPCServiceListener>();
        public long timeoutMs;
        private boolean isConnected;

        @Override
        public void connect(Reader inputReader, Writer outputWriter) {
            isConnected = true;
        }

        @Override
        public boolean isConnected() {
            return isConnected;
        }

        @Override
        public void disconnect() {
            isConnected = false;
        }

        @Override
        public boolean addListener(RPCServiceListener listener) {
            return listeners.add(listener);
        }

        @Override
        public boolean removeListener(RPCServiceListener listener) {
            return listeners.remove(listener);
        }

        @Override
        public void setTimeoutMs(long timeoutMs) {
            this.timeoutMs = timeoutMs;
        }

        @Override
        public long getTimeoutMs() {
            return timeoutMs;
        }
    }

    TestService service;
    RPCRequest request;
    RPCResponse response;
    RPCResponseWaiter waiter;

    @Before
    public void setUp() throws Exception {
        service = new TestService();
        request = new RPCRequest("someMethod", 1);
        response = new RPCResponse("success", 1);
    }

    @Test(timeout = 2000)
    public void testGetResponse() throws Exception {

        RPCRequest request = new RPCRequest("someMethod", 1);

        waiter = new RPCResponseWaiter(request, service);

        assertThat(service.listeners, hasItem(waiter));

        RPCResponse response = new RPCResponse("success", 1);

        waiter.onResponse(response);
        assertEquals(response, waiter.getResponse(0));
    }

    @Test(timeout = 2000)
    public void waiterAddsAndRemovesListenersToService() throws Exception {

        for (int i = 1; i <= 1000; i++) {
            waiter = new RPCResponseWaiter(request, service);
            assertEquals(i, service.listeners.size());
        }
        service.listeners.clear();

        for (int i = 1; i <= 1000; i++) {
            waiter = new RPCResponseWaiter(request, service);
            assertEquals(1, service.listeners.size());
            waiter.onResponse(response);
            waiter.getResponse(0);
            assertEquals(0, service.listeners.size());
        }
    }

    @Test(timeout = 2000)
    public void waiterAcceptsErrorMessagesWithNullId() throws Exception {
        waiter = new RPCResponseWaiter(request, service);

        response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}\n");
        assertNotNull(response);
        assertNotNull(response.getError());

        waiter.onResponse(response);
        assertEquals(response, waiter.getResponse(0));
    }

    @Test
    public void testConstructor() throws Exception {
        assertNotNull(request);
        assertNotNull(service);

        try{
            waiter = new RPCResponseWaiter(null, service);
            fail("Null request should throw exception");
        } catch(Exception e) {}

        try{
            waiter = new RPCResponseWaiter(request, null);
            fail("Null service should throw exception");
        } catch(Exception e) {}

        try{
            waiter = new RPCResponseWaiter(null, null);
            fail("Null request or null service should throw exception");
        } catch(Exception e) {}
    }
}
