package com.cavan.java;

public class WifiQrCodeParser {

	private String mType;
	private String mSsid;
	private String mPass;

	public String getType() {
		return mType;
	}

	public void setType(String type) {
		mType = type;
	}

	public String getSsid() {
		return mSsid;
	}

	public void setSsid(String ssid) {
		mSsid = ssid;
	}

	public String getPass() {
		return mPass;
	}

	public void setPass(String pass) {
		mPass = pass;
	}

	public boolean parse(String text) {
		mType = null;
		mSsid = null;
		mPass = null;

		if (!text.startsWith("WIFI:")) {
			return false;
		}

		for (int i = 5; i < text.length(); i++) {
			StringBuilder builder = new StringBuilder();
			String key = null;

			label_once: while (i < text.length()) {
				char c = text.charAt(i);

				switch (c) {
				case ':':
					key = builder.toString();
					builder = new StringBuilder();
					break;

				case '"':
					label_value: while (++i < text.length()) {
						c = text.charAt(i);

						switch (c) {
						case '\\':
							if (++i < text.length()) {
								builder.append(text.charAt(i));
							}
							break;

						case '"':
							break label_value;

						default:
							builder.append(c);
						}
					}
					break;

				case ';':
					break label_once;

				default:
					builder.append(c);
				}

				i++;
			}

			if (key != null && key.length() == 1 && builder.length() > 0) {
				switch (key.charAt(0)) {
				case 'T':
					mType = builder.toString();
					break;

				case 'S':
					mSsid = builder.toString();
					break;

				case 'P':
					mPass = builder.toString();
					break;
				}
			}
		}

		return true;
	}

	public static void main(String[] args) {
		WifiQrCodeParser parser = new WifiQrCodeParser();

		if (parser.parse("WIFI:T:WPA;S:\"test-ssid\";P:test-pass;;")) {
			CavanJava.dLog("type = " + parser.getType());
			CavanJava.dLog("ssid = " + parser.getSsid());
			CavanJava.dLog("pass = " + parser.getPass());
		}
	}

}
