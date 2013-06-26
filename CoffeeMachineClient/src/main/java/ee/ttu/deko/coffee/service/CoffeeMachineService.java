package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;

import java.io.Reader;
import java.io.Writer;
import java.util.List;

public interface CoffeeMachineService extends Service {
    ServiceContract getServiceContract();

    // coffee machine methods
    Object getInfo();
    List<Product>  getProducts();
    void orderProduct(Product product);
    void cancelProduct(Product product);
}
