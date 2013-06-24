package ee.ttu.deko.coffee.service;

import ee.ttu.deko.coffee.service.domain.Product;

import java.util.List;

public interface CoffeeMachine {
    Object getInfo();
    List<Product> getProducts();
    void orderProduct(Product product);
}
