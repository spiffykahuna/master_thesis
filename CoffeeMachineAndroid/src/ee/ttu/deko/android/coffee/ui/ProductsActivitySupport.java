package ee.ttu.deko.android.coffee.ui;

import java.io.File;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Message;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;


import ee.ttu.deko.android.coffee.R;

import ee.ttu.deko.android.coffee.ui.tasks.InitCoffeeMachineTask;
import ee.ttu.deko.android.coffee.ui.tasks.PrepareProductTask;
import ee.ttu.deko.android.coffee.ui.tasks.ReconnectTask;
import ee.ttu.deko.android.coffee.ui.widgets.ProductProgressDialog;
import ee.ttu.deko.coffee.service.CoffeeMachineService;
import ee.ttu.deko.coffee.service.JsonRpcCoffeeMachineService;


public class ProductsActivitySupport extends Activity {
		protected UsbManager usbManager;
	protected AlertDialog alertDialog;
	public PrepareProductTask prepareProductTask;
	protected InitCoffeeMachineTask initTask;
	protected ReconnectTask reconnectTask;
	public ProductProgressDialog productProgressDialog;
	
	
	public CoffeeMachineService remoteService = new JsonRpcCoffeeMachineService();
	
	public static final ExecutorService taskExecutor = Executors.newFixedThreadPool(
			Runtime.getRuntime().availableProcessors());

	private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
				UsbAccessory accessory = (UsbAccessory) intent
						.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
				if (accessory != null) {					
					System.exit(0);
				}
			}
		}
	};

	public void showAppVersion() {
		StringBuilder sb = new StringBuilder();

		try {
			sb.append(getString(R.string.app_name));
			PackageInfo pi = getPackageManager().getPackageInfo(
					getPackageName(), 0);
			sb.append(" " + pi.versionName + " (" + pi.versionCode + ")");
		} catch (NameNotFoundException e) {
			throw new RuntimeException(e);
		}

		setTitle(sb.toString());
	}

	protected AsyncTask<Void, String, Map<String, Object>> performHandshake() {
		if(initTask != null) {
			initTask.cancelTask();
			initTask = null;
		}
		
		initTask = new InitCoffeeMachineTask(this);
		return initTask.execute((Void) null);		
	}
	

	protected void initAlertDialog() {
		alertDialog = new AlertDialog.Builder(this).create();
		alertDialog.setTitle(R.string.confirm_dialog_title);
		alertDialog.setMessage(getResources().getString(
				R.string.confirm_dialog_text));

		alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE, getResources()
				.getString(R.string.confirm_dialog_no),
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						alertDialog.dismiss();
					}
				});
//		alertDialog.setButton(DialogInterface.BUTTON_NEUTRAL, getResources()
//				.getString(R.string.confirm_dialog_maybe),
//				new DialogInterface.OnClickListener() {
//					public void onClick(DialogInterface dialog, int which) {
//						alertDialog.dismiss();
//					}
//				});
	}

	public View createProductView(ProductResource p, ViewGroup container) {
		View productView = getLayoutInflater().inflate(R.layout.item,
				container, false);
		Button productButton = (Button) productView.findViewById(R.id.title);
		
		StringBuilder sb = new StringBuilder();
		sb.append(p.getName(productView.getContext()));
		sb.append("\n");
		sb.append(p.getPriceText());		
		
		productButton.setText(sb);
		
	
		productButton.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, p.getImageResource());
		productButton.setBackgroundColor(Color.WHITE);

		initProductButton(p, productButton, this);
		return productView;
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		showAppVersion();
		initAlertDialog();
		
	}

	protected void initTutorialButton(final BluetoothProductsActivity activity) {	
		ImageButton btn = (ImageButton) activity.findViewById(R.id.sourceButton);

		btn.setClickable(true);		
		btn.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {	
				final Intent intent = new Intent(Intent.ACTION_VIEW).setData(Uri.parse("https://github.com/spiffykahuna/master_thesis/"));
				activity.startActivity(intent);
			}
		});
		
		btn = (ImageButton) activity.findViewById(R.id.thesisTextButton);
		btn.setClickable(true);		
		btn.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {	
				Intent intent = new Intent();
				intent.setAction(android.content.Intent.ACTION_VIEW);
				File file = new File("/sdcard/Download/master_thesis_report.pdf");
				intent.setDataAndType(Uri.fromFile(file), "application/pdf");
				activity.startActivity(intent);
			}
		});
		
		
		

//		int screenWidth = getWindow().getAttributes().width;
//		int screenHeight = getWindow().getAttributes().height;
//		
//		btn.setLeft(screenWidth/2);
////		btn.setAdjustViewBounds(true);
//		btn.getBackground().setBounds(0, 0, 100, 100);
//		
//		btn.setMaxWidth(screenWidth/5);
		
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		//unregisterReceiver(mUsbReceiver);		
		
	}

	protected void connectToCoffeeMachine() {
		usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

		IntentFilter filter = new IntentFilter(
				UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		registerReceiver(mUsbReceiver, filter);

		UsbAccessory accessory = (UsbAccessory) getIntent().getParcelableExtra(
				UsbManager.EXTRA_ACCESSORY);
	}

	protected void initProductButton(final ProductResource p, Button productButton,
			final Context context) {
		final ProductsActivitySupport activity = this;

		productButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (alertDialog.isShowing()) {
					return;
				}
				
				if( isServiceReady()) {

					Button productButton = (Button) v.findViewById(R.id.title);
	
					alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
							getResources().getString(R.string.confirm_dialog_yes),
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int which) {
									alertDialog.dismiss();
									
									prepareProductTask = new PrepareProductTask(
											activity, p);
									prepareProductTask.execute();	
								}
							});
							
					alertDialog.setMessage(getResources().getString(
								R.string.confirm_dialog_formatted_text, 
								p.getName(context),
								p.getPriceText()
							)
					);
					
					alertDialog.setIcon(getProductIcon(productButton));
					alertDialog.show();
				} else {
					showToast(getString(R.string.not_connected));
				}
			}

			private boolean isServiceReady() {				
				return remoteService != null && 
						remoteService.isConnected() &&
						remoteService.isRunning();
			}

			private Drawable getProductIcon(Button productButton) {
				return productButton.getCompoundDrawables()[3];
			}
		});
	}


	
	public void cancelPrepareProductDialog() {
		if (prepareProductTask != null) {
			prepareProductTask.cancel(true);
			prepareProductTask = null;
		}
		if(productProgressDialog != null) {
			productProgressDialog.dismissManually();
		}
		
	}


	private void showToast(String message) {		
		Toast.makeText(getApplicationContext(), message,
                Toast.LENGTH_SHORT).show();
	}

	public void setInfo(CharSequence text) {
		View textView = getWindow().getDecorView().findViewById(R.id.status_text);
		
		if(textView != null) {
			TextView textBox = (TextView) textView;
			textBox.setText(text);
		}				
	}
	
	public String getInfo() {
		String text = "";
		View textView = getWindow().getDecorView().findViewById(R.id.status_text);
		
		if(textView != null) {
			TextView textBox = (TextView) textView;
			text = textBox.getText().toString();
		}	
		
		return text;
	}

	

}
