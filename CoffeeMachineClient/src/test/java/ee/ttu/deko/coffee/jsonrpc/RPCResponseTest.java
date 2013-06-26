package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Response;
import org.junit.Before;
import org.junit.Test;

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

        try {
            response = RPCResponse.parse("{\"result\": \"success\", \"id\": 1}");
            fail("Invalid json-rpc message should not be parsed.");
        }catch (JSONRPC2ParseException e) {}

        // TODO check that this is valid or not and implement this in parser
        JSONRPC2Response resp = JSONRPC2Response.parse("{\"jsonrpc\": \"2.0\", \"result\": \"success\"}");
        assertNotNull(response);

//        resp = JSONRPC2Response.parse("{\"jsonrpc\": \"2.0\"}");
//        assertNotNull(response);
//        try {
//            response = RPCResponse.parse("{\"jsonrpc\": \"2.0\", \"result\": \"success\"}");
//            fail("Invalid json-rpc message should not be parsed.");
//        }catch (JSONRPC2ParseException e) {}

    }
}
