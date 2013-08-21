package ee.ttu.deko.android.coffee.ui.widgets;

import android.app.ProgressDialog;
import ee.ttu.deko.android.coffee.ui.ProductsActivitySupport;

public class ProductProgressDialog extends ProgressDialog {
	private ProductsActivitySupport activity;

	public ProductProgressDialog(ProductsActivitySupport activity) {
		super(activity);
	}

	@Override
	public void dismiss() {
		// NOOP
	}

	public void dismissManually() {
		super.dismiss();
	}
}
