package com.cavan.android;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.content.Context;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeInfo.AccessibilityAction;
import android.widget.AbsListView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TabHost;
import android.widget.TextView;

import com.cavan.java.CavanJava;
import com.cavan.java.CavanJava.Closure;
import com.cavan.java.CavanJava.ClosureVoid;

public class CavanAccessibility {

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

			if (text.equals(getNodeText(node))) {
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
		for (int i = root.getChildCount() - 1; i >= 0; i--) {
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

	public static List<AccessibilityNodeInfo> findNodesByClassName(AccessibilityNodeInfo node, String clsName) {
		List<AccessibilityNodeInfo> list = new ArrayList<AccessibilityNodeInfo>();
		findNodesByClassName(list, node, clsName);
		return list;
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

	public static String getNodeViewId(AccessibilityNodeInfo node) {
		CharSequence text = node.getViewIdResourceName();
		if (text != null) {
			return text.toString();
		}

		return null;
	}

	public static String getChildText(AccessibilityNodeInfo parent, int index) {
		try {
			AccessibilityNodeInfo child = parent.getChild(index);
			String text = getNodeText(child);
			child.recycle();
			return text;
		} catch (Exception e) {
			e.printStackTrace();
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

	public static int getNodeCountByViewId(AccessibilityNodeInfo root, String viewId) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		return getNodeCountAndRecycle(nodes);
	}

	public static int getNodeCountByText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		return getNodeCountAndRecycle(nodes);
	}

	public static int getChildCountByViewId(AccessibilityNodeInfo parent, String viewId) {
		int count = 0;

		for (int i = parent.getChildCount() - 1; i >= 0; i--) {
			if (viewId.equals(getChildViewId(parent, i))) {
				count++;
			}
		}

		return count;
	}

	public static int getChildCountByText(AccessibilityNodeInfo parent, String text) {
		int count = 0;

		for (int i = parent.getChildCount() - 1; i >= 0; i--) {
			if (text.equals(getChildText(parent, i))) {
				count++;
			}
		}

		return count;
	}

	public static boolean containsText(AccessibilityNodeInfo root, String text) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null || nodes.isEmpty()) {
			return false;
		}

		recycleNodes(nodes);

		return true;
	}

	public static boolean containsViewId(AccessibilityNodeInfo root, String viewId) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		if (nodes == null || nodes.isEmpty()) {
			return false;
		}

		recycleNodes(nodes);

		return true;
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

	public static int traverseNodesByViewId(AccessibilityNodeInfo root, String viewId, Closure closure) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByViewId(viewId);
		if (nodes == null) {
			return 0;
		}

		int count = 0;

		for (AccessibilityNodeInfo node : nodes) {
			if ((Boolean) closure.call(node)) {
				count++;
			}

			node.recycle();
		}

		return count;
	}

	public static int traverseNodesByText(AccessibilityNodeInfo root, String text, Closure closure) {
		List<AccessibilityNodeInfo> nodes = root.findAccessibilityNodeInfosByText(text);
		if (nodes == null) {
			return 0;
		}

		int count = 0;

		for (AccessibilityNodeInfo node : nodes) {
			if (text.equals(getNodeText(node)) && (Boolean) closure.call(node)) {
				count++;
			}

			node.recycle();
		}

		return count;
	}

	public static boolean performClick(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_CLICK);
	}

	public static boolean performFocus(AccessibilityNodeInfo node) {
		return node.performAction(AccessibilityNodeInfo.ACTION_FOCUS);
	}

	public static boolean performSelection(AccessibilityNodeInfo node, int start, int length) {
		if (length > 0) {
			Bundle arguments = new Bundle();
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_START_INT, start);
			arguments.putInt(AccessibilityNodeInfo.ACTION_ARGUMENT_SELECTION_END_INT, start + length);
			return node.performAction(AccessibilityNodeInfo.ACTION_SET_SELECTION, arguments);
		} else {
			return node.performAction(AccessibilityNodeInfo.ACTION_SELECT);
		}
	}

	public static int performActionByViewId(AccessibilityNodeInfo root, String viewId, final int action, final Bundle arguments) {
		return traverseNodesByViewId(root, viewId, new Closure() {

			@Override
			public Object call(Object... args) {
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[0];
				return node.performAction(action, arguments);
			}
		});
	}

	public static int performActionByText(AccessibilityNodeInfo root, String text, final int action, final Bundle arguments) {
		return traverseNodesByText(root, text, new Closure() {

			@Override
			public Object call(Object... args) {
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[0];
				return node.performAction(action, arguments);
			}
		});
	}

	public static boolean setNodeText(Context context, AccessibilityNodeInfo node, String text) {
		performFocus(node);

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

		performSelection(node, 0, oldText.length());
		CavanAndroid.postClipboardText(context, CavanAndroid.CLIP_LABEL_SKIP, text);

		return node.performAction(AccessibilityNodeInfo.ACTION_PASTE);
	}

	public static boolean setNodeTextAndRecycle(Context context, AccessibilityNodeInfo node, String text) {
		boolean success = setNodeText(context, node, text);
		node.recycle();
		return success;
	}

	public static boolean setChildText(Context context, AccessibilityNodeInfo parent, int index, String text) {
		try {
			AccessibilityNodeInfo child = parent.getChild(index);
			return setNodeTextAndRecycle(context, child, text);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static int setNodeTextByViewId(final Context context, AccessibilityNodeInfo root, String viewId, final String text) {
		return traverseNodesByViewId(root, viewId, new Closure() {

			@Override
			public Object call(Object... args) {
				return setNodeText(context, (AccessibilityNodeInfo) args[0], text);
			}
		});
	}

	public static boolean performActionAndRecycle(AccessibilityNodeInfo node, int action, Bundle arguments) {
		boolean success = node.performAction(action, arguments);
		node.recycle();
		return success;
	}

	public static boolean performClickAndRecycle(AccessibilityNodeInfo node) {
		return performActionAndRecycle(node, AccessibilityNodeInfo.ACTION_CLICK, null);
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

	public static boolean hasAction(AccessibilityNodeInfo node, AccessibilityAction action) {
		return node.getActionList().contains(action);
	}

	public static boolean hasAction(AccessibilityNodeInfo node, int value) {
		for (AccessibilityAction action : node.getActionList()) {
			if (action.getId() == value) {
				return true;
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
		CavanAndroid.dLog(dumpNodeTo(node, closure));
	}

	public static void dumpNode(AccessibilityNodeInfo node) {
		dumpNode(node, new ClosureVoid() {

			@Override
			public void call(Object... args) {
				StringBuilder builder = (StringBuilder) args[0];
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[1];

				builder.append(node);
			}
		});
	}

	public static void dumpNodeSimple(AccessibilityNodeInfo node) {
		dumpNode(node, new ClosureVoid() {

			@Override
			public void call(Object... args) {
				StringBuilder builder = (StringBuilder) args[0];
				AccessibilityNodeInfo node = (AccessibilityNodeInfo) args[1];

				builder.append(node.getClassName());
				builder.append('[').append(Integer.toHexString(node.hashCode())).append(']');
				builder.append("@");
				builder.append(node.getViewIdResourceName());
				builder.append(": ");
				builder.append(node.getText());
			}
		});
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
}
