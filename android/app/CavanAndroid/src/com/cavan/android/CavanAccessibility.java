package com.cavan.android;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class CavanAccessibility {

	public static final String CLASS_VIEW = View.class.getName();
	public static final String CLASS_BUTTON = Button.class.getName();
	public static final String CLASS_TEXTVIEW = TextView.class.getName();
	public static final String CLASS_EDITTEXT = EditText.class.getName();

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
		return isInstanceOf(node, CLASS_TEXTVIEW);
	}

	public static boolean isEditText(AccessibilityNodeInfo node) {
		return isInstanceOf(node, CLASS_EDITTEXT);
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes, int start, int end) {
		for (int i = end - 1; i >= start; i--) {
			nodes.get(i).recycle();
		}
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes, int start) {
		recycleNodes(nodes, start, nodes.size());
	}

	public static void recycleNodes(List<AccessibilityNodeInfo> nodes) {
		recycleNodes(nodes, 0);
	}

	public static List<AccessibilityNodeInfo> findNodesByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return null;
		}

		Iterator<AccessibilityNodeInfo> iterator = nodes.iterator();

		while (iterator.hasNext()) {
			AccessibilityNodeInfo node = iterator.next();
			CharSequence sequence = node.getText();

			if (sequence != null && text.equals(sequence.toString())) {
				continue;
			}

			iterator.remove();
			node.recycle();
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
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		return getFirstNode(nodes);
	}

	public static String getNodeText(AccessibilityNodeInfo node) {
		CharSequence text = node.getText();
		if (text != null) {
			return text.toString();
		}

		return null;
	}

	public static String getChildNodeText(AccessibilityNodeInfo node, int index) {
		try {
			AccessibilityNodeInfo child = node.getChild(index);
			String text = getNodeText(child);
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

	public static void setNodeSelection(AccessibilityNodeInfo node, int start, int length) {
		if (length > 0) {
			Bundle arguments = new Bundle();
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, start);
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, start + length);
			node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
		} else {
			node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		}
	}

	public static boolean setNodeText(Context context, AccessibilityNodeInfo node, String text) {
		node.performAction(AccessibilityNodeInfo.ACTION_FOCUS);

		String oldText = getNodeText(node);
		if (text.equals(oldText)) {
			return false;
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			Bundle arguments = new Bundle(1);
			arguments.putCharSequence(AccessibilityNodeInfo.ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, text);

			if (node.performAction(AccessibilityNodeInfo.ACTION_SET_TEXT, arguments)) {
				return true;
			}
		}

		if (context == null) {
			return false;
		}

		setNodeSelection(node, 0, oldText.length());
		CavanAndroid.postClipboardText(context, CavanAndroid.CLIP_LABEL_SKIP, text);
		node.performAction(AccessibilityNodeInfo.ACTION_PASTE);

		return true;
	}

	public static boolean performChildAction(AccessibilityNodeInfo node, int index, int action) {
		AccessibilityNodeInfo child = node.getChild(index);
		boolean success = child.performAction(action);
		child.recycle();
		return success;
	}

	public static boolean performChildActionClick(AccessibilityNodeInfo node, int index) {
		AccessibilityNodeInfo child = node.getChild(index);
		boolean success = child.performAction(AccessibilityNodeInfo.ACTION_CLICK);
		child.recycle();
		return success;
	}

	public static void dumpNode(StringBuilder builder, String prefix, AccessibilityNodeInfo node) {
		if (node == null) {
			return;
		}

		builder.append(prefix);
		builder.append(node);
		builder.append('\n');
		prefix += "  ";

		for (int i = 0, count = node.getChildCount(); i < count; i++) {
			dumpNode(builder, prefix, node.getChild(i));
		}
	}

	public static void dumpNode(AccessibilityNodeInfo node) {
		StringBuilder builder = new StringBuilder();
		dumpNode(builder, "", node);
		CavanAndroid.dLog(builder.toString());
	}

	public static void dumpNodeSimple(StringBuilder builder, String prefix, AccessibilityNodeInfo node) {
		if (node == null) {
			return;
		}

		builder.append(prefix);
		builder.append("├─ ");
		builder.append(node.getClassName());
		builder.append("@");
		builder.append(node.getViewIdResourceName());
		// builder.append(node.hashCode());
		builder.append(": ");
		builder.append(node.getText());
		builder.append('\n');
		prefix += "├──";

		for (int i = 0, count = node.getChildCount(); i < count; i++) {
			dumpNodeSimple(builder, prefix, node.getChild(i));
		}
	}

	public static void dumpNodeSimple(AccessibilityNodeInfo node) {
		StringBuilder builder = new StringBuilder();
		dumpNodeSimple(builder, "", node);
		CavanAndroid.dLog(builder.toString());
	}
}
