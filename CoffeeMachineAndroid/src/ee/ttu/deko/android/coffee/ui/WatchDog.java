package ee.ttu.deko.android.coffee.ui;

import java.util.concurrent.TimeUnit;

import android.util.Log;

public class WatchDog implements Runnable {
	private static final String DEBUG_TAG = WatchDog.class.getSimpleName();
    private static final boolean DEBUG = true;

	private Runnable task;
	
	private long delay, initialDelay;

	private boolean isDead = false;
	
	private Thread watchDog;
	private Thread mission;
	
	public WatchDog(long delay) {
		this.delay = delay;
		this.initialDelay = delay;
	}
	
	public synchronized void onDie(Runnable mission) {
		this.task = mission;
	}
	
	@Override
	public void run() {
		
		while(delay-- > 0) {			
			if(isDead()) {
				return;
			}
					
			try {
				TimeUnit.MILLISECONDS.sleep(1);
			} catch (InterruptedException e) {			
				if(DEBUG) Log.e(DEBUG_TAG, "Sleep failed", e);
			}			
		}
		
		mission = new Thread(task, WatchDog.class.getSimpleName());
		mission.start();
		
		while(mission.isAlive()) {
			if(isDead()) {
				return;
			}
			try {
				TimeUnit.MILLISECONDS.sleep(100);
			} catch (InterruptedException e) {			
				if(DEBUG) Log.e(DEBUG_TAG, "Mission was interrupted", e);
			}
		}		
		isDead = true;		
	}
	
	public synchronized void update() {		
		delay = initialDelay;
	}
	
	public long getDelay() {
		return delay;
	}

	public void setDelay(long delay) {
		this.delay = delay;
		this.initialDelay = delay;
	}
	
	public synchronized boolean isDead() {
		return isDead;
	}
	
	
	public void start() {
		watchDog = new Thread(this, WatchDog.class.getSimpleName());
		watchDog.start();
	}
	
	public void kill() throws InterruptedException {
		synchronized (this) {
			isDead = true;
		}
		
		if(watchDog != null) {
			watchDog.join();		
			watchDog = null;
		}		
	}
}
