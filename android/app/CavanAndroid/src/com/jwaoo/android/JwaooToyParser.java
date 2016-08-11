package com.jwaoo.android;


@SuppressWarnings("serial")
public class JwaooToyParser extends JwaooDepthDecoder {

	public JwaooToyParser(double fuzz) {
		super(fuzz);
	}

	public void putData(JwaooToySensor sensor) {
		putCapacityValue(sensor.getCapacitys());
	}
}