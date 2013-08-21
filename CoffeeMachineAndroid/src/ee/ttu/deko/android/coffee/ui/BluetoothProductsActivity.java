package ee.ttu.deko.android.coffee.ui;





import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.ActionBar;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.SharedPreferences;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

import ee.ttu.deko.android.coffee.DeviceListActivity;
import ee.ttu.deko.android.coffee.R;

import ee.ttu.deko.android.coffee.bluetooth.BluetoothAdapterListener;
import ee.ttu.deko.android.coffee.bluetooth.BluetoothConnectionService;
import ee.ttu.deko.android.coffee.bluetooth.BluetoothEventListener;
import ee.ttu.deko.android.coffee.bluetooth.BluetoothConnectionService.BluetoothConnectionListener;

import ee.ttu.deko.android.coffee.ui.tasks.ReconnectTask;
import ee.ttu.deko.coffee.service.CoffeeMachineService;
import ee.ttu.deko.coffee.service.JsonRpcCoffeeMachineService;
import ee.ttu.deko.coffee.service.request.RepeatedRPCRequest;
import ee.ttu.deko.coffee.service.request.RequestProcessor;





public class BluetoothProductsActivity extends ProductsActivity 
implements  BluetoothConnectionListener, BluetoothEventListener{
	

	private static final String DEBUG_TAG = BluetoothProductsActivity.class.getSimpleName();
    private static final boolean DEBUG = true;

    
    public static final int MESSAGE_STATE_CHANGE = 1;
    public static final int MESSAGE_READ = 2;
    public static final int MESSAGE_WRITE = 3;
    public static final int MESSAGE_DEVICE_NAME = 4;
    public static final int MESSAGE_TOAST = 5;
    public static final int MESSAGE_RECONNECT = 6;
    
    public static final String DEVICE_NAME = "device_name";
    public static final String TOAST = "toast";

    // Intent request codes
    private static final int REQUEST_CONNECT_DEVICE_SECURE = 1;
    private static final int REQUEST_CONNECT_DEVICE_INSECURE = 2;
    private static final int REQUEST_ENABLE_BT = 3;
    
    
//	private CoffeeMachineViaBlueTooth machine = new CoffeeMachineViaBlueTooth();
	
	
	
	private BluetoothAdapter mBluetoothAdapter = null;    
    private BluetoothConnectionService mConnectionService = null;
    
    protected AsyncTask<Void, Void, String> initTask;
    
 	
	private volatile boolean reconnecting = false;
    private WatchDog watchDog;
	private BluetoothDevice remoteBluetoothDevice; 
	
	
	


	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(DEBUG) Log.e(DEBUG_TAG, "+++ ON CREATE +++");
              
        
        
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if (mBluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        
        mConnectionService = new BluetoothConnectionService(this, mHandler, this);
        hideButtons();
        initTutorialButton(this);
        
        BluetoothAdapterListener.addEventListener(this);
    }

	@Override
    public void onStart() {
        super.onStart();
        if(DEBUG) Log.e(DEBUG_TAG, "++ ON START ++");
      
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);        
        } 
    }
	
	 @Override
    public synchronized void onResume() {
		super.onResume();
		if(DEBUG) Log.e(DEBUG_TAG, "+ ON RESUME +");
		
		// Performing this check in onResume() covers the case in which BT was
		// not enabled during onStart(), so we were paused to enable it...
		// onResume() will be called when ACTION_REQUEST_ENABLE activity returns.
//		if (mConnectionService != null && mBluetoothAdapter.isEnabled()) {
//		    // Only if the state is STATE_NONE, do we know that we haven't started already
//		    if (mConnectionService.getState() == BluetoothConnectionService.STATE_NONE) {              
//		      mConnectionService.start();
//		      connectPairedDevice();
//		    }
//		}	        
	 }
	
	public void connectPairedDevice() {		
		String mac = getPairedDeviceAddressFromConfig();
//		if(!mac.isEmpty()) {
//			Intent address = new Intent();
//			address.putExtra(DeviceListActivity.EXTRA_DEVICE_ADDRESS, mac);
//			connectDevice(address, false);
//		}		
	}
	
	public synchronized void disconnectPairedDevice() {	
		
		disconnectBluetoothFromMachine();
				
//		Thread disc = new Thread( new Runnable() {
//			@Override
//			public void run() {
//				
//				if (machine != null) {
//					try {
//						machine.cancelProduct();
//					} catch(IllegalStateException ise) {
//						if(DEBUG) Log.e(DEBUG_TAG, "Product cancelling failed", ise);				
//					}
//						
//					disconnectBluetoothFromMachine();
//		        }
//				mConnectionService.restart();
//			}			
//			
//		}, "Disconnect_device_thread");
//		
//		disc.start();
//		
//		try {
//			disc.join();
//		} catch (InterruptedException ignored) {			
//		}
//		
	}
	
	public synchronized AsyncTask<Void, Void, String> getInitTask() {
		return initTask;
	}


	@Override
    public synchronized void onPause() {
        super.onPause();
        if(DEBUG) Log.e(DEBUG_TAG, "- ON PAUSE -");
    }

    @Override
    public void onStop() {
        super.onStop();
        if(DEBUG) Log.e(DEBUG_TAG, "-- ON STOP --");
    }
    
	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		if(DEBUG) Log.e(DEBUG_TAG, "--- ON DESTROY ---");
		disconnectRemoteService();
		disconnectBluetoothFromMachine();
		stopConnectionWatchdog();
        if (mConnectionService != null) mConnectionService.stop();
        
	}

	public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(DEBUG) Log.d(DEBUG_TAG, "onActivityResult " + resultCode);
        switch (requestCode) {
        case REQUEST_CONNECT_DEVICE_SECURE:
            // When DeviceListActivity returns with a device to connect
            if (resultCode == Activity.RESULT_OK) {
            	
            	hideButtons();
            	setInfo("No connection");
            	disconnectRemoteService();
            	mConnectionService.restart();
            	
                connectDevice(data, true);
            }
            break;
        case REQUEST_CONNECT_DEVICE_INSECURE:
            // When DeviceListActivity returns with a device to connect
            if (resultCode == Activity.RESULT_OK) {
                connectDevice(data, false);
            }
            break;
        case REQUEST_ENABLE_BT:
            // When the request to enable Bluetooth returns
            if (resultCode == Activity.RESULT_OK) {
                showToast(R.string.bt_is_now_enabled);
                reconnectPairedDevice();
            } else {
                Log.d(DEBUG_TAG, "BT not enabled");
                Toast.makeText(this, R.string.bt_not_enabled_leaving, Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

    private void connectDevice(Intent data, boolean secure) {
        // Get the device MAC address
        String address = data.getExtras()
            .getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);
        // Get the BluetoothDevice object
        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        this.remoteBluetoothDevice = device;
        // Attempt to connect to the device
        mConnectionService.connect(device, secure);
    }
    
 // The Handler that gets information back from the connection service    
 	private final Handler mHandler = new Handler() {
         private String mConnectedDeviceName;

		@Override
         public void handleMessage(Message msg) {
             switch (msg.what) {
             case MESSAGE_STATE_CHANGE:
                 if(DEBUG) Log.i(DEBUG_TAG, "MESSAGE_STATE_CHANGE: " + msg.arg1);
                 switch (msg.arg1) {
                 case BluetoothConnectionService.STATE_CONNECTED:
                     setStatus(getString(R.string.title_connected_to, mConnectedDeviceName));
                     //mConversationArrayAdapter.clear();
                     break;
                 case BluetoothConnectionService.STATE_CONNECTING:
                     setStatus(R.string.title_connecting);
                     break;
                 case BluetoothConnectionService.STATE_LISTEN:
                 case BluetoothConnectionService.STATE_NONE:
                     setStatus(R.string.title_not_connected);
                     break;
                 }
                 break;
             case MESSAGE_WRITE:
                 byte[] writeBuf = (byte[]) msg.obj;
                 // construct a string from the buffer
                 String writeMessage = new String(writeBuf);
                 //mConversationArrayAdapter.add("Me:  " + writeMessage);
                 break;
             case MESSAGE_READ:
                 byte[] readBuf = (byte[]) msg.obj;
                 // construct a string from the valid bytes in the buffer
                 String readMessage = new String(readBuf, 0, msg.arg1);
                 //mConversationArrayAdapter.add(mConnectedDeviceName+":  " + readMessage);
                 break;
             case MESSAGE_DEVICE_NAME:
                 // save the connected device's name
                 mConnectedDeviceName = msg.getData().getString(DEVICE_NAME);
                 Toast.makeText(getApplicationContext(), "Connected to "
                                + mConnectedDeviceName, Toast.LENGTH_SHORT).show();
                 break;
             case MESSAGE_TOAST:
                 Toast.makeText(getApplicationContext(), msg.getData().getString(TOAST),
                                Toast.LENGTH_SHORT).show();
                 break;
             case MESSAGE_RECONNECT:
            	 setStatus(R.string.title_not_connected);
            	 reconnectPairedDevice();
            	 break;
             }
         }

		
     };
	 private int autoConnectFailureCounter = 10;
	
     private void requestBluetoothToTurnON() {
			Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
		    startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);			
     }
     
     private final void setStatus(int resId) {
         final ActionBar actionBar = getActionBar();
         actionBar.setSubtitle(resId);
     }

     private final void setStatus(CharSequence subTitle) {
         final ActionBar actionBar = getActionBar();
         actionBar.setSubtitle(subTitle);
     }
     
     
    
 
 	@Override
 	public void onBluetoothConnectionEstablished(BluetoothSocket socket,
 			BluetoothDevice device, String socketType) { 
 		
 		
// 		machine.connectToBluetooth(socket); 
 		
 		InputStream tmpIn = null;
        OutputStream tmpOut = null;		
        try {
            tmpIn = socket.getInputStream();
            tmpOut = socket.getOutputStream();
        } catch (IOException e) {
            if(DEBUG) Log.e(DEBUG_TAG, "Cannot connect Coffe machine to bluetooth socket", e);
        }
        
        Reader reader = new InputStreamReader(tmpIn, Charset.forName("UTF-8"));
        Writer writer = new OutputStreamWriter(tmpOut, Charset.forName("UTF-8"));
        
        
        remoteService.connect(reader, writer);
        
        
        RequestProcessor processor = remoteService.getRequestProcessorByType(RepeatedRPCRequest.class, 5000, 3);
        remoteService.setRequestProcessor(processor);
        
        remoteService.start();
        
        
        
//		isBluetoothConnected = true;
		
		
// 		machine.registerInputHandler(this); 
// 		coffeeMachine = machine;
 		runInitTask();
// 		addPairedDeviceToConfig(device.getAddress());
 		 		
 	} 
 	
 	public synchronized void startWatchDogForDelay(long delayInMilliseconds) {
 		stopConnectionWatchdog();
 		
 		watchDog = new WatchDog(delayInMilliseconds);
        watchDog.onDie(new Runnable() {
			@Override
			public void run() {
				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						Message msg = mHandler.obtainMessage(MESSAGE_RECONNECT);				
						mHandler.sendMessage(msg);	
						
					}					
				});		
			}
        });
        
        watchDog.start();
        reconnecting = false;
 	}

	private void stopConnectionWatchdog() {
		if(watchDog != null) {			
 			try {
 				watchDog.kill();
			} catch (InterruptedException e) {
				if(DEBUG) Log.e(DEBUG_TAG, "Watchdog stop failed", e);				
			}
 		}
	}
 	
 	private synchronized void reconnectPairedDevice() {
 		if(!reconnecting) {
 			reconnecting = true;
 			
 			if(reconnectTask != null) {
 				reconnectTask.cancel(true);
 				reconnectTask = null;
 			} 			
 			reconnectTask = new ReconnectTask(this);
 			reconnectTask.execute();
 		} 		
	}

	private void runInitTask() {
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				performHandshake();																									
			}			
		});
	}
	
	private void stopAllTasks() {
		if(initTask != null) {
			initTask.cancel(true);
		}
		
		if(prepareProductTask != null) {
			prepareProductTask.cancel(true);
		}
		
	}

	@Override
 	public void onBluetoothConnectionLost() {
		hideButtons();
		disconnectBluetoothFromMachine();	
		reconnectBluetoothDevice();
 	}
	

 	@Override
 	public void onBluetoothConnectionFailed() {
 		hideButtons();
 		disconnectBluetoothFromMachine();
 		reconnectBluetoothDevice();
 	}
 	
 	private void reconnectBluetoothDevice() {
 		
 		if(autoConnectFailureCounter--  < 0) {
 			autoConnectFailureCounter = 10;
 			setInfo("Unable to connect to remote device");
 			return;
 		}
 		if(remoteBluetoothDevice != null) {
 			 mConnectionService.connect(remoteBluetoothDevice, true); 			 
 		}
		
	}
 	
 	

	private void disconnectBluetoothFromMachine() {
// 		synchronized (machine) {
// 			if(machine != null) {			
// 				machine.disconnectBluetooth();
// 			}
//		}
 		
 		mConnectionService.stop(); 		
	}
 	
 	private void addPairedDeviceToConfig(String address) {
 		SharedPreferences config = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());		
		config.edit().putString("paired_device_address", address).commit();		
	}
 	
 	private String getPairedDeviceAddressFromConfig() {
 		SharedPreferences config = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());		
		return config.getString("paired_device_address", "");		
	}

    
 	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.option_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Intent serverIntent = null;
        switch (item.getItemId()) {
        case R.id.secure_connect_scan:
            // Launch the DeviceListActivity to see devices and do scan
//        	if(machine.isBluetoothConnected()) {        	

//        	}
            serverIntent = new Intent(this, DeviceListActivity.class);
            startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE_SECURE);
            return true;
            
