package ee.ttu.deko.coffee.jsonrpc;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import org.junit.Before;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

import static junit.framework.Assert.assertEquals;
import static junit.framework.TestCase.assertTrue;

public class RPCRequestTest {
    private Map<String,Object> params;
    private String id;

    @Before
    public void setUp() throws Exception {
        params = new HashMap<String,Object>();
        params.put("recipient", "me");
        params.put("age", new Long(24));  /* <-- numbers are longs */

        id = "req-001";
    }

    @Test
    public void testToString() throws Exception {
        RPCRequest request = new RPCRequest("someMethod", params, id);
        String jsonString = request.toString();

        assertTrue(jsonString.matches(".*\"jsonrpc\".*:.*\"2.0\".*"));
        assertTrue(jsonString.matches(".*\"id\".*:.*\"req-001\".*"));
        assertTrue(jsonString.matches(".*\"recipient\".*:.*\"me\".*"));
        assertTrue(jsonString.matches(".*\"age\".*:.*24.*"));
    }

    @Test
    public void testParseJsonString() throws Exception {
        RPCRequest request = new RPCRequest("someMethod", params, id);
        String jsonString = request.toString();

        RPCRequest parsedRequest = RPCRequest.parse(jsonString);
        assertTrue(request.equals(parsedRequest));
    }
}
