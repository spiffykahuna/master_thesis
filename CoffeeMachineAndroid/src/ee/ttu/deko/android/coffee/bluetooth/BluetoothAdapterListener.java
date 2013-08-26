package ee.ttu.deko.android.coffee.bluetooth;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;


import static android.bluetooth.BluetoothAdapter.*;
	
public class BluetoothAdapterListener extends BroadcastReceiver {
	protected final Logger logger = LoggerFactory.getLogger(BluetoothAdapterListener.class.getSimpleName());
	
	protected static final Set<BluetoothEventListener> eventListeners = new CopyOnWriteArraySet<BluetoothEventListener>();
	
	public BluetoothAdapterListener() {}

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();

		if (ACTION_STATE_CHANGED.equals(action)) {
			checkAdapterStateChange(intent);
		}
		
		if(ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
			checkConnectionStateChange(intent);
		}
		
		if(BluetoothDevice.ACTION_ACL_CONNECTED.equals(action)) {
			BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
			logger.info("Bluetooth connection state changed: ACTION_ACL_CONNECTED device={}", device);			
			onBluetoothConnect(device);
		}
		
		if(BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
			BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
			logger.info("Bluetooth connection state changed: ACTION_ACL_DISCONNECTED device={}", device);			
			onBluetoothDisconnect(device);
		}
		
		if(BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED.equals(action)) {
			BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
			logger.info("Bluetooth connection state changed: ACTION_ACL_DISCONNECT_REQUESTED device={}", device);			
			onBluetoothDisconnectRequested(device);
		}
		
	}

	private void onBluetoothConnect(BluetoothDevice device) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onBluetoothConnect(device);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onBluetoothConnect event: listener=%s", listener.toString())
						, e
				);
			}
		}		
	}
	
	private void onBluetoothDisconnect(BluetoothDevice device) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onBluetoothDisconnect(device);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onBluetoothDisconnect event: listener=%s", listener.toString())
						, e
				);
			}
		}		
	}
	
	private void onBluetoothDisconnectRequested(BluetoothDevice device) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onBluetoothDisconnectRequested(device);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onBluetoothDisconnectRequested event: listener=%s", listener.toString())
						, e
				);
			}
		}		
	}
	
	
	

	private void checkConnectionStateChange(Intent intent) {
		int connState = intent.getIntExtra(EXTRA_CONNECTION_STATE, 0);
		int previousConnState = intent.getIntExtra(EXTRA_PREVIOUS_CONNECTION_STATE, 0);
		
		BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
		
		
		switch(connState) {
		case STATE_DISCONNECTED:
			logger.info("Bluetooth connection state changed: {} -> {} Device: {}",
					adapterStateToString(previousConnState),
					adapterStateToString(connState),
					device);
			break;
		
		case STATE_CONNECTING:
			logger.info("Bluetooth connection state changed: {} -> {} Device: {}",
					adapterStateToString(previousConnState),
					adapterStateToString(connState),
					device);
			break;
		case STATE_CONNECTED:
			logger.info("Bluetooth connection state changed: {} -> {} Device: {}",
					adapterStateToString(previousConnState),
					adapterStateToString(connState),
					device);
			break;
		case STATE_DISCONNECTING:
			logger.info("Bluetooth connection state changed: {} -> {} Device: {}",
					adapterStateToString(previousConnState),
					adapterStateToString(connState),
					device);
			break;
			
		default:
			logger.info("Bluetooth connection unknown state change: {} -> {} Device: {}",
					adapterStateToString(previousConnState),
					adapterStateToString(connState),
					device);
			break;		
		}
		
	}

	private void checkAdapterStateChange(Intent intent) {
		int state = intent.getIntExtra(EXTRA_STATE, 0);
		int previousState = intent.getIntExtra(EXTRA_PREVIOUS_STATE, 0);
		
		switch(state) {
		case STATE_OFF:
			logger.info("Bluetooth adapter state changed: {} -> {}", adapterStateToString(previousState), adapterStateToString(state));
			onAdapterStateOff(previousState);
			break;
		case STATE_TURNING_ON:
			logger.info("Bluetooth adapter state changed: {} -> {}", adapterStateToString(previousState), adapterStateToString(state));
			onAdapterStateTurningOn(previousState);
			break;
		case STATE_ON:
			logger.info("Bluetooth adapter state changed: {} -> {}", adapterStateToString(previousState), adapterStateToString(state));
			onAdapterStateOn(previousState);
			break;
		case STATE_TURNING_OFF:
			logger.info("Bluetooth adapter state changed: {} -> {}", adapterStateToString(previousState), adapterStateToString(state));
			break;
		default:
			logger.info("Bluetooth adapter unknown state: {} -> {}", adapterStateToString(previousState), adapterStateToString(state));
			break;
		}
	}

	private synchronized void onAdapterStateTurningOn(int previousState) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onAdapterStateTurningOn(previousState);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onAdapterStateTurningOn event: listener=%s", listener.toString())
						, e
				);
			}
		}
	}
	
	private synchronized void onAdapterStateOff(int previousState) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onAdapterStateOff(previousState);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onAdapterStateOff event: listener=%s", listener.toString())
						, e
				);
			}
		}
	}
	
	private synchronized void onAdapterStateOn(int previousState) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onAdapterStateOn(previousState);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onAdapterStateOn event: listener=%s", listener.toString())
						, e
				);
			}
		}
	}

	private synchronized void onAdapterStateTurningOff(int previousState) {
		for(BluetoothEventListener listener: eventListeners) {
			try{
				listener.onAdapterStateTurningOff(previousState);
			} catch(Exception e) {				
				logger.error(
						String.format("Failed to inform listener about onAdapterStateTurningOff event: listener=%s", listener.toString())
						, e
				);
			}
		}
	}

	public static String adapterStateToString(int state) {
		
		switch(state) {
		case STATE_OFF:				return "STATE_OFF";			
			
		case STATE_TURNING_ON:		return "STATE_TURNING_ON";
			
		case STATE_ON:				return "STATE_ON";
				
		case STATE_TURNING_OFF:		return "STATE_TURNING_OFF";
		
		case STATE_DISCONNECTED:	return "STATE_DISCONNECTED";
		case STATE_CONNECTING:		return "STATE_CONNECTING";
		case STATE_CONNECTED:		return "STATE_CONNECTED";
		case STATE_DISCONNECTING:	return "STATE_DISCONNECTING";
		
		default:					return "UNKNOWN";
		}		
	} 
	
	public static boolean addEventListener(BluetoothEventListener listener) {
		return eventListeners.add(listener);		 
	}
	
	public static boolean removeEventListener(BluetoothEventListener listener) {
		return eventListeners.remove(listener);		 
	}
}


