package ee.ttu.deko.coffee.service.domain;

import java.util.Map;

public class ServiceContract {
    public String id;
    public String description;
    public String version;
    public Map<String, Object> operations;
    public Map<String, Object> definitions;
}
