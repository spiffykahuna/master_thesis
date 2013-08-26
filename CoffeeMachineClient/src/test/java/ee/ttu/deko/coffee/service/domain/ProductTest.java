package ee.ttu.deko.coffee.service.domain;

import org.junit.Test;

import java.math.BigDecimal;
import java.util.Currency;

import static org.junit.Assert.fail;

public class ProductTest {
    Product product;
    Price price;

    @Test
    public void nullConstructorArgs() throws Exception {
        try {
            product = new Product( -1 , null, null);
            fail("Nulls or negative id are not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            product = new Product( -1 , "cappuchino", null);
            fail("Nulls or negative id are not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( -1 , null, price );
            fail("Nulls or negative id are not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( 0 , "cappuchino", price );
            product.setPrice(null);
            fail("Null price is not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( 0 , "cappuchino", price );
            product.setName(null);
            fail("Null name is not allowed");
        } catch(IllegalArgumentException iae){}
    }

    @Test
    public void emptyNameIsNotAllowed() throws Exception {
        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( 0 , "", price );
            fail("Empty name is not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( 0 , "cappuchino", price );
            product.setName("");
            fail("Empty name is not allowed");
        } catch(IllegalArgumentException iae){}

    }

    @Test
    public void negativeIdIsNotAllowed() throws Exception {
        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( -1 , "cappuchino", price );
            fail("negetive id is not allowed");
        } catch(IllegalArgumentException iae){}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            product = new Product( 0, "cappuchino", price );
            product.setId(-1);
            fail("negetive id is not allowed");
        } catch(IllegalArgumentException iae){}

    }
}
