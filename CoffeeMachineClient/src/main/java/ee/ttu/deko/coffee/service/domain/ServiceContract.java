package ee.ttu.deko.coffee.service.domain;

import java.util.HashMap;
import java.util.Map;

public class ServiceContract extends HashMap<String, Object> {

    public ServiceContract(Object contract) {
        super();
        copyContractMap(contract);
    }


    public ServiceContract() {
        super();
    }

    private void copyContractMap(Object contract) {
        if(!(contract instanceof Map)) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument is not instanse of map");

        Map<String, Object> contractMap = new HashMap<String, Object>();
        try{
            contractMap = (Map<String, Object>) contract;
        } catch(ClassCastException cce) {
            throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument is not instanse of map");
        }

        if(!contractMap.containsKey("id")) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument does not have member with key \"id\"");
        if(!contractMap.containsKey("description")) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument does not have member with key \"description\"");
        if(!contractMap.containsKey("version")) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument does not have member with key \"version\"");
        if(!contractMap.containsKey("operations")) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument does not have member with key \"operations\"");
        if(!contractMap.containsKey("definitions")) throw new IllegalArgumentException("Unable to create service contract object. Provided constructor argument does not have member with key \"definitions\"");

        put("id", contractMap.get("id"));
        put("description", contractMap.get("description"));
        put("version", contractMap.get("version"));
        put("operations", contractMap.get("operations"));
        put("definitions", contractMap.get("definitions"));
    }

    public String getId() {
        return getStringField("id");
    }

    private String getStringField(String fieldName) {
        String field = null;
        try {
            field = (String) get(fieldName);
        } catch (ClassCastException cce) {
            throw new IllegalStateException(String.format("%s should be string.", fieldName));
        }

//        if(field == null) throw new IllegalStateException(String.format("%s should not be null", fieldName));
        return  field;
    }

    public void setId(String id) {
        put("id", id);
    }

    public String getDescription() {
        return getStringField("description");
    }

    public void setDescription(String description) {
        put("description", description);
    }

    public String getVersion() {
        return getStringField("version");
    }

    public void setVersion(String version) {
        put("version", version);
    }

    public Map<String, Object> getOperations() {
        return getMapField("operations");
    }

    private Map<String, Object> getMapField(String fieldName) {
        Map<String, Object> field = null;
        try {
            field = (Map<String, Object>) get(fieldName);
        } catch (ClassCastException cce) {
            throw new IllegalStateException(String.format("%s should be map with string keys.", fieldName));
        }

//        if(field == null) throw new IllegalStateException(String.format("%s should not be null", fieldName));
        return  field;
    }

    public void setOperations(Map<String, Object> operations) {
        put("operations", operations);
    }

    public Map<String, Object> getDefinitions() {
        return getMapField("definitions");
    }

    public void setDefinitions(Map<String, Object> definitions) {
        put("definitions", definitions);
    }
}
