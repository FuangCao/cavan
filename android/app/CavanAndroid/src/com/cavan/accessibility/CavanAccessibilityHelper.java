package com.cavan.accessibility;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.content.Context;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeInfo.AccessibilityAction;
import android.widget.AbsListView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TabHost;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanJava.Closure;
import com.cavan.java.CavanJava.ClosureVoid;
import com.cavan.java.CavanString;

public class CavanAccessibilityHelper {

	public static class CavanAccessibilityViewId {

		private String mValue;

		public synchronized String get() {
			return mValue;
		}

		public synchronized void set(String value) {
			mValue = value;
		}

		public synchronized boolean isValid() {
			return mValue != null;
		}

		public synchronized void clear() {
			mValue = null;
		}

		public synchronized AccessibilityNodeInfo find(AccessibilityNodeInfo root) {
			String viewId = mValue;
			if (viewId == null) {
				return null;
			}

			return CavanAccessibilityHelper.findNodeByViewId(root, viewId);
		}

		public synchronized List<AccessibilityNodeInfo> findAll(AccessibilityNodeInfo root) {
			String viewId = mValue;
			if (viewId == null) {
				return null;
			}

			return root.findAccessibilityNodeInfosByViewId(viewId);
		}

		public synchronized AccessibilityNodeInfo findLast(AccessibilityNodeInfo root) {
			List<AccessibilityNodeInfo> nodes = findAll(root);
			if (nodes == null) {
				return null;
			}

			Iterator<AccessibilityNodeInfo> iterator = nodes.iterator();

			if (iterator.hasNext()) {
				AccessibilityNodeInfo node = iterator.next();

				while (iterator.hasNext()) {
					node.recycle();
					node = iterator.next();
				}

				return node;
			}

			return null;
		}
	}

	public static final String CLASS_VIEW = View.class.getName();
	public static final String CLASS_BUTTON = Button.class.getName();
	public static final String CLASS_TAB_HOST = TabHost.class.getName();
	public static final String CLASS_TEXT_VIEW = TextView.class.getName();
	public static final String CLASS_EDIT_TEXT = EditText.class.getName();
	public static final String CLASS_ABS_LIST_VIEW = AbsListView.class.getName();
	public static final String CLASS_IMAGE_BUTTON = ImageButton.class.getName();

	public static String getClassName(AccessibilityNodeInfo node) {
		CharSequence sequence = node.getClassName();
		if (sequence == null) {
			return null;
		}

		return sequence.toString();
	}

	public static boolean isInstanceOf(AccessibilityNodeInfo node, String clsName) {
		CharSequence sequence = node.getClassName();
		if(sequence == null) {
			return false;
		}

		return clsName.equals(sequence.toString());
	}

