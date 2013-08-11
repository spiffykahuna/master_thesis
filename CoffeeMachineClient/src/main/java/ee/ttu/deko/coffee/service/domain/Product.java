package ee.ttu.deko.coffee.service.domain;

public class Product {
    private int id;
    private String name;
    private Price price;
    private byte[] image;




    public enum Status {
        PRODUCT_STATUS_MISSING,
        PRODUCT_STATUS_READY_TO_START,
        PRODUCT_STATUS_STARTED,
        PRODUCT_STATUS_IN_PROGRESS,
        PRODUCT_STATUS_READY,
        PRODUCT_STATUS_STOPPED,
        PRODUCT_STATUS_CANCELLED,
        PRODUCT_STATUS_FAILED,
        PRODUCT_STATUS_BUSY; // if another request comes while first is served
    }
    public Product(int id, String name, Price price) {
        this.id = checkedId(id);
        this.name = checkedName(name);
        this.price = checkedPrice(price);
    }

    private Price checkedPrice(Price price) {
        if(price == null) throw new IllegalArgumentException("price is null");
        return price;
    }

    private String checkedName(String name) {
        if(name == null) throw new IllegalArgumentException("name is null");
        if(name.isEmpty()) throw new IllegalArgumentException("name is empty");
        return name;
    }

    private int checkedId(int id) {
        if(id < 0) throw new IllegalArgumentException("Negative id is not allowed");
        return id;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public Price getPrice() {
        return price;
    }

    public void setPrice(Price price) {
        this.price = checkedPrice(price);
    }


    public void setName(String name) {
        this.name = checkedName(name);
    }

    public void setId(int id) {
        this.id = checkedId(id);
    }


    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + id;
        result = prime * result + name.hashCode();
        result = prime * result + price.hashCode();
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

        Product other = (Product) obj;
        if (this.id != other.getId())
            return false;
        if (!this.name.equals(other.getName()))
            return false;
        if (!this.price.equals(other.getPrice()))
            return false;

        return true;
    }
}
