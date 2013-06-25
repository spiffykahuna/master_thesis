package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Response;

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
}
