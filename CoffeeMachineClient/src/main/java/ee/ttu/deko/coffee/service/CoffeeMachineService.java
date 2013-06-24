package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;

import java.io.Reader;
import java.io.Writer;
import java.util.List;

public interface CoffeeMachineService {
    ServiceContract getServiceContract();

    // coffee machine methods
    Object getInfo();
    List<Product>  getProducts();
    void orderProduct(Product product);
    void cancelProduct(Product product);


    // connect methods
    void connect(Reader inputReader, Writer outputWriter);
    boolean isConnected();
    void disconnect();

    void addListener(ServiceListener listener);
    void removeListener(ServiceListener listener);
}
