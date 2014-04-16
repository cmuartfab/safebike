package edu.cmu.safebike.hubsubprototype;

import java.util.Random;

import android.app.Activity;

/**
 * Example {@link Plugin} which performs input tasks.
 */
public class InputExample implements Plugin {
	
	private static final Random r = new Random();
	private static final int MAX_SEVERITY = 5;
	
	@Override
	public void start(final Hub hub) {
		
		// Publish ALERT messages every 5 seconds.
		new Thread() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(5000);
					} catch (InterruptedException e) {}
					int severity = r.nextInt(MAX_SEVERITY);
					hub.publish(new Message(Message.Topic.ALERT, new String[]{
							"test suggestion",
							Integer.toString(severity)}));
				}
			}
		}.start();
		
		// Publish SUGGESTION messages every 3 seconds.
		new Thread() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(3000);
					} catch (InterruptedException e) {}
					hub.publish(new Message(Message.Topic.SUGGESTION, new String[]{"test suggestion"}));
				}
			}
		}.start();
	}

	@Override
	public void receive(final Message message) {}
}
