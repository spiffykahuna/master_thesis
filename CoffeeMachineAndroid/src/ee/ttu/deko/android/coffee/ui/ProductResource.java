package ee.ttu.deko.android.coffee.ui;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;

import ee.ttu.deko.android.coffee.R;
import ee.ttu.deko.coffee.service.domain.Price;



public class ProductResource {
	private static final ProductResource[] ALL = new ProductResource[] {
			new ProductResource(R.drawable.cappuccino_1, R.string.cappuccino_1, 0x04),
			new ProductResource(R.drawable.cappuccino_2, R.string.cappuccino_2, 0x14),
			new ProductResource(R.drawable.coffee_1, R.string.coffee_1, 0x03),
			new ProductResource(R.drawable.coffee_2, R.string.coffee_2, 0x13),
			new ProductResource(R.drawable.espresso_1, R.string.espresso_1, 0x02),
			new ProductResource(R.drawable.espresso_2, R.string.espresso_2, 0x12),
			new ProductResource(R.drawable.espresso_macchiato_1,
					R.string.espresso_macchiato_1, 0x06),
			new ProductResource(R.drawable.espresso_macchiato_2,
					R.string.espresso_macchiato_2, 0x16),
			new ProductResource(R.drawable.hotwater_1, R.string.hotwater_1, 0x0D),
			new ProductResource(R.drawable.latte_macchiato_1,
					R.string.latte_macchiato_1, 0x07),
			new ProductResource(R.drawable.latte_macchiato_2,
					R.string.latte_macchiato_2, 0x17),
			new ProductResource(R.drawable.milk_1, R.string.milk_1, 0x0A),
			new ProductResource(R.drawable.milk_2, R.string.milk_2, 0x1A),
			new ProductResource(R.drawable.milk_foam_1, R.string.milk_foam_1, 0x08),
			new ProductResource(R.drawable.milk_foam_2, R.string.milk_foam_2, 0x18),
			new ProductResource(R.drawable.milkcoffee_1, R.string.milkcoffee_1, 0x05),
			new ProductResource(R.drawable.milkcoffee_2, R.string.milkcoffee_2, 0x15),
			new ProductResource(R.drawable.pot_1, R.string.pot_1, 0x0C),
			new ProductResource(R.drawable.ristretto_1, R.string.ristretto_1, 0x01),
			new ProductResource(R.drawable.ristretto_2, R.string.ristretto_2, 0x11) };

	private static final ProductResource[] COMMON = new ProductResource[] { ALL[0], ALL[1],
			ALL[2], ALL[3], ALL[4], ALL[5] };

	private int imageResource;
	private int nameResource;
	private String name;
	private Price price;
	private int code;

	public ProductResource(int imageResource, int nameResource, int code) {
		this.imageResource = imageResource;
		this.nameResource = nameResource;
		this.code = code;
	}

	public int getImageResource() {
		return imageResource;
	}

	public int getNameResource() {
		return nameResource;
	}

	public int getCode() {
		return code;
	}

	public static ProductResource[] all() {
		return ALL;
	}

	public static ProductResource[] common() {
		return COMMON;
	}

	public static ProductResource[] uncommon() {
		List<ProductResource> list = new ArrayList<ProductResource>();

		for (ProductResource p : ALL) {
			if (p.isUncommon()) {
				list.add(p);
			}
		}

		return list.toArray(new ProductResource[0]);
	}

	private boolean isUncommon() {
		for (ProductResource p : COMMON) {
			if (p.equals(this)) {
				return false;
			}
		}

		return true;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + code;
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
		ProductResource other = (ProductResource) obj;
		if (code != other.code)
			return false;
		return true;
	}
	
	public void setName(String newProductName) {
		name = newProductName;
	}

	public CharSequence getName(Context context) {		
		return (name != null && !name.isEmpty()) ? name : context.getText(nameResource);
	}

	public Price getPrice() {	
		return price;
	}

	public void setPrice(Price newProductPrice) {		
		price = newProductPrice;
	}

	public String getPriceText() {
		StringBuilder sb = new StringBuilder();
		if(price != null) {
			sb.append(price.getAmount().toString());
			sb.append(" ");
			sb.append(price.getCurrency().toString());
		}
		return sb.toString();
	}

	
}
