package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
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
        super(response.getResult(), response.getID());
        if(response.getNonStdAttributes() != null && response.getNonStdAttributes().size() > 0) {
            for(Map.Entry<String, Object> attribute: response.getNonStdAttributes().entrySet()) {
                appendNonStdAttribute(attribute.getKey(), attribute.getValue());
            }
        }
    }
}
