package ee.ttu.deko.android.coffee.ui.tasks;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;

import ee.ttu.deko.android.coffee.ui.BluetoothProductsActivity;
import ee.ttu.deko.android.coffee.ui.ProductResource;
import ee.ttu.deko.android.coffee.ui.ProductsActivity;
import ee.ttu.deko.android.coffee.ui.ProductsActivitySupport;
import ee.ttu.deko.android.coffee.R;

import ee.ttu.deko.coffee.service.domain.Product;

public class InitCoffeeMachineTask extends AsyncTask<Void, String, Map<String, Object>> {
	protected final Logger logger = LoggerFactory.getLogger(InitCoffeeMachineTask.class.getCanonicalName());
	
	private ProductsActivitySupport activity;

	private ProgressDialog progressDialog;
	

	public InitCoffeeMachineTask(ProductsActivitySupport activity) {
		this.activity = activity;
	}

	@Override
	protected Map<String, Object> doInBackground(Void... params) {
		Thread.currentThread().setName(InitCoffeeMachineTask.class.getSimpleName());
		
		if(activity.remoteService != null) {			
			try{
				Map<String, Object> info = activity.remoteService.getInfo();				
				if(info == null) {
					return null;
				} else {
					publishProgress("Received coffee machine information\nRetrieving list of products...");
				}
				
				List<Product> products = activity.remoteService.getProducts();
				
				if(info != null && products != null) {
					info.put("products", products);
					publishProgress("Received list of products");
				}
				
				return info;
			} catch(IllegalStateException ise) {
				logger.error("CoffeMachine init failed", ise);
				return null;
			}			
		} else {
			return null;
		}		
	}

	@Override
	protected void onPostExecute(Map<String, Object> info) {
		StringBuilder sb = new StringBuilder();
		if(info != null) {
			sb.append("Connected to coffee machine\n");
			
			String name = (String) info.get("machineName");
			if(name != null && !name.isEmpty()) {
				sb.append("Machine name: ")
				.append(name)
				.append("\n");
			}
			
			String version = (String) info.get("machineFirmvareVersion");
			if(version != null && !version.isEmpty()) {
				sb.append("Machine firmware version: ")
				.append(version)
				.append("\n");
			}
			
			
					
			
			appendProductsToListView(info);
			
			((ProductsActivity) activity).getContainer().setVisibility(View.VISIBLE);
			
		} else {
			String infoText = "Unable to connect to coffee machine.\nPlease reconnect.";
			sb.append(infoText);			
			activity.remoteService.stop();
			activity.remoteService.disconnect();
			
			final String connectDialogMessage = infoText;
			activity.runOnUiThread(new Runnable() {				
				@Override
				public void run() {
					AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(activity);                      
				    dlgAlert.setMessage(connectDialogMessage);
				    dlgAlert.setTitle("Unable to connect");
//				    dlgAlert.setIcon();
				    dlgAlert.setPositiveButton("OK", null);
				    dlgAlert.setCancelable(true);
				    dlgAlert.create().show();					
				}
			});
			
			((BluetoothProductsActivity) activity).getBluetoothConnectionService().stop();
		}
		
		activity.setInfo(sb);
		
		
//		executeConnectionWatchDog();
		
		
		progressDialog.dismiss();
	}

	private void appendProductsToListView(Map<String, Object> info) {
		if(info != null) {
			List objectList = (List) info.get("products");
			if(objectList == null) {
				logger.warn("Unable to retrieve list of products, product list is null");
				return;
			}
			
			Iterator products = objectList.iterator();
			
			if(products != null) {
				((ProductsActivity) activity).getContainer().removeAllViews();
				while(products.hasNext()) {
					Product product = (Product) products.next();
					for(ProductResource res: ProductResource.all()) {
						if(res.getCode() == product.getId()) {	
							res.setName(product.getName());
							res.setPrice(product.getPrice());
							addProductToListView(res);
						}
					}
				}
			}
		}		
	}

	private void addProductToListView(ProductResource resource) {
		View productView = activity.createProductView(resource, ((ProductsActivity) activity).getContainer());
		((ProductsActivity) activity).getContainer().addView(productView);
	}

//	private void executeConnectionWatchDog() {		
//		if(activity instanceof BluetoothProductsActivity) {
//			
//			CoffeeMachineViaBlueTooth machine = ((BluetoothProductsActivity) activity).getCoffeMachine();
//			try {
//				if(machine != null) {
//					machine.sendMessage("EK:");
//			 		machine.cancelProduct();
//				}
//			} catch(IllegalStateException ise) {
//				logger.error("Product cancelling failed", ise);				
//			}
//			
//			((BluetoothProductsActivity) activity).startWatchDogForDelay(10*1000);
//		}
//	}

	@Override
	protected void onPreExecute() {
		progressDialog = new ProgressDialog(activity);
		progressDialog.setCancelable(false);
		progressDialog.setTitle(R.string.init_progress_dialog_title);
		progressDialog.setMessage(activity.getResources().getText(
				R.string.init_progress_dialog_text));
		progressDialog.show();
	
		super.onPreExecute();
	}
	
	@Override
	protected void onProgressUpdate(String... values) {	
		logger.info("Progress update: {}", values[0]);
		
		progressDialog.setMessage(values[0]);
		
		super.onProgressUpdate(values);
	}
	
	
	public void cancelTask() {
		progressDialog.dismiss();
		super.cancel(true);		
	}
};
