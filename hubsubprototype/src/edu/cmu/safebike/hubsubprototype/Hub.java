package edu.cmu.safebike.hubsubprototype;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * A {@link Hub} is a simple publish-subscribe system.
 * {@link Plugin}s can subscribe to {@link Message.Topic}s on the {@link Hub}. Then, they will
 * the {@link Message}s that other {@link Plugin}s publish to these {@link Message.Topic}s.
 */
public class Hub {

	private Map<Message.Topic, Set<Plugin>> subscriptions = new HashMap<Message.Topic, Set<Plugin>>();
	
	public void publish(Message message) {
		Message.Topic topic = message.getTopic();
		if (subscriptions.containsKey(topic)) {
			Set<Plugin> plugins = subscriptions.get(topic);
			for (Plugin plugin : plugins) {
				plugin.receive(message);
			}
		}
	}

	// TODO: add config to add subscription params (e.g. throttling)
	public void subscribe(Plugin plugin, Message.Topic topic) {
		if (!subscriptions.containsKey(topic)) {
			subscriptions.put(topic, new HashSet<Plugin>());
		}
		subscriptions.get(topic).add(plugin);
	}
}
