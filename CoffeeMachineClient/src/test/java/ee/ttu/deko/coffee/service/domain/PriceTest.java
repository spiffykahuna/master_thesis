package ee.ttu.deko.coffee.service.domain;

import org.junit.Test;

import java.math.BigDecimal;
import java.util.Currency;


import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class PriceTest {
    public Price price;

    @Test
    public void constructorNullTest() throws Exception {
        try {
            price = new Price(null, null);
            fail("nulls are not accepted");
        } catch(IllegalArgumentException iae) {}


        try {
            price = new Price(null, Currency.getInstance("EUR"));
            fail("nulls are not accepted");
        } catch(IllegalArgumentException iae) {}

        try {
            price = new Price(BigDecimal.ONE,  null);
            fail("nulls are not accepted");
        } catch(IllegalArgumentException iae) {}


        price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
        assertNotNull(price);
        assertEquals(BigDecimal.ONE, price.getAmount());
        assertEquals(Currency.getInstance("EUR"), price.getCurrency());
    }

    @Test
    public void negativeAndNullAmountIsNotAllowed() throws Exception {
        try {
            price = new Price(new BigDecimal("-2.5"), Currency.getInstance("EUR"));
            fail("negative values are not accepted");
        } catch(IllegalArgumentException iae) {}

        try {
            price = new Price(null, Currency.getInstance("EUR"));
            fail("null values are not accepted");
        } catch(IllegalArgumentException iae) {}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            price.setAmount(new BigDecimal("-0.99"));
            fail("negative values are not accepted");
        } catch(IllegalArgumentException iae) {}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            price.setAmount(null);
            fail("null values are not accepted");
        } catch(IllegalArgumentException iae) {}
    }

    @Test
    public void nullCurrencyIsNotAllowed() throws Exception {
        try {
            price = new Price(new BigDecimal("2.5"), null);
            fail("null values are not accepted");
        } catch(IllegalArgumentException iae) {}

        try {
            price = new Price(BigDecimal.ONE, Currency.getInstance("EUR"));
            price.setCurrency(null);
            fail("null values are not accepted");
        } catch(IllegalArgumentException iae) {}
    }
}
