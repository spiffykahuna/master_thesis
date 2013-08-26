package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Response;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import static org.junit.Assert.*;

public class RPCResponseTest {
    RPCResponse response;

    //TODO string --> request parsing
    @Test
    public void parseError() throws Exception {
        response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}\n");
        assertNotNull(response);
        assertNotNull(response.getError());
        assertNull(response.getResult());
        assertNull(response.getID());
        assertEquals(-32600, response.getError().getCode());
        assertEquals("Invalid Request", response.getError().getMessage());

        response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\", \"data\": \"someData\"}, \"id\": null}\n");
        assertNotNull(response);
        assertNotNull(response.getError());
        assertNull(response.getResult());
        assertNull(response.getID());
        assertEquals(-32600, response.getError().getCode());
        assertEquals("Invalid Request", response.getError().getMessage());
        assertEquals("someData", response.getError().getData());
    }

    @Test
    public void parseResponse() throws Exception {
        response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"result\": \"success\", \"id\": 1}");
        assertNotNull(response);
        assertNull(response.getError());
        assertNotNull(response.getResult());
        assertNotNull(response.getID());
        assertEquals(1L, response.getID());
        assertEquals("success", response.getResult());

        List<String> invalidMessages = new ArrayList<String>() {{
            // no jsonrpc
            add("{\"result\": \"success\", \"id\": 1}");

            // wrong jsonrpc version
            add("{\"jsonrpc\": \"1.9\", \"result\": \"success\", \"id\": 1}");

            // no result or error
            add("{\"jsonrpc\": \"2.0\", \"id\": 1}");

            // error and result together
            add("{\"jsonrpc\": \"2.0\", \"result\": \"success\",  \"error\": {\"code\": -32600, \"message\": \"Invalid Request\", \"data\": \"someData\"}, \"id\": 1}");

            // no id
            add("{\"jsonrpc\": \"2.0\", \"result\": \"success\"}");
        }};

        testRequestStrings(invalidMessages, false);

        List<String>   validMessages =  new ArrayList<String>() {{
            // additional json members
            add("{\"jsonrpc\": \"2.0\", \"result\": \"success\", \"id\": 1, \"transport\": \"serial\"}");
            add("{ \"date\": \"today\", \"jsonrpc\": \"2.0\", \"result\": \"success\", \"id\": 1}");
            add("{\"jsonrpc\": \"2.0\",\"number\": 42, \"result\": \"success\", \"id\": 1}");
            add("{\"jsonrpc\": \"2.0\",\"bool\": false, \"result\": \"success\", \"id\": 1}");
            add("{\"jsonrpc\": \"2.0\",\"no_value\": null, \"result\": \"success\", \"id\": 1}");
            add("{\"jsonrpc\": \"2.0\",\"true_value\": true, \"result\": \"success\", \"id\": 1}");
            add("{\"jsonrpc\": \"2.0\",\"false_value\": false, \"result\": \"success\", \"id\": 1}");
        }};
        testRequestStrings(validMessages, true);
    }

    @Test
    public void rpcErrorShouldHaveCodeAndMessage() throws Exception {
        // valid error
        response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}\n");
        assertNotNull(response);
        assertNotNull(response.getError());
        assertNull(response.getResult());
        assertNull(response.getID());
        assertEquals(-32600, response.getError().getCode());
        assertEquals("Invalid Request", response.getError().getMessage());

        List<String> invalidMessages = new ArrayList<String>() {{
            // no code
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"message\": \"Invalid Request\"}, \"id\": null}");

            // no message
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32600}, \"id\": null}\n");

            // empty error member
            add("{\"jsonrpc\": \"2.0\", \"error\": {}, \"id\": null}\n");

            // code is wrong type
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": \"-32600\", \"message\": \"Invalid Request\"}, \"id\": null}\n");
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": null, \"message\": \"Invalid Request\"}, \"id\": null}\n");
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": false, \"message\": \"Invalid Request\"}, \"id\": null}\n");
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": true, \"message\": \"Invalid Request\"}, \"id\": null}\n");
            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": 3.26, \"message\": \"Invalid Request\"}, \"id\": null}\n");
            // < Integer.MIN_VALUE = -2147483648
            //  this will return overflowed int object.
            //  So this might be a bug, but error code generally should be small enough to place it to integer value
//            int invalid = new Long("-2147483650").intValue();
//            add("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -2147483650, \"message\": \"Invalid Request\"}, \"id\": null}\n");
        }};

        testRequestStrings(invalidMessages, false);

    }

    private void testRequestStrings(Collection<String> jsonRPCmessages, boolean areValid) throws JSONRPC2ParseException {
        RPCResponse response;
        if(areValid) {
            for(String rpcJson: jsonRPCmessages) {
                response = RPCResponse.parse(rpcJson);
            }
        } else {
            for(String rpcJson: jsonRPCmessages) {
                try {
                    response = RPCResponse.parse(rpcJson);
                    fail("Invalid json-rpc message should not be parsed. Json: " + rpcJson);
                } catch (JSONRPC2ParseException e) {}
            }
        }
    }
}