	public static boolean isView(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_VIEW);
	}

	public static boolean isButton(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_BUTTON);
	}

	public static boolean isTextView(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_TEXT_VIEW);
	}

	public static boolean isEditText(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_EDIT_TEXT);
	}

	public static boolean isTabHost(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_TAB_HOST);
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes, int start, int end) {
		while (start < end) {
			try {
				nodes.get(start++).recycle();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes, int start) {
		recycleNodes(nodes, start, nodes.size());
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes) {
		recycleNodes(nodes, 0);
	}

	public static void recycleNodes(AccessibilityNodeInfo[] nodes, int index, int count) {
		for (int end = index + count; index < end; index++) {
			try {
				nodes[index].recycle();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static void recycleNodes(AccessibilityNodeInfo[] nodes, int count) {
		recycleNodes(nodes, 0, count);
	}

	public static void recycleNodes(AccessibilityNodeInfo[] nodes) {
		recycleNodes(nodes, 0, nodes.length);
	}

	public static List<AccessibilityNodeInfo> findNodesByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return null;
		}

		Iterator<AccessibilityNodeInfo> iterator = nodes.iterator();

		while (iterator.hasNext()) {
			AccessibilityNodeInfo node = iterator.next();

			if (!text.equals(getNodeText(node))) {
				iterator.remove();
				node.recycle();
			}
		}

		return nodes;
	}

	public static List<AccessibilityNodeInfo> findNodesByTexts(AccessibilityNodeInfo root, String... texts) {
		List<AccessibilityNodeInfo> infos = new ArrayList<AccessibilityNodeInfo>();
		for (String text : texts) {
			List<AccessibilityNodeInfo> nodes = findNodesByText(root, text);
			if (nodes == null) {
				continue;
			}

			for (AccessibilityNodeInfo node : nodes) {
				infos.add(node);
			}
		}

		return infos;
	}

	public static List<AccessibilityNodeInfo> findChildsByClassName(AccessibilityNodeInfo parent, String clsName) {
		List<AccessibilityNodeInfo> nodes = new ArrayList<AccessibilityNodeInfo>();
		int childs = parent.getChildCount();

		for (int i = 0; i < childs; i++) {
			AccessibilityNodeInfo child = parent.getChild(i);
			if (child == null) {
				continue;
			}

			if (isInstanceOf(child, clsName)) {
				nodes.add(child);
			} else {
				child.recycle();
			}
		}

		return nodes;
	}

	public static AccessibilityNodeInfo findChildByClassName(AccessibilityNodeInfo parent, String clsName) {
		int childs = parent.getChildCount();

		for (int i = 0; i < childs; i++) {
			AccessibilityNodeInfo child = parent.getChild(i);
			if (child == null) {
				continue;
			}

			if (isInstanceOf(child, clsName)) {
				return child;
			}

			child.recycle();
		}

		return null;
	}

	public static void findNodesByClassName(List<AccessibilityNodeInfo> list, AccessibilityNodeInfo root, String clsName) {
		for (int i = 0, childs = root.getChildCount(); i < childs; i++) {
			AccessibilityNodeInfo child = root.getChild(i);
			if (child == null) {
				continue;
			}

			findNodesByClassName(list, child, clsName);

			if (clsName.equals(child.getClassName())) {
				list.add(child);
			} else {
				child.recycle();
			}
		}
	}

	public static List<AccessibilityNodeInfo> findNodesByClassName(AccessibilityNodeInfo root, String clsName) {
		List<AccessibilityNodeInfo> list = new ArrayList<AccessibilityNodeInfo>();
		findNodesByClassName(list, root, clsName);
		return list;
	}

	public static AccessibilityNodeInfo findNodeByClassName(AccessibilityNodeInfo root, String clsName) {
		for (int i = 0, childs = root.getChildCount(); i < childs; i++) {
			AccessibilityNodeInfo child = root.getChild(i);
			if (child != null) {
				if (clsName.equals(child.getClassName())) {
					return child;
				}

				AccessibilityNodeInfo node = findNodeByClassName(child, clsName);

				child.recycle();

				if (node != null) {
					return node;
				}
			}
		}

		return null;
	}

	public static AccessibilityNodeInfo getFirstNode(List<AccessibilityNodeInfo> nodes) {
		if (nodes == null) {
			return null;
		}

		int size = nodes.size();
		if (size > 0) {
			recycleNodes(nodes, 1, size);
			return nodes.get(0);
		}

		return null;
	}

	public static AccessibilityNodeInfo findNodeByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = findNodesByText(root, text);
		return getFirstNode(nodes);
	}

	public static AccessibilityNodeInfo findNodeByViewId(AccessibilityNodeInfo root, String viewId) {
		if (CavanAndroid.SDK_VERSION < 18) {
			return null;
		}

		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		return getFirstNode(nodes);
	}

	public static String getNodeText(AccessibilityNodeInfo node) {
		CharSequence text = node.getText();
		if (text != null) {
			return text.toString().trim();
		}

		return CavanString.EMPTY_STRING;
	}

	public static boolean isTextEquals(AccessibilityNodeInfo node, String text) {
		return getNodeText(node).equals(text);
	}

	public static String getNodeDescription(AccessibilityNodeInfo node) {
		CharSequence description = node.getContentDescription();
		if (description != null) {
			return description.toString().trim();
		}

		return CavanString.EMPTY_STRING;
	}

	public static boolean isDescriptionEquals(AccessibilityNodeInfo node, String text) {
		return getNodeDescription(node).equals(text);
	}

	public static String getEventText(AccessibilityEvent event) {
		List<CharSequence> texts = event.getText();
		if (texts.isEmpty()) {
			return CavanString.EMPTY_STRING;
		}

		return texts.get(0).toString();
	}

	public static String getNodeViewId(AccessibilityNodeInfo node) {
		if (CavanAndroid.SDK_VERSION < 18) {
			return null;
		}

		CharSequence text = node.getViewIdResourceName();
		if (text != null) {
			return text.toString();
		}

		return null;
	}

	public static String getChildText(AccessibilityNodeInfo parent, int index) {
		AccessibilityNodeInfo child = getChild(parent, index);
		if (child == null) {
			return null;
		}

		try {
			return getNodeText(child);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			child.recycle();
		}

		return null;
	}

	public static String getChildDesction(AccessibilityNodeInfo parent, int index) {
		AccessibilityNodeInfo child = getChild(parent, index);
		if (child == null) {
			return null;
		}

		try {
			return getNodeDescription(child);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			child.recycle();
		}

		return null;
	}

	public static String getChildViewId(AccessibilityNodeInfo parent, int index) {
		try {
			AccessibilityNodeInfo child = parent.getChild(index);
			String text = getNodeViewId(child);
			child.recycle();
			return text;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static String getNodeTextByViewId(AccessibilityNodeInfo root, String viewId) {
		AccessibilityNodeInfo node = findNodeByViewId(root, viewId);
		if (node == null) {
			return null;
		}

		String text = getNodeText(node);
		node.recycle();
		return text;
	}

	public static int getNodeCountAndRecycle(List<AccessibilityNodeInfo> nodes) {
		if (nodes == null) {
			return 0;
		}

		recycleNodes(nodes);

		return nodes.size();
	}

	public static int getNodeCountByViewIds(AccessibilityNodeInfo root, String... ids) {
		if (CavanAndroid.SDK_VERSION < 18) {
			return 0;
		}

		int count = 0;

		for (String id : ids) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(id);

			if (nodes != null) {
				count += getNodeCountAndRecycle(nodes);
			}
		}

		return count;
	}

	public static int getNodeCountByTexts(AccessibilityNodeInfo root, String... texts) {
		int count = 0;

		for (String text : texts) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);

			if (nodes != null) {
				count += getNodeCountAndRecycle(nodes);
			}
		}

		return count;
	}

	public static int getNodeCountByTextsWhole(AccessibilityNodeInfo root, String... texts) {
		int count = 0;

		for (String text : texts) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
			if (nodes == null) {
				continue;
			}

			for (AccessibilityNodeInfo node : nodes) {
				if (text.equals(node.getText())) {
					count++;
				}
			}

			recycleNodes(nodes);
		}

		return count;
	}

	public static int getChildCountByViewId(AccessibilityNodeInfo parent, String viewId) {
		int count = 0;

		for (int i = 0, childs = parent.getChildCount(); i < childs; i++) {
			if (viewId.equals(getChildViewId(parent, i))) {
				count++;
			}
		}

		return count;
	}

	public static int getChildCountByText(AccessibilityNodeInfo parent, String text) {
		int count = 0;

		for (int i = 0, childs = parent.getChildCount(); i < childs; i++) {
			if (text.equals(getChildText(parent, i))) {
				count++;
			}
		}

		return count;
	}

	public static boolean containsTexts(AccessibilityNodeInfo root, String... texts) {
		return getNodeCountByTexts(root, texts) > 0;
	}

	public static boolean containsTextsWhole(AccessibilityNodeInfo root, String... texts) {
		return getNodeCountByTextsWhole(root, texts) > 0;
	}

	public static boolean containsViewIds(AccessibilityNodeInfo root, String... ids) {
		return getNodeCountByViewIds(root, ids) > 0;
	}

	public static int traverseNodes(AccessibilityNodeInfo root, Closure closure) {
		int count;

		if ((Boolean) closure.call(root)) {
			count = 1;
		} else {
			count = 0;
		}

		for (int i = 0, childs = root.getChildCount(); i < childs; i++) {
			AccessibilityNodeInfo child = root.getChild(i);
			if (child == null) {
				continue;
			}

			count += traverseNodes(root, closure);
			child.recycle();
		}

		return count;
	}

	public static int traverseChilds(AccessibilityNodeInfo parent, Closure closure) {
		int count = 0;

		for (int i = 0, childs = parent.getChildCount(); i < childs; i++) {
			AccessibilityNodeInfo child = parent.getChild(i);
			if (child == null) {
				continue;
			}

			if ((Boolean) closure.call(child)) {
				count++;
			}

			child.recycle();
		}

		return count;
	}

	public static int traverseChildsByViewId(AccessibilityNodeInfo root, String viewId, Closure closure) {
		if (CavanAndroid.SDK_VERSION < 18) {
			return 0;
		}

		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		if (nodes == null) {
			return 0;
		}

		int count = 0;

		for (AccessibilityNodeInfo node : nodes) {
			count += traverseChilds(node, closure);
			node.recycle();
		}

		return count;
	}

	public static int traverseChildsByText(AccessibilityNodeInfo root, String text, Closure closure) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return 0;
		}

		int count = 0;

		for (AccessibilityNodeInfo node : nodes) {
			if (text.equals(getNodeText(node))) {
				count += traverseChilds(node, closure);
			}

			node.recycle();
		}

		return count;
	}

	public static int traverseNodesByViewIds(AccessibilityNodeInfo root, Closure closure, String... ids) {
		if (CavanAndroid.SDK_VERSION < 18) {
			return 0;
		}

		int count = 0;

		for (String id : ids) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(id);
			if (nodes == null) {
				continue;
			}

			for (AccessibilityNodeInfo node : nodes) {
				if ((Boolean) closure.call(node)) {
					count++;
				}

				node.recycle();
			}
		}

		return count;
	}

	public static int traverseNodesByTexts(AccessibilityNodeInfo root, Closure closure, String... texts) {
		int count = 0;

		for (String text : texts) {
			List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
			if (nodes == null) {
				continue;
			}

			for (AccessibilityNodeInfo node : nodes) {
				if (text.equals(getNodeText(node)) && (Boolean) closure.call(node)) {
					count++;
				}

				node.recycle();
			}
		}

		return count;
	}

	public static boolean performClick(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
	}

	public static boolean performLongClick(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_LONG_CLICK);
	}

	public static boolean performFocus(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_FOCUS);
	}

	public static boolean performSelection(AccessibilityNodeInfo node, int start, int length) {
		if (length > 0) {
			if (CavanAndroid.SDK_VERSION < 18) {
				return false;
			}

			Bundle arguments = new Bundle();
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, start);
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, start + length);
			return node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
		} else {
			return node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		}
	}

	public static int performActionByViewIds(AccessibilityNodeInfo root, final int action, final Bundle arguments, String... ids) {
		return traverseNodesByViewIds(root, new Closure() {

			@Override
			public Object call(Object... args) {
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[0];
				return node.performAction(action, arguments);
			}
		}, ids);
	}

	public static int performClickByViewIds(AccessibilityNodeInfo root, String... ids) {
		return performActionByViewIds(root, AccessibilityNodeInfo.ACTION_CLICK, null, ids);
	}

	public static int performActionByTexts(AccessibilityNodeInfo root, final int action, final Bundle arguments, String... texts) {
		return traverseNodesByTexts(root, new Closure() {

			@Override
			public Object call(Object... args) {
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[0];
				return node.performAction(action, arguments);
			}
		}, texts);
	}

	public static String setNodeText(Context context, AccessibilityNodeInfo node, String text) {
		if (text == null) {
			text = CavanString.EMPTY_STRING;
		}

		performFocus(node);

		String old = getNodeText(node);
		if (text.equals(old)) {
			return old;
		}

		if (CavanAndroid.SDK_VERSION >= 21) {
			Bundle arguments = new Bundle(1);
			arguments.putCharSequence(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, text);

			if (node.performAction(AccessibilityNodeInfo.ACTION_SET_TEXT, arguments)) {
				return old;
			}
		}

		if (context != null) {
			performSelection(node, 0, old.length());
			CavanAndroid.postClipboardText(context, CavanAndroid.CLIP_LABEL_TEMP, text);

			if (node.performAction(AccessibilityNodeInfo.ACTION_PASTE)) {
				return old;
			}
		}

		return null;
	}

	public static String setNodeTextAndRecycle(Context context, AccessibilityNodeInfo node, String text) {
		String old = setNodeText(context, node, text);
		node.recycle();
		return old;
	}

	public static String setChildText(Context context, AccessibilityNodeInfo parent, int index, String text) {
		try {
			AccessibilityNodeInfo child = parent.getChild(index);
			return setNodeTextAndRecycle(context, child, text);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	public static int setNodeTextByViewId(final Context context, AccessibilityNodeInfo root, String viewId, final String text) {
		return traverseNodesByViewIds(root, new Closure() {

			@Override
			public Object call(Object... args) {
				return setNodeText(context, (AccessibilityNodeInfo) args[0], text);
			}
		}, viewId);
	}

	public static boolean performActionAndRecycle(AccessibilityNodeInfo node, int action, Bundle arguments) {
		boolean success = node.performAction(action, arguments);
		node.recycle();
		return success;
	}

	public static boolean performActionAndRecycle(AccessibilityNodeInfo node, int action) {
		return performActionAndRecycle(node, action, null);
	}

	public static boolean performClickAndRecycle(AccessibilityNodeInfo node) {
		return performActionAndRecycle(node, AccessibilityNodeInfo.ACTION_CLICK);
	}

	public static boolean performLongClickAndRecycle(AccessibilityNodeInfo node) {
		return performActionAndRecycle(node, AccessibilityNodeInfo.ACTION_LONG_CLICK);
	}

	public static boolean performClickParent(AccessibilityNodeInfo node) {
		AccessibilityNodeInfo parent = node.getParent();
		if (parent != null) {
			return CavanAccessibilityHelper.performClickAndRecycle(parent);
		}

		return false;
	}

	public static boolean performClickParentAndRecycle(AccessibilityNodeInfo node) {
		try {
			return performClickParent(node);
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			node.recycle();
		}

		return false;
	}

	public static boolean performChildAction(AccessibilityNodeInfo parent, int index, int action) {
		try {
			AccessibilityNodeInfo child = parent.getChild(index);
			return performActionAndRecycle(child, action, null);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean performChildClick(AccessibilityNodeInfo parent, int index) {
		return performChildAction(parent, index, AccessibilityNodeInfo.ACTION_CLICK);
	}

	@SuppressWarnings("deprecation")
	public static boolean hasAction(AccessibilityNodeInfo node, AccessibilityAction action) {
		if (CavanAndroid.SDK_VERSION < 21) {
			return (node.getActions() & action.getId()) != 0;
		} else {
			return node.getActionList().contains(action);
		}
	}

	@SuppressWarnings("deprecation")
	public static boolean hasAction(AccessibilityNodeInfo node, int value) {
		if (CavanAndroid.SDK_VERSION < 21) {
			return (node.getActions() & value) != 0;
		} else {
			for (AccessibilityAction action : node.getActionList()) {
				if (action.getId() == value) {
					return true;
				}
			}
		}

		return false;
	}

	public static boolean performAction(AccessibilityNodeInfo node, int action, Bundle arguments, int count) {
		boolean success = false;

		if (hasAction(node, action)) {
			while (count > 0 && node.performAction(action, arguments)) {
				CavanAndroid.dLog("action" + count + " = " + action);

				CavanJava.msleep(100);
				success = true;
				count--;
			}
		}

		return success;
	}

	public static boolean performScrollUp(AccessibilityNodeInfo node, int count) {
		return performAction(node, AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD, null, count);
	}

	public static boolean performScrollDown(AccessibilityNodeInfo node, int count) {
		return performAction(node, AccessibilityNodeInfo.ACTION_SCROLL_FORWARD, null, count);
	}

	public static boolean performScrollUp(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD);
	}

	public static boolean performScrollDown(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_SCROLL_FORWARD);
	}

	private static void dumpNodePrivate(StringBuilder builder, String prefix, AccessibilityNodeInfo node, ClosureVoid closure) {
		closure.call(builder, node);
		builder.append('\n');

		int childs = node.getChildCount();
		if (childs > 0) {
			String subPrefix = prefix + "─┼─";

			for (int i = 0; i < childs; i++) {
				AccessibilityNodeInfo child = node.getChild(i);
				if (child == null) {
					continue;
				}

				builder.append(prefix).append(' ').append(i).append(". ");
				dumpNodePrivate(builder, subPrefix, child, closure);
				child.recycle();
			}
		}
	}

	public static void dumpNodeTo(StringBuilder builder, AccessibilityNodeInfo node, ClosureVoid closure) {
		if (builder != null && node != null && closure != null) {
			dumpNodePrivate(builder, "├─", node, closure);
		}
	}

	public static String dumpNodeTo(AccessibilityNodeInfo node, ClosureVoid closure) {
		StringBuilder builder = new StringBuilder();
		dumpNodeTo(builder, node, closure);
		return builder.toString();
	}

	public static void dumpNode(AccessibilityNodeInfo node, ClosureVoid closure) {
		CavanAndroid.dLogLarge(dumpNodeTo(node, closure));
	}

	public static class ClosureDumpNode implements ClosureVoid {

		@Override
		public void call(Object... args) {
			StringBuilder builder = (StringBuilder) args[0];
			AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[1];

			builder.append(node);
		}
	}

	public static void dumpNode(AccessibilityNodeInfo node) {
		dumpNode(node, new ClosureDumpNode());
	}

	public static class ClosureDumpNodeSimple implements ClosureVoid {

		@Override
		public void call(Object... args) {
			StringBuilder builder = (StringBuilder) args[0];
			AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[1];

			builder.append(node.getClassName());
			builder.append('[').append(Integer.toHexString(node.hashCode())).append(']');
			builder.append("@");

			if (CavanAndroid.SDK_VERSION >= 18) {
				builder.append(node.getViewIdResourceName());
			}

			builder.append(": ");
			builder.append(node.getText());
			builder.append('@').append(node.getContentDescription());
		}
	}

	public static void dumpNodeSimple(AccessibilityNodeInfo node) {
		dumpNode(node, new ClosureDumpNodeSimple());
	}

	public static void dumpNode(AccessibilityNodeInfo node, boolean simple) {
		if (simple) {
			dumpNodeSimple(node);
		} else {
			dumpNode(node);
		}
	}

	public static void dumpNode(AccessibilityNodeInfo node, int enable) {
		if (enable > 0) {
			if (enable > 1) {
				dumpNode(node);
			} else {
				dumpNodeSimple(node);
			}
		}
	}

	public static int dumpNode(AccessibilityNodeInfo node, String prop) {
		int enable = SystemProperties.getInt(prop, 0);
		dumpNode(node, enable);
		return enable;
	}

	public static void dumpEvent(AccessibilityEvent event) {
		CavanAndroid.dLog("event = " + event);

		AccessibilityNodeInfo source = event.getSource();
		if (source != null) {
			CavanAndroid.dLog("source = " + source);
			source.recycle();
		}
	}

	public static boolean dumpEvent(AccessibilityEvent event, String prop) {
		if (SystemProperties.getBoolean(prop, false)) {
			dumpEvent(event);
			return true;
		}

		return false;
	}

	public static Rect getBoundsInParent(AccessibilityNodeInfo node) {
		Rect bounds = new Rect();
		node.getBoundsInParent(bounds);
		return bounds;
	}

	public static Rect getBoundsInScreen(AccessibilityNodeInfo node) {
		Rect bounds = new Rect();
		node.getBoundsInScreen(bounds);
		return bounds;
	}

	public static boolean performGlobalAction(AccessibilityService service, int action) {
		CavanAndroid.dLog("performGlobalAction: " + action);
		CavanAndroid.dumpstack();
		return service.performGlobalAction(action);
	}

	public static boolean performGlobalBack(AccessibilityService service) {
		return performGlobalAction(service, AccessibilityService.GLOBAL_ACTION_BACK);
	}

	public static AccessibilityNodeInfo[] getChildsRaw(AccessibilityNodeInfo node, int index, int count) {
		AccessibilityNodeInfo[] childs = new AccessibilityNodeInfo[count];

		for (int i = 0; i < count; i++, index++) {
			try {
				AccessibilityNodeInfo child = node.getChild(index);
				if (child == null) {
					return null;
				}

				childs[i] = child;
			} catch (Exception e) {
				e.printStackTrace();
				return null;
			}
		}

		return childs;
	}

	public static AccessibilityNodeInfo[] getChilds(AccessibilityNodeInfo node, int index, int count) {
		if (index + count > node.getChildCount()) {
			return null;
		}

		return getChildsRaw(node, index, count);
	}

	public static AccessibilityNodeInfo[] getChilds(AccessibilityNodeInfo node, int count) {
		return getChilds(node, 0, count);
	}

	public static AccessibilityNodeInfo[] getChilds(AccessibilityNodeInfo node) {
		return getChildsRaw(node, 0, node.getChildCount());
	}

	public static AccessibilityNodeInfo getChild(AccessibilityNodeInfo node, int index) {
		int count = node.getChildCount();
		if (index < count) {
			if (index < 0) {
				index += count;
				if (index < 0) {
					return null;
				}
			}

			try {
				return node.getChild(index);
			} catch (Exception e) {
				return null;
			}
		}

		return null;
	}

	public static AccessibilityNodeInfo getChildRecursive(AccessibilityNodeInfo node, int... indexs) {
		if (indexs.length > 0) {
			node = getChild(node, indexs[0]);

			for (int i = 1; i < indexs.length; i++) {
				if (node == null) {
					break;
				}

				AccessibilityNodeInfo child = getChild(node, indexs[i]);
				node.recycle();
				node = child;
			}
		}

		return node;
	}

	public static AccessibilityNodeInfo[] getChildsRecursive(AccessibilityNodeInfo node, int... indexs) {
		AccessibilityNodeInfo[] childs = new AccessibilityNodeInfo[indexs.length];

		for (int i = 0; i < childs.length; i++) {
			AccessibilityNodeInfo child = getChild(node, indexs[i]);
			if (child == null) {
				while (--i > 0) {
					childs[i].recycle();
				}

				return null;
			}

			childs[i] = child;
			node = child;
		}

		return childs;
	}

	public static List<CharSequence> getChildTexts(AccessibilityNodeInfo parent) {
		int count = parent.getChildCount();
		List<CharSequence> texts = new ArrayList<CharSequence>();

		try {
			for (int i = 0; i < count; i++) {
				AccessibilityNodeInfo node = parent.getChild(i);
				if (node != null) {
					texts.add(node.getText());
					node.recycle();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return texts;
	}

	public static List<AccessibilityNodeInfo> getChildsByClassName(AccessibilityNodeInfo parent, CharSequence clsName) {
		List<AccessibilityNodeInfo> childs = new ArrayList<AccessibilityNodeInfo>();

		try {
			int count = parent.getChildCount();

			for (int i = 0; i < count; i++) {
				AccessibilityNodeInfo child = parent.getChild(i);
				if (child.getClassName().equals(clsName)) {
					childs.add(child);
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return childs;
	}

	public static List<AccessibilityNodeInfo> getChildsByClassName(AccessibilityNodeInfo root, CharSequence clsName, int... indexs) {
		AccessibilityNodeInfo parent = getChildRecursive(root, indexs);
		if (parent == null) {
			return null;
		}

		List<AccessibilityNodeInfo> childs = getChildsByClassName(parent, clsName);

		if (indexs.length > 0) {
			parent.recycle();
		}

		return childs;
	}

	public static String getNodePackageName(AccessibilityNodeInfo node) {
		return CavanString.fromCharSequence(node.getPackageName(), null);
	}

	public static boolean isNodePackgeEquals(AccessibilityNodeInfo node, String pkg) {
		CharSequence name = node.getPackageName();
		if (name == null) {
			return false;
		}

		return name.toString().equals(pkg);
	}

	public static boolean isNodeClassEquals(AccessibilityNodeInfo node, String cls) {
		CharSequence name = node.getClassName();
		if (name == null) {
			return false;
		}

		return name.toString().equals(cls);
	}

	public static boolean isNodeClassEquals(AccessibilityNodeInfo node, Class<?> cls) {
		return isNodeClassEquals(node, cls.getName());
	}
}
