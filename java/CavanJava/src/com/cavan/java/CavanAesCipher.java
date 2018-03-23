package com.cavan.java;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

public class CavanAesCipher {

	private static final String AES_PASSWORD = "CavanAesPassword";
	private static final SecretKey AES_KEY = new SecretKeySpec(AES_PASSWORD.getBytes(), "AES");

	private SecretKey mKey;

	public CavanAesCipher(SecretKey key) {
		mKey = key;
	}

	public CavanAesCipher() {
		this(AES_KEY);
	}

	public void setKey(SecretKey key) {
		mKey = key;
	}

	public SecretKey getKey() {
		return mKey;
	}

	public boolean setKey(byte[] password) {
		try {
			KeyGenerator generator = KeyGenerator.getInstance("AES");
			SecureRandom random = SecureRandom.getInstance("SHA1PRNG");

			random.setSeed(password);
			generator.init(128, random);
			setKey(generator.generateKey());

			return true;
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}

		return false;
	}

	public boolean setKey(String password) {
		return setKey(password.getBytes());
	}

	public byte[] crypt(byte[] bytes, int opmode) {
		try {
			Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
			cipher.init(opmode, mKey);
			return cipher.doFinal(bytes);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public byte[] encrypt(byte[] bytes) {
		return crypt(bytes, Cipher.ENCRYPT_MODE);
	}

	public byte[] encrypt(String text) {
		return encrypt(text.getBytes());
	}

	public byte[] AesDecrypt(byte[] bytes) {
		return crypt(bytes, Cipher.DECRYPT_MODE);
	}
}
