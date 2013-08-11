package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.MachineConfig;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;

import java.util.List;
import java.util.Map;

public interface CoffeeMachineService extends Service {
    ServiceContract getServiceContract();

    // coffee machine methods
    Map<String, Object> getInfo(); // service and machine information and limits
    Object getServiceInfo();

    // we need to configure machine somehow
    void configCoffeeMachine(MachineConfig config);
    MachineConfig getCoffeeMachineConfig();

    // products
    List<Product>  getProducts();
    Product.Status orderProduct(int productId);
    Product.Status cancelProduct(int productId);
    Product.Status getProductStatus(int productId);

    void login(String password);
}
