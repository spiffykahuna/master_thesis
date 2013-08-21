package ee.ttu.deko.android.coffee.ui;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import ee.ttu.deko.android.coffee.R;

public class ProductsActivity extends ProductsActivitySupport {
	private LinearLayout container;



	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		setContainer((LinearLayout) findViewById(R.id.container));


//		for (ProductResource p : ProductResource.all()) {
//			View productView = createProductView(p, getContainer());
//			getContainer().addView(productView);
//		}

		//connectToCoffeeMachine();
		//performHandshake();
	}

	public LinearLayout getContainer() {
		return container;
	}

	public void setContainer(LinearLayout container) {
		this.container = container;
	}
	
	public void hideButtons() {
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {				
				getContainer().setVisibility(View.INVISIBLE);							
			}			
		});
		
	}

	public void showButtons() {
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {				
				getContainer().setVisibility(View.VISIBLE);							
			}			
		});		
	}

}
