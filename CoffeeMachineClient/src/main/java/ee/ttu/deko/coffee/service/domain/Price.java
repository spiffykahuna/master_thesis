package ee.ttu.deko.coffee.service.domain;

import java.math.BigDecimal;
import java.util.Currency;

public class Price {
    private BigDecimal amount;
    private Currency currency;

    public Price(BigDecimal amount, Currency currency) {
        this.amount = checkAmount(amount);
        this.currency = checkedCurrency(currency);
    }

    private Currency checkedCurrency(Currency currency) {
        if(currency == null)
            throw new IllegalArgumentException("Unable to create new price object. Provided currency is wrong");

        return currency;
    }

    private BigDecimal checkAmount(BigDecimal amount) {
        if(amount == null || (amount.compareTo(BigDecimal.ZERO) < 0)) {
            throw new IllegalArgumentException(
                    String.format("Provided amount is wrong: %s",
                            amount == null ? "null" : amount.toString()
                    )
            );
        }
        return amount;
    }

    public BigDecimal getAmount() {
        return amount;
    }

    public Currency getCurrency() {
        return currency;
    }

    public void setAmount(BigDecimal amount) {
        checkAmount(amount);
        this.amount = amount;
    }

    public void setCurrency(Currency currency) {
        this.currency = checkedCurrency(currency);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + amount.hashCode();
        result = prime * result + currency.hashCode();
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;

        Price other = (Price) obj;
        if (!this.amount.equals(other.getAmount()))
            return false;
        if (!this.currency.equals(other.getCurrency()))
            return false;

        return true;
    }
}
