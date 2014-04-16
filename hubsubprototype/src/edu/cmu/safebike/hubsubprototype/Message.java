package edu.cmu.safebike.hubsubprototype;

import java.util.HashMap;
import java.util.Map;

/**
 * {@link Plugin}s publish and receive {@link Message}s via the {@link Hub}.
 * 
 * TODO: should messages identify sender? or just topic?
 */
public class Message {
	// Topics of {@link Message}s.
	public static enum Topic {
		ALERT,
		SUGGESTION
	}
	
	// Names of {@link Message}s, by {@link Message.Topic}.
		static Map<Topic, String> namesByTopic; 
		static {
			namesByTopic = new HashMap<Topic, String>();
			namesByTopic.put(Topic.ALERT, "Alert");
			namesByTopic.put(Topic.SUGGESTION, "Suggestion");
	}
	
	// Attributes of {@link Message}s, by {@link Message.Topic}.
	static Map<Topic, String[]> attributesByTopic; 
	static {
		attributesByTopic = new HashMap<Topic, String[]>();
		attributesByTopic.put(Topic.ALERT, new String[]{"text", "severity"});
		attributesByTopic.put(Topic.SUGGESTION, new String[]{"text"});
	}

	private Topic topic;
	private String[] attributes;
	
	public Topic getTopic() {
		return topic;
	}
	
	public String[] getAttributes() {
		// TODO: copy first?
		return attributes;
	}
	
	@Override
	public String toString() {
		String str = namesByTopic.get(topic);
		str += " {";

		String[] attributeNames = attributesByTopic.get(topic);
		for (int i = 0; i < attributes.length; i++) {
			str += String.format("%s: %s,", attributeNames[i], attributes[i]);
		}
		
		str += "}";
		return str;
	}
	
	public Message(Topic topic, String[] attributes) {
		if (attributesByTopic.containsKey(topic)) {
			this.topic = topic;
		} else {
			throw new RuntimeException("Invalid Topic given to Message.");
		}
		
		if (attributesByTopic.get(topic).length == attributes.length) {
			this.attributes = new String[attributes.length];
			System.arraycopy(attributes, 0, this.attributes, 0, attributes.length);
		} else {
			String err = String.format(
					"Invalid attributes: Message with topic %s requires %d attributes, not %d",
					namesByTopic.get(topic), attributesByTopic.get(topic).length, attributes.length);
			throw new RuntimeException(err);
		}
	}
}
