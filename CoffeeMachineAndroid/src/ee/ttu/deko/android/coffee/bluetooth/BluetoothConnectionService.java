/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ee.ttu.deko.android.coffee.bluetooth;

import java.io.IOException;

import java.util.UUID;

import ee.ttu.deko.android.coffee.ui.BluetoothProductsActivity;



import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

/**
 * This class does all the work for setting up and managing Bluetooth
 * connections with other devices. It has a thread that listens for
 * incoming connections, a thread for connecting with a device, and a
 * thread for performing data transmissions when connected.
 */
public class BluetoothConnectionService {
    // Debugging
    private static final String DEBUG_TAG = "BluetoothControlService";
    private static final boolean DEBUG = true;

    // Name for the SDP record when creating server socket
    private static final String NAME_SECURE = "BluetoothControlSecure";
    private static final String NAME_INSECURE = "BluetoothControlInsecure";

    // Unique UUID for this application
    private static final UUID MY_UUID_SECURE =
        //UUID.fromString("fa87c0d0-afac-11de-8a39-0800200c9a66");
    	// this UUID is SerialPortServiceClass_UUID
    	UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private static final UUID MY_UUID_INSECURE =
        //UUID.fromString("2e7657e0-3adc-11e2-81c1-0800200c9a66");
    	UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    
   
    
    // Member fields
    private final BluetoothAdapter mAdapter;
    private final Handler mHandler;
    private final BluetoothConnectionListener mConnectionListener;
    private static AcceptThread mSecureAcceptThread;
    
    private static ConnectThread mConnectThread;

    private int mState;
	private static boolean isStarted;

    // Constants that indicate the current connection state
    public static final int STATE_NONE = 0;       // we're doing nothing
    public static final int STATE_LISTEN = 1;     // now listening for incoming connections
    public static final int STATE_CONNECTING = 2; // now initiating an outgoing connection
    public static final int STATE_CONNECTED = 3;  // now connected to a remote device
    public static final int STATE_CLOSING = 4;  // now connected to a remote device
    public static final int STATE_UNAVAILABLE = 5;  // now connected to a remote device
    
    
    private static final int REQUEST_ENABLE_BT = 3;
    
    public interface BluetoothConnectionListener {
    	/**
    	 * Execute any action when bluetooth connection is established.
    	 * @param socket - bluetooth connection socket. You may get streams from this socket     	 * 
    	 * @param device - connected device hardware information
    	 * @param socketType - secure, insecure connection
    	 */
		void onBluetoothConnectionEstablished(BluetoothSocket socket,
				BluetoothDevice device, String socketType);
		/**
		 * Execute any action when bluetooth connection is lost.
		 */
		void onBluetoothConnectionLost();
		void onBluetoothConnectionFailed();
    	
    }

    /**
     * Constructor. Prepares a new BluetoothChat session.
     * @param context  The UI Activity Context
     * @param handler  A Handler to send messages back to the UI Activity
     */
    public BluetoothConnectionService(Context context, Handler handler,
    		BluetoothConnectionListener listener) {
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mState = STATE_NONE;
        mHandler = handler;
        mConnectionListener = listener;
    }

    /**
     * Set the current state of the chat connection
     * @param state  An integer defining the current connection state
     */
    private synchronized void setState(int state) {
        if (DEBUG) Log.d(DEBUG_TAG, "setState() " + mState + " -> " + state);
        mState = state;

        // Give the new state to the Handler so the UI Activity can update
        mHandler.obtainMessage(BluetoothProductsActivity.MESSAGE_STATE_CHANGE, state, -1).sendToTarget();
    }

    /**
     * Return the current connection state. */
    public synchronized int getState() {
        return mState;
    }

    /**
     * Start the chat service. Specifically start AcceptThread to begin a
     * session in listening (server) mode. Called by the Activity onResume() */
    public synchronized void start() {
    	if (DEBUG) Log.d(DEBUG_TAG, "Starting Bluetooth connection service...");
    	
    	if(isStarted) {
    		throw new IllegalStateException("Service is already started");
    	}

        // Cancel any thread attempting to make a connection
        if (mConnectThread != null) {
        	if (DEBUG) Log.d(DEBUG_TAG, "Cancelling connect thread...");
        	mConnectThread.cancel();         	
//        	mConnectThread = null;
        }
        
        if(mAdapter != null && !mAdapter.isEnabled()) {        	
        	int state = mAdapter.getState();
        	boolean enabled = mAdapter.enable();
        	if (DEBUG) Log.d(DEBUG_TAG, "Bluetooth is turned on? :" + enabled);        	
        }

        setState(STATE_LISTEN);
        
        if(mSecureAcceptThread != null && mSecureAcceptThread.isAlive() ) {
        	if (DEBUG) Log.d(DEBUG_TAG, "Accept thread was already running. Stopping accept thread...");
        	mSecureAcceptThread.cancel();
        	mSecureAcceptThread.interrupt();
//        	mSecureAcceptThread = null;
        }

        // Start the thread to listen on a BluetoothServerSocket
        
    	if (DEBUG) Log.d(DEBUG_TAG, "Starting accept thread...");
        mSecureAcceptThread = new AcceptThread(true);
        mSecureAcceptThread.start();
        while(!mSecureAcceptThread.isAlive()) {
        	try {
				Thread.sleep(100);
			} catch (InterruptedException e) {					
				if (DEBUG) Log.w(DEBUG_TAG, "Starting accept thread was interrupted");       
			}
        }
            
        
        
        isStarted = true;
    }

    /**
     * Start the ConnectThread to initiate a connection to a remote device.
     * @param device  The BluetoothDevice to connect
     * @param secure Socket Security type - Secure (true) , Insecure (false)
     */
    public synchronized void connect(BluetoothDevice device, boolean secure) {
        if (DEBUG) Log.d(DEBUG_TAG, "connect to: " + device + " " + device.getName());

        // Cancel any thread attempting to make a connection
//        if (mState == STATE_CONNECTING) {
            if (mConnectThread != null) {mConnectThread.cancel(); mConnectThread.interrupt();}
            if(mSecureAcceptThread != null) {
            	mSecureAcceptThread.cancel();
            	mSecureAcceptThread.interrupt();
            }
//        }

        // Start the thread to connect with the given device
        mConnectThread = new ConnectThread(device, secure);
        mConnectThread.start();
        
        mSecureAcceptThread = new AcceptThread(true);
        mSecureAcceptThread.start();
        
        
        setState(STATE_CONNECTING);
    }

    /**
     * Start the ConnectedThread to begin managing a Bluetooth connection
     * @param socket  The BluetoothSocket on which the connection was made
     * @param device  The BluetoothDevice that has been connected
     */
    public synchronized void connected(BluetoothSocket socket, BluetoothDevice
            device, final String socketType) {
        if (DEBUG) Log.d(DEBUG_TAG, "connected, Socket Type:" + socketType);

        // Cancel the thread that completed the connection
//        if (mConnectThread != null) {mConnectThread.cancel(); mConnectThread = null;}



        // Cancel the accept thread because we only want to connect to one device
        if (mSecureAcceptThread != null) {
            mSecureAcceptThread.cancel();
//            mSecureAcceptThread = null;
        }
        
        
//        if (mInsecureAcceptThread != null) {
//            mInsecureAcceptThread.cancel();
//            mInsecureAcceptThread = null;
//        }

//        // Start the thread to manage the connection and perform transmissions
//        mConnectedThread = new ConnectedThread(socket, socketType);
//        mConnectedThread.start();

        // Send the name of the connected device back to the UI Activity
        Message msg = mHandler.obtainMessage(BluetoothProductsActivity.MESSAGE_DEVICE_NAME);
        Bundle bundle = new Bundle();
        bundle.putString(BluetoothProductsActivity.DEVICE_NAME, device.getName());
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        
        

        setState(STATE_CONNECTED);
        
        mConnectionListener.onBluetoothConnectionEstablished(socket, device, socketType);
    }

    /**
     * Stop all threads
     */
    public synchronized void stop() {
        if (DEBUG) Log.d(DEBUG_TAG, "Stopping Bluetooth connection service...");

        if (mConnectThread != null) {
        	if (DEBUG) Log.d(DEBUG_TAG, "Cancelling connect thread...");
            mConnectThread.cancel(); 
            mConnectThread.interrupt();
//            mConnectThread = null;
        }      

        if (mSecureAcceptThread != null) {
        	if (DEBUG) Log.d(DEBUG_TAG, "Cancelling accept thread...");
            mSecureAcceptThread.cancel(); 
            mSecureAcceptThread.interrupt();
//            mSecureAcceptThread = null;
        }
        setState(STATE_NONE);
        isStarted = false;
    }
    
    public synchronized void restart() {
    	stop();
    	start();
    }

    /**
     * Write to the ConnectedThread in an unsynchronized manner
     * @param out The bytes to write
     * @see ConnectedThread#write(byte[])
     */
    public void write(byte[] out) {

        // Synchronize a copy of the ConnectedThread
        synchronized (this) {
            if (mState != STATE_CONNECTED) return;
            
        }

    }

