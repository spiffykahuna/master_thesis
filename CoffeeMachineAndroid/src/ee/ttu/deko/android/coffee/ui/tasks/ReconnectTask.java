package ee.ttu.deko.android.coffee.ui.tasks;

import java.util.concurrent.TimeUnit;


import ee.ttu.deko.android.coffee.ui.BluetoothProductsActivity;
import ee.ttu.deko.android.coffee.ui.ProductResource;
import ee.ttu.deko.android.coffee.ui.ProductsActivitySupport;
import android.os.AsyncTask;

public class ReconnectTask extends AsyncTask<Void, Integer, Boolean> {
	
	public static final int HIDE_BUTTONS = 0;
	public static final int SHOW_APP_VERSION = 1;
	public static final int CANCEL_PROD_DIALOG = 2;
	
	private BluetoothProductsActivity activity;
	
	public ReconnectTask(BluetoothProductsActivity activity) {
		this.activity = activity;		
	}
	
	
	
	@Override
	protected void onPreExecute() {
		if(activity != null) {
			activity.hideButtons();
			activity.showAppVersion();
			activity.cancelPrepareProductDialog();
		}
	}
	
	@Override
	protected Boolean doInBackground(Void... arg0) {
//		Thread.currentThread().setName(ReconnectTask.class.getSimpleName());
////		CoffeeMachineViaBlueTooth machine = null;
//		if(activity != null) {
//			activity.disconnectPairedDevice();
//			sleepSeconds(1);
////			machine = activity.getCoffeMachine();
//			while(machine != null && 
//					!machine.isBluetoothConnected() &&
//					!isCancelled()) {
//				publishProgress(HIDE_BUTTONS);
//				publishProgress(SHOW_APP_VERSION);
//				publishProgress(CANCEL_PROD_DIALOG);
//				activity.disconnectPairedDevice();
//				sleepSeconds(1);
//				activity.connectPairedDevice();						
//				sleepSeconds(3);
//			}	
//		}
//		
//		if(machine != null) {
//			return machine.isBluetoothConnected();
//		} else {
//			return false;
//		}	
		return false;
	}
	
	@Override
	protected void onProgressUpdate(Integer... messages) {
		final int message = messages[0];
		if(activity != null) {
			switch(message) {
			case HIDE_BUTTONS:
				activity.hideButtons();
				break;
			case SHOW_APP_VERSION:
				activity.showAppVersion();
				break;
				
			case CANCEL_PROD_DIALOG:
				activity.cancelPrepareProductDialog();
				break;
			default:
				break;
			}
		}
		
	}

	@Override
	protected void onPostExecute(Boolean blueToothIsConnected) {
		if(blueToothIsConnected && activity != null) {
			activity.setReconnecting(false);			
		}
	}
	
	private void sleepSeconds(int delay) {
		try {
			TimeUnit.SECONDS.sleep(delay);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
