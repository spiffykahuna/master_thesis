package ee.ttu.deko.android.coffee.ui.tasks;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.view.View;
import ee.ttu.deko.android.coffee.ui.ProductResource;
import ee.ttu.deko.android.coffee.ui.ProductsActivitySupport;
import ee.ttu.deko.android.coffee.R;
import ee.ttu.deko.android.coffee.ui.widgets.ProductProgressDialog;
import ee.ttu.deko.coffee.service.domain.Product;

import static ee.ttu.deko.coffee.service.domain.Product.Status.*;

public class PrepareProductTask extends AsyncTask<Void, String, Product.Status> {
	protected final Logger logger = LoggerFactory.getLogger(PrepareProductTask.class.getCanonicalName());
	private static final int TIMEOUT = 1 * 60 * 1000;
	private static final int FAILURE_RETRIES = 10;

	private ProductsActivitySupport activity;

	private ProductResource product;
	
	private Boolean wasCancelled = false;
	private int failureCounter = FAILURE_RETRIES;

	public PrepareProductTask(ProductsActivitySupport activity, ProductResource p) {
		this.activity = activity;
		this.product = p;
	}

	@Override
	protected Product.Status doInBackground(Void... params) {	
		
		final CharSequence productName = product.getName(activity.getApplicationContext());
		String infoText;
		failureCounter = FAILURE_RETRIES;
		
		Product.Status status = activity.remoteService.orderProduct(product.getCode());
		
		if(PRODUCT_STATUS_STARTED.equals(status)) {			
			publishProgress("Product " +  productName + " started");			
		} else if(status == null) {
			synchronized (wasCancelled) {
				if(wasCancelled) {
					wasCancelled = false;
					infoText = "Product was cancelled";
					publishProgress(infoText);
					activity.runOnUiThread(new Runnable() {				
						@Override
						public void run() {					
							activity.setInfo("Product "  + productName + " was cancelled");						
						}
					});
				} else {
					infoText = "Unable to start product";
					publishProgress(infoText);
					activity.runOnUiThread(new Runnable() {				
						@Override
						public void run() {					
							activity.setInfo("Unable to start product: " + productName);						
						}
					});
				}
			
			}
			sleepSeconds(5);
			return null;
		} else {
			infoText = "Unable to start product";
			publishProgress(infoText);
			activity.runOnUiThread(new Runnable() {				
				@Override
				public void run() {					
					activity.setInfo("Unable to start product: " + productName);						
				}
			});
			return null;
		}
		
		status = activity.remoteService.getProductStatus(product.getCode());
		
		if(isCancelled()) return status;
		
		while(!PRODUCT_STATUS_READY.equals(status)) {
			if(isCancelled()) return status;
			sleepSeconds(1);
			if(isCancelled()) return status;
			
			publishProgress("Product in progress...");
			status = activity.remoteService.getProductStatus(product.getCode());
			
			if(PRODUCT_STATUS_MISSING.equals(status)) {
				if(failureCounter-- < 0) {
					failureCounter = FAILURE_RETRIES;
					status = PRODUCT_STATUS_FAILED;
				} else {
					continue;
				}				
			}
			
			if(PRODUCT_STATUS_FAILED.equals(status) ) {
				infoText = "Preparing product " + productName + " failed";
				logger.info(infoText);
				publishProgress(infoText);				
				setInfoOnMainActivity(infoText);
				
				final String failedDialogMessage = infoText;
				activity.runOnUiThread(new Runnable() {				
					@Override
					public void run() {
						AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(activity);                      
					    dlgAlert.setMessage(failedDialogMessage);
					    dlgAlert.setTitle("Product prepare failed");
					    dlgAlert.setIcon(product.getImageResource());
					    dlgAlert.setPositiveButton("OK", null);
					    dlgAlert.setCancelable(true);
					    dlgAlert.create().show();					
					}
				});
				
				return status;
			}
			
			
			if(isCancelled()) return status;
			
		}
		
		if(PRODUCT_STATUS_READY.equals(status)) {
			infoText = "Product " + productName + " is ready";
			logger.info(infoText);
			publishProgress(infoText);	
			
			sleepSeconds(3);
			
			setInfoOnMainActivity(infoText);
			
			final String readyDialogMessage = infoText;
			activity.runOnUiThread(new Runnable() {				
				@Override
				public void run() {
					AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(activity);                      
				    dlgAlert.setMessage(readyDialogMessage);
				    dlgAlert.setTitle("Product ready");
				    dlgAlert.setIcon(product.getImageResource());
				    dlgAlert.setPositiveButton("OK", null);
				    dlgAlert.setCancelable(true);
				    dlgAlert.create().show();					
				}
			});
		    
		    
		}
		
		return status;
	}