//        case R.id.unpair_device:
//        	unpairDevice();
//        	return true;
            
        case R.id.exit_application:
        	finish();
        	return true;
        }
        return false;
    }

	private synchronized void disconnectRemoteService() {
		if(remoteService != null) {
 			if(remoteService.isRunning()) remoteService.stop();
 			if(remoteService.isConnected()) remoteService.disconnect();
 		}		
	}

	private void unpairDevice() {
		Thread unpair = new Thread(new Runnable () {

			@Override
			public void run() {
				disconnectPairedDevice();
				addPairedDeviceToConfig("");
				stopConnectionWatchdog();				
			}
			
		}, "Unpair");
		unpair.start();
	}
    
	private void showToast(String message) {
		Message msg = mHandler.obtainMessage(MESSAGE_TOAST);
		Bundle bundle = new Bundle();
		bundle.putString(TOAST, message);       
		msg.setData(bundle);
		mHandler.sendMessage(msg);
	}
	
	private void showToast(int resourceCode) {
		showToast(getString(resourceCode));
	}	
	

	
	private void sleepSeconds(int delay) {
		try {
			TimeUnit.SECONDS.sleep(delay);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	
	public synchronized boolean isReconnecting() {
		return reconnecting;
	}

	public synchronized void setReconnecting(boolean reconnecting) {
		this.reconnecting = reconnecting;
	}
	
	
	public synchronized BluetoothConnectionService getBluetoothConnectionService() {
		return mConnectionService;
	}

	@Override
	public void onAdapterStateOff(int previousState) {
		taskExecutor.submit(new Runnable() {			
			@Override
			public void run() {
				showToast("Bluetooth adapter is turned OFF");
				runOnUiThread(new Runnable() {					
					@Override
					public void run() {
						setInfo("Bluetooth is turned off");						
					}
				});
				
				hideButtons();
				disconnectRemoteService();				
		 		disconnectBluetoothFromMachine();		 		
			}
		});	
		
	}

	@Override
	public void onAdapterStateTurningOn(int previousState) {
//		taskExecutor.submit(new Runnable() {			
//			@Override
//			public void run() {
//				hideButtons();
//		 		disconnectBluetoothFromMachine();
//		 		reconnectBluetoothDevice();				
//			}
//		});			
	}

	@Override
	public void onAdapterStateOn(int previousState) {
		showToast("Bluetooth adapter turned ON");		
	}

	@Override
	public void onAdapterStateTurningOff(int previousState) {
		taskExecutor.submit(new Runnable() {			
			@Override
			public void run() {
				showToast("Bluetooth adapter is turning OFF");
				runOnUiThread(new Runnable() {					
					@Override
					public void run() {
						setInfo("Bluetooth was is turning OFF");						
					}
				});
				
				hideButtons();
				disconnectRemoteService();				
		 		disconnectBluetoothFromMachine();		 		
			}
		});			
	}

	@Override
	public void onBluetoothConnect(BluetoothDevice device) {
		showToast("Bluetooth connected to " + device.getName());		
	}

	@Override
	public void onBluetoothDisconnect(BluetoothDevice device) {
		taskExecutor.submit(new Runnable() {			
			@Override
			public void run() {
				showToast("Bluetooth was disconnected");
				runOnUiThread(new Runnable() {					
					@Override
					public void run() {
						setInfo("Bluetooth was disconnected");
						
					}
				});
				
				stopAllTasks();
				hideButtons();
				disconnectRemoteService();				
		 		disconnectBluetoothFromMachine();		 		
			}

			
		});	
		
	}

	@Override
	public void onBluetoothDisconnectRequested(BluetoothDevice device) {
		taskExecutor.submit(new Runnable() {			
			@Override
			public void run() {
				showToast("Bluetooth is going to disconnect");
				hideButtons();
				disconnectRemoteService();				
		 		disconnectBluetoothFromMachine();		 		
			}
		});			
	}
	

}
