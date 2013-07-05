package ee.ttu.deko.coffee.service.domain;

import ee.ttu.deko.coffee.service.domain.Product;

import java.util.List;

public interface CoffeeMachine {
    Object getInfo();
    List<Product> getProducts();
    void orderProduct(Product product);
    void cancelProduct(Product product);
    Product.Status getProductStatus(Product product);

}
