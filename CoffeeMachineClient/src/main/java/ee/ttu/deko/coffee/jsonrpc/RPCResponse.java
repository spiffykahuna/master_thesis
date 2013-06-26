package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Response;

import java.util.Map;

public class RPCResponse extends JSONRPC2Response {

    public RPCResponse(Object result, Object id) {
        super(result, id);
    }

    public RPCResponse(Object id) {
        super(id);
    }

    public RPCResponse(JSONRPC2Error error, Object id) {
        super(error, id);
    }

    public RPCResponse(JSONRPC2Response response) {
        super(response.getID());
        if(response.getNonStdAttributes() != null && response.getNonStdAttributes().size() > 0) {
            for(Map.Entry<String, Object> attribute: response.getNonStdAttributes().entrySet()) {
                appendNonStdAttribute(attribute.getKey(), attribute.getValue());
            }
        }
        try {
            setResult(response.getResult());
        } catch (IllegalArgumentException e) {

        }

        try {
            setError(response.getError());
        } catch (IllegalArgumentException e) {

        }

    }

    public static RPCResponse parse(final String jsonString)
            throws JSONRPC2ParseException {
        RPCResponse response = null;
        response = new RPCResponse(JSONRPC2Response.parse(jsonString));

        return response;
    }
}
