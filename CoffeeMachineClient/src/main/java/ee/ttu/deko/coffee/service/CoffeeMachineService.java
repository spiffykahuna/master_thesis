package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.MachineConfig;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;

import java.util.List;

public interface CoffeeMachineService extends Service {
    ServiceContract getServiceContract();

    // coffee machine methods
    Object getInfo(); // service and machine information and limits
    Object getServiceInfo();

    // we need to configure machine somehow
    void configCoffeeMachine(MachineConfig config);
    MachineConfig getCoffeeMachineConfig();

    // products
    List<Product>  getProducts();
    void orderProduct(Product product);
    void cancelProduct(Product product);
    Product.Status getProductStatus(Product product);

    void login(String password);
}
