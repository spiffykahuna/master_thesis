package ee.ttu.deko.coffee.jsonrpc;

import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import com.thetransactioncompany.jsonrpc2.JSONRPC2Request;

import java.io.IOException;
import java.util.List;
import java.util.Map;

public class RPCRequest extends JSONRPC2Request {
    public RPCRequest(String method, Object id) {
        super(method, id);
    }

    public RPCRequest(String method, List<Object> positionalParams, Object id) {
        super(method, positionalParams, id);
    }

    public RPCRequest(String method, Map<String, Object> namedParams, Object id) {
        super(method, namedParams, id);
    }

    public RPCRequest(JSONRPC2Request request) {
        super(request.getMethod(), request.getID());
        setNamedParams(request.getNamedParams());
        setPositionalParams(request.getPositionalParams());

        if(request.getNonStdAttributes() != null && request.getNonStdAttributes().size() > 0) {
            for(Map.Entry<String, Object> attribute: request.getNonStdAttributes().entrySet()) {
                appendNonStdAttribute(attribute.getKey(), attribute.getValue());
            }
        }
    }

    public String toJsonString(boolean prettyPrint) {
        String jsonString = "";
        try {
            jsonString = PojoMapper.toJson(toJSONObject(), prettyPrint);
        } catch (IOException e) {
           throw new RuntimeException(e);
        }
        return jsonString;
    }

//    public static RPCRequest fromJsonString(String jsonString) {
//
//    }

    @Override
    public String toString() {
        return toJsonString(false);
    }


    public static RPCRequest parse(final String jsonString)
            throws JSONRPC2ParseException {

        return new RPCRequest(parse(jsonString, false, false, false));
    }

    @Override
    public boolean equals(Object obj) {
        if(!(obj instanceof RPCRequest))  return false;
        RPCRequest that = (RPCRequest) obj;

        if(this.getMethod() != null) {
            String method = that.getMethod();
            if(!this.getMethod().equals(method)) return false;
        } else {
            if(that.getMethod() != null) return false;
        }

        if(this.getPositionalParams() != null) {
            List<Object> params = that.getPositionalParams();
            if(!this.getPositionalParams().equals(params)) return false;
        } else {
            if(that.getPositionalParams() != null) return false;
        }

        if(this.getNamedParams() != null) {
            Map<String, Object> thatParams = that.getNamedParams();
            Map<String, Object> thisParams = this.getNamedParams();
            if(!thisParams.equals(thatParams)) return false;
        } else {
            if(that.getNamedParams() != null) return false;
        }

        if(this.getID() != null) {
            Object id = that.getID();
            if(!this.getID().equals(id)) return false;
        } else {
            if(that.getID() != null) return false;
        }

        if(this.getNonStdAttributes() != null) {
            Map<String, Object> attrs = that.getNonStdAttributes();
            if(!this.getNonStdAttributes().equals(attrs)) return false;
        } else {
            if(that.getNonStdAttributes() != null) return false;
        }

        return true;
    }
}
