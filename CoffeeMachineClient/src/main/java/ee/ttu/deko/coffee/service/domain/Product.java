package ee.ttu.deko.coffee.service.domain;

import java.math.BigDecimal;

public class Product {
    private int id;
    private String name;
    private Price price;
    private byte[] image;

    public enum Status {
        MISSING,
        READY_TO_START,
        STARTED,
        IN_PROGRESS,
        READY,
        STOPPED,
        BUSY, // if another request comes while first is served
    }
}
