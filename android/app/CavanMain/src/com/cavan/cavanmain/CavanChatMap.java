package com.cavan.cavanmain;

import java.util.HashMap;
import java.util.Iterator;

public class CavanChatMap extends HashMap<Integer, CavanChatNode> {

	private static final long serialVersionUID = -7273191141819850152L;
	private static final int MAX_SIZE = 100;

	@Override
	public CavanChatNode put(Integer key, CavanChatNode value) {
		if (size() > MAX_SIZE) {
			Iterator<CavanChatNode> iterator = values().iterator();
			long time = System.currentTimeMillis();

			while (iterator.hasNext()) {
				CavanChatNode node = iterator.next();
				if (node.isOvertime(time)) {
					iterator.remove();
				}
			}
		}

		return super.put(key, value);
	}

	public CavanChatNode put(Integer key, String value) {
		CavanChatNode node = get(key);
		if (node != null) {
			node.setName(value);
		} else {
			node = new CavanChatNode(value);
			put(key, node);
		}

		return node;
	}
}
