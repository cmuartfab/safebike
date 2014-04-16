package edu.cmu.safebike.hubsubprototype;

import android.app.Activity;
import android.widget.TextView;

/**
 * Example {@link Plugin} which performs output tasks.
 */
public class OutputExample implements Plugin {

	private Activity activity;
	
	@Override
	public void start(final Hub hub) {
		hub.subscribe(this, Message.Topic.ALERT);
		hub.subscribe(this, Message.Topic.SUGGESTION);
	}

	@Override
	public void receive(final Message message) {
		System.out.println(String.format("Got message: %s", message));
		
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				TextView tv = (TextView) activity.findViewById(R.id.my_text_view);
				tv.append(message.toString() + "\n");
			}
		});
	}
	
	public OutputExample(Activity activity) {
		this.activity = activity;
	}
}