	private void setInfoOnMainActivity(final String infoText) {
		activity.runOnUiThread(new Runnable() {				
			@Override
			public void run() {						
				activity.setInfo(infoText);						
			}
		});
	}
	
	private String getInfoOnMainActivity() {
		String text = "";
				
		text = activity.getInfo();						
		return text;		
	}

	private void sleepSeconds(int secDelay) {
		try {
			Thread.sleep(secDelay * 1000);
		} catch (InterruptedException e) {
			
		}
		
	}

	@Override
	protected void onPreExecute() {
		logger.debug("Starting preparation of product: code={} name={}",
				product.getCode(), product.getName(activity.getApplicationContext()));
		
		final String productTitle = activity.getResources().getString(
				product.getNameResource());
		String text = String.format(activity.getResources().getString(
				R.string.product_progress_dialog_text, productTitle));
		String cancelText = activity.getResources().getString(
				R.string.product_progress_dialog_cancel);

		activity.productProgressDialog = new ProductProgressDialog(activity);
		activity.productProgressDialog
				.setTitle(R.string.product_progress_dialog_title);
		activity.productProgressDialog.setMessage(text);

		activity.productProgressDialog.setButton(
				DialogInterface.BUTTON_NEGATIVE, cancelText,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						activity.runOnUiThread(new Runnable() {
							@Override
							public void run() {				
								activity.productProgressDialog.setMessage(
										activity.getText(R.string.product_progress_dialog_cancel_message_text)
								);
								setInfoOnMainActivity("Cancelling product: " + productTitle + " ...");
								
							}			
						});
						ProductsActivitySupport.taskExecutor.submit(new Runnable() {
								public void run() {
									String infoText = "";
									
									synchronized (activity.prepareProductTask) {
										activity.prepareProductTask.cancel(false);
									}
									
									synchronized (wasCancelled) {
										wasCancelled = true;
									}
									
									Product.Status status = activity.remoteService.cancelProduct(product.getCode());
									while(!PRODUCT_STATUS_CANCELLED.equals(status)) {
										status = activity.remoteService.cancelProduct(product.getCode());
										if(PRODUCT_STATUS_FAILED.equals(status) || (status == null)) {
											infoText = "Unable to cancel product: " + productTitle;
											logger.error(infoText);
											setInfoOnMainActivity(infoText);
											return;
										}
										if(activity.prepareProductTask.isCancelled()) return;
										sleepSeconds(2);
										if(activity.prepareProductTask.isCancelled()) return;
									}
									infoText = "Product " + productTitle + " cancelled";
									logger.info(infoText);
									setInfoOnMainActivity(infoText);
									
									if(PRODUCT_STATUS_CANCELLED.equals(status)) {
										synchronized (activity.prepareProductTask) {
											activity.prepareProductTask.cancel(true);
										}
									}
									
									final String cancelledDialogMessage = infoText;
									activity.runOnUiThread(new Runnable() {				
										@Override
										public void run() {
											AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(activity);                      
										    dlgAlert.setMessage(cancelledDialogMessage);
										    dlgAlert.setTitle("Product cancelled");
										    dlgAlert.setIcon(product.getImageResource());
										    dlgAlert.setPositiveButton("OK", null);
										    dlgAlert.setCancelable(true);
										    dlgAlert.create().show();					
										}
									});
								};
							}
						);
						
					}
				});

		activity.productProgressDialog.setCancelable(false);
		activity.productProgressDialog.setIndeterminate(true);
		activity.productProgressDialog.setIcon(product.getImageResource());
		activity.productProgressDialog.show();

		super.onPreExecute();
	}

	@Override
	protected void onPostExecute(Product.Status status) {
		activity.productProgressDialog.dismissManually();
	}

	@Override
	protected void onCancelled() {
		activity.productProgressDialog.dismissManually();
	}
	
	@Override
	protected void onProgressUpdate(String... values) {	
		logger.info("Progress update: {}", values[0]);
		
		activity.productProgressDialog.setMessage(values[0]);
		
		super.onProgressUpdate(values);
	}

}
