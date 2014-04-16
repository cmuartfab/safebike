package edu.cmu.safebike.hubsubprototype;

/**
 * Objects that publish {@link Message}s to/from the {@link Hub} must implement this interface.
 * 
 * TODO: should Plugins have a method which gives a unique identifier?
 * TODO: should plugins have a stop() method?
 */
public interface Plugin {
	public void start(final Hub hub);
	public void receive(final Message message);
}