    /**
     * Indicate that the connection attempt failed and notify the UI Activity.
     */
    private void connectionFailed(String failureText) {
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(BluetoothProductsActivity.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(BluetoothProductsActivity.TOAST, failureText);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        
        // send signal to connection listener
        mConnectionListener.onBluetoothConnectionFailed();

        // Start the service over to restart listening mode
        BluetoothConnectionService.this.stop();
        BluetoothConnectionService.this.start();
    }

    /**
     * Indicate that the connection was lost and notify the UI Activity.
     */
    private void connectionLost() {
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(BluetoothProductsActivity.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(BluetoothProductsActivity.TOAST, "Device connection was lost");       
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        
        mHandler.obtainMessage(BluetoothProductsActivity.MESSAGE_STATE_CHANGE, mState, -1).sendToTarget();
        
        
        mConnectionListener.onBluetoothConnectionLost();

        // Start the service over to restart listening mode
        BluetoothConnectionService.this.stop();
        BluetoothConnectionService.this.start();
    }

    /**
     * This thread runs while listening for incoming connections. It behaves
     * like a server-side client. It runs until a connection is accepted
     * (or until cancelled).
     */
    private class AcceptThread extends Thread {
        // The local server socket
        private final BluetoothServerSocket mmServerSocket;
        private String mSocketType;

        public AcceptThread(boolean secure) {
            BluetoothServerSocket tmp = null;
            mSocketType = secure ? "Secure":"Insecure";

            // Create a new listening server socket
            try {
                if (secure) {
                    tmp = mAdapter.listenUsingRfcommWithServiceRecord(NAME_SECURE,
                        MY_UUID_SECURE);
                } else {
                    tmp = mAdapter.listenUsingInsecureRfcommWithServiceRecord(
                            NAME_INSECURE, MY_UUID_INSECURE);
                }
            } catch (IOException e) {
                Log.e(DEBUG_TAG, "Socket Type: " + mSocketType + "listen() failed", e);
                try {
					tmp = mAdapter.listenUsingInsecureRfcommWithServiceRecord(
					        NAME_INSECURE, MY_UUID_INSECURE);
				} catch (IOException ignored) {									
				}
            }
            mmServerSocket = tmp;
        }

        public void run() {
            if (DEBUG) Log.d(DEBUG_TAG, "Socket Type: " + mSocketType +
                    " BEGIN mAcceptThread" + this);
            setName("AcceptThread" + mSocketType);

            BluetoothSocket socket = null;

            // Listen to the server socket if we're not connected
            while (mState != STATE_CONNECTED) {
                try {
                    // This is a blocking call and will only return on a
                    // successful connection or an exception
                	if(mmServerSocket != null) {
                		socket = mmServerSocket.accept();
                	} else {
                		Log.e(DEBUG_TAG, "Socket is NULL");
                		try {
                			if(socket != null) {
                				socket.close();
                			}                            
                        } catch (IOException e) {
                            Log.e(DEBUG_TAG, "Could not close unwanted socket", e);
                        }
                		break;
                	}
                } catch (IOException e) {
                    Log.e(DEBUG_TAG, "Socket Type: " + mSocketType + "  accept() failed", e);                    
                    break;
                }

                // If a connection was accepted
                if (socket != null) {
                    synchronized (BluetoothConnectionService.this) {
                        switch (mState) {
                        case STATE_LISTEN:
                        case STATE_CONNECTING:
                            // Situation normal. Start the connected thread.
                            connected(socket, socket.getRemoteDevice(),
                                    mSocketType);
                            break;
                        case STATE_NONE:
                        case STATE_CONNECTED:
                            // Either not ready or already connected. Terminate new socket.
                            try {
                            	if(socket != null) {
                            		socket.close();
                            	}                                
                            } catch (IOException e) {
                                Log.e(DEBUG_TAG, "Could not close unwanted socket", e);
                            }
                            break;
                        }
                    }
                }
            }
            if (DEBUG) Log.i(DEBUG_TAG, "END mAcceptThread, socket Type: " + mSocketType);

        }

        public synchronized void cancel() {
            if (DEBUG) Log.d(DEBUG_TAG, "Socket Type" + mSocketType + "cancel " + this);
            try {
            	if(mmServerSocket != null) {
            		if (DEBUG) Log.d(DEBUG_TAG, "Closing server socket: " + mmServerSocket);
            		mmServerSocket.close();
            	}                
            } catch (IOException e) {
                Log.e(DEBUG_TAG, "Socket Type" + mSocketType + "close() of server failed", e);
            }
        }
    }


    /**
     * This thread runs while attempting to make an outgoing connection
     * with a device. It runs straight through; the connection either
     * succeeds or fails.
     */
    private class ConnectThread extends Thread {
        private BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;
        private String mSocketType;
        
        private static final int RETRIES = 10;
        
        public ConnectThread(BluetoothDevice device, boolean secure) {
            mmDevice = device;
            BluetoothSocket tmp = null;
            mSocketType = secure ? "Secure" : "Insecure";
            

            // Get a BluetoothSocket for a connection with the
            // given BluetoothDevice
            try {
                if (secure) {
                    tmp = device.createRfcommSocketToServiceRecord(
                            MY_UUID_SECURE);
                } else {
                    tmp = device.createInsecureRfcommSocketToServiceRecord(
                            MY_UUID_INSECURE);
                }
            } catch (IOException e) {
                Log.e(DEBUG_TAG, "Socket Type: " + mSocketType + "create() failed", e);
            }
            mmSocket = tmp;
        }

        public void run() {
            Log.i(DEBUG_TAG, "BEGIN mConnectThread Socket Type:" + mSocketType);
            setName("ConnectThread" + mSocketType);

            // Always cancel discovery because it will slow down a connection
            try {
	            while(mAdapter.isDiscovering()) {
	            	mAdapter.cancelDiscovery();	            	
					Thread.sleep(500);					
	            }
	            
	            while(BluetoothConnectionService.mSecureAcceptThread == null || 
	            		!BluetoothConnectionService.mSecureAcceptThread.isAlive()) {
	            	Thread.sleep(100);
	            }
	            
            } catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
            synchronized (mmSocket) {				
				int retries = RETRIES;
	            // Make a connection to the BluetoothSocket
				
				if(mmSocket != null) {
            		while(!mmSocket.isConnected() && (retries-- > 0)) {
            			
			            try {
			                // This is a blocking call and will only return on a
			                // successful connection or an exception
			            	sleepMs(2000);
			            	mmSocket.connect();	
			            	
			            } catch (IOException e) {
			            	if (DEBUG) Log.e(DEBUG_TAG, "Connection failed: ", e);
			                // Close the socket
			                try {
			                	if(mmSocket != null) {
			                		mmSocket.close();
			                		if ("Secure".equals(mSocketType)) {
			                			mmSocket = mmDevice.createRfcommSocketToServiceRecord(
			                                    MY_UUID_SECURE);
			                        } else {
			                        	mmSocket = mmDevice.createInsecureRfcommSocketToServiceRecord(
			                                    MY_UUID_INSECURE);
			                        }
			                	}
			                    
			                } catch (IOException e2) {
			                    Log.e(DEBUG_TAG, "unable to close() " + mSocketType +
			                            " socket during connection failure", e2);
			                }			               
			            }
            		}
				} else {
            		connectionFailed("Unable to retrieve socket");
                    return;
            	}
	            
				if(mmSocket != null && mmSocket.isConnected()) {
					connected(mmSocket, mmDevice, mSocketType);
				} else {
					connectionFailed("Unable to retrieve socket");
				}
	            
	
	            // Reset the ConnectThread because we're done
	//            synchronized (BluetoothConnectionService.this) {
	//                mConnectThread = null;
	//            }
	
	            // Start the connected thread
	            
            }
        }

		private void sleepMs(long ms) {
			try {
				Thread.sleep(ms);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

        @SuppressLint("NewApi")
		public synchronized void cancel() {       	
        	
    		try {
    			if (DEBUG) Log.d(DEBUG_TAG, "Closing bluetooth socket: " + mmSocket);
//    			if(mmSocket.isConnected()) {
    				mmSocket.close();
//    			}     			
    			setState(STATE_NONE);                      
            } catch (IOException e) {
                Log.e(DEBUG_TAG, "close() of connect " + mSocketType + " socket failed", e);
            }        		
            
        }
    }
    
    public class BluetoothAdapterListener extends BroadcastReceiver {
    	
    	public BluetoothAdapterListener() {}

    	@Override
    	public void onReceive(Context context, Intent intent) {
    		String action = intent.getAction();

    		if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
    			UsbAccessory accessory = (UsbAccessory) intent
    					.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
    			if (accessory != null) {
    				
    				System.exit(0);
    			}
    		}
    		
    	} 
    	
    }
}
