package ee.ttu.deko.coffee.jsonrpc;

import java.math.BigDecimal;

public class JsonRpc2_0Spec {

    public static void checkId(Object id) {
        if (   id != null             &&
                ! (id instanceof Number ) &&
                ! (id instanceof String )     )
            throw new IllegalArgumentException("An identifier MUST contain a String, Number, or NULL value if included.");

        if( (id instanceof Number)
                && ((id instanceof Double) || (id instanceof BigDecimal))) {
            throw new IllegalArgumentException("An identifier SHOULD NOT contain fractional parts");
        }
    }
}
