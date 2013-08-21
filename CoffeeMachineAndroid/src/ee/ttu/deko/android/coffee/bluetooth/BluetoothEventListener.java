package ee.ttu.deko.android.coffee.bluetooth;

import android.bluetooth.BluetoothDevice;

public interface BluetoothEventListener {

	void onAdapterStateOff(int previousState);

	void onAdapterStateTurningOn(int previousState);

	void onAdapterStateOn(int previousState);

	void onAdapterStateTurningOff(int previousState);

	void onBluetoothConnect(BluetoothDevice device);

	void onBluetoothDisconnect(BluetoothDevice device);

	void onBluetoothDisconnectRequested(BluetoothDevice device);
		
}
