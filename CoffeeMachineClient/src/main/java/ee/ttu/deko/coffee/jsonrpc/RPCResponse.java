package ee.ttu.deko.coffee.jsonrpc;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Response;

import java.io.IOException;
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

    public static RPCResponse parse(final String jsonRPCString)
            throws JSONRPC2ParseException {

        if(jsonRPCString == null) throw new IllegalArgumentException("Provided string is null.");

        ensureJsonRPCStringHasId(jsonRPCString);

        RPCResponse response = null;
        response = new RPCResponse(JSONRPC2Response.parse(jsonRPCString));

        if(response.getError() != null) {
            /* TODO error code integer overflow see also unit test rpcErrorShouldHaveCodeAndMessage method  */
            ensureResponseHasValidErrorMember(response, jsonRPCString);
        }

        return response;
    }

    private static void ensureResponseHasValidErrorMember(RPCResponse response, String jsonRPCString)
            throws JSONRPC2ParseException {
        JSONRPC2Error error = response.getError();
        if( error == null) throw new IllegalArgumentException("Argument is null");

        if(!isObjectInteger(error.getCode())) {
            throw new JSONRPC2ParseException(
                    "Invalid jsonRPC error object. Code is missing or it is wrong type.",
                    JSONRPC2ParseException.PROTOCOL,
                    jsonRPCString
            );
        }

        if (error.getMessage() == null) {
            throw new JSONRPC2ParseException(
                    "Invalid jsonRPC error object. Message is missing.",
                    JSONRPC2ParseException.PROTOCOL,
                    jsonRPCString
            );
        }
    }

    private static boolean isObjectInteger(Object o) {
        return o instanceof Integer;
    }

    public static void ensureJsonRPCStringHasId(String jsonRPCString) throws JSONRPC2ParseException {
        if( !jsonRPCString.contains("\"id\"") ) {
            throw new JSONRPC2ParseException("id member is REQUIRED.",JSONRPC2ParseException.PROTOCOL, jsonRPCString);
        }

        ObjectMapper mapper = new ObjectMapper(); // can reuse, share globally
        Map<String,Object> jsonData;
        try {
            jsonData = mapper.readValue(jsonRPCString, Map.class);
        } catch (IOException e) {
            throw new JSONRPC2ParseException("Unable to parse json-rpc string",JSONRPC2ParseException.JSON, jsonRPCString);
        }

        if(jsonData == null) {
            throw new JSONRPC2ParseException("Unable to parse json-rpc string",JSONRPC2ParseException.JSON, jsonRPCString);
        }
        if(!jsonData.containsKey("id")) {
            throw new JSONRPC2ParseException("id member is REQUIRED.",JSONRPC2ParseException.PROTOCOL, jsonRPCString);
        }
    }
}
