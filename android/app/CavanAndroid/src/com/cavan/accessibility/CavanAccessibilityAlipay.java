package com.cavan.accessibility;

import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanAccessibilityAlipay extends CavanAccessibilityPackage {

	private static final String[] BACK_VIEW_IDS = {
		"com.alipay.mobile.ui:id/title_bar_back_button",
		"com.alipay.mobile.nebula:id/h5_tv_nav_back",
	};

	public static CavanAccessibilityAlipay instance;

	private CavanRedPacketAlipay mMaybeInvalid;
	private CavanRedPacketAlipay mInputPacket;
	private String mInputCode;

	public class BaseWindow extends CavanAccessibilityWindow {

		public BaseWindow(String name) {
			super(name);
		}
	}

	public class LoginActivity extends BaseWindow {

		public LoginActivity(String name) {
			super(name);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, "红包");
			if (node != null) {
				AccessibilityNodeInfo parent = node.getParent();
				if (parent == null) {
					node.recycle();
					return false;
				}

				CavanAccessibilityHelper.performActionAndRecycle(node, AccessibilityNodeInfo.ACTION_SELECT);
				CavanAccessibilityHelper.performClickAndRecycle(parent);
				return true;
			}

			node = CavanAccessibilityHelper.findNodeByViewId(root, "com.alipay.android.phone.openplatform:id/tab_description");
			if (node == null) {
				node = CavanAccessibilityHelper.findNodeByText(root, "首页");
				if (node == null) {
					return false;
				}
			}

			AccessibilityNodeInfo parent = node.getParent();
			if (parent != null) {
				CavanAccessibilityHelper.performClickAndRecycle(parent);
			}

			node.recycle();
			return true;
		}
	}

	public class HomeActivity extends BaseWindow {

		public HomeActivity(String name) {
			super(name);
			setBackViewId("com.alipay.mobile.ui:id/title_bar_back_button");
		}

		public boolean inputRedPacketCode(AccessibilityNodeInfo input, String code) {
			String text = CavanAccessibilityHelper.setNodeText(mService, input, code);
			if (text != null) {
				return true;
			}

			CavanInputMethodService ime = mService.getInputMethodService();
			if (ime == null) {
				CavanAndroid.eLog("ime == null");
				return false;
			}

			return ime.inputAlipayCode(code);
		}

		public boolean commitRedPacketCode() {
			CavanInputMethodService ime = mService.getInputMethodService();
			if (ime == null) {
				CavanAndroid.eLog("ime == null");
				return false;
			}

			return ime.commitAlipayCode();
		}

		public AccessibilityNodeInfo findInputNode(AccessibilityNodeInfo root) {
			for (int i = 0; i < 10; i++) {
				AccessibilityNodeInfo node = root.findFocus(AccessibilityNodeInfo.FOCUS_INPUT);
				if (node != null) {
					if (CavanAccessibilityHelper.isEditText(node)) {
						return node;
					}

					node.recycle();
				}

				CavanInputMethodService ime = mService.getInputMethodService();
				if (ime == null) {
					CavanAndroid.eLog("ime == null");
					return null;
				}

				ime.sendKeyDownUp(KeyEvent.KEYCODE_DPAD_DOWN);
				CavanJava.msleep(50);
			}

			return null;
		}

		public boolean postRedPacketCode(AccessibilityNodeInfo root, CavanRedPacketAlipay packet) {
			CavanAndroid.dLog("post: " + packet);

			AccessibilityNodeInfo input = findInputNode(root); // CavanAccessibilityHelper.findNodeByViewId(root, "com.alipay.android.phone.discovery.envelope:id/solitaire_edit");
			if (input == null) {
				return false;
			}

			addRecycleNode(input);

			CavanRedPacketAlipay invalid = getMaybeInvalid();
			if (invalid != null && invalid.getCode().equals(CavanAccessibilityHelper.getNodeText(input))) {
				if (invalid.isInvalid()) {
					packet = invalid;
				} else if (getTimeConsume() > 1000) {
					invalid.setInvalid();
					packet = invalid;
				} else {
					return true;
				}
			}

			setMaybeInvalid(null);

			if (!packet.isPending()) {
				if (packet.isInvalid()) {
					return inputRedPacketCode(input, CavanString.EMPTY_STRING);
				}

				return true;
			}

			if (!inputRedPacketCode(input, packet.getCode())) {
				return false;
			}

			setInputPacket(packet);

			if (packet.getUnpackDelay(0) > 0) {
				showCountDownView(packet);
				return true;
			}

			dismissCountDownView();

			if (!commitRedPacketCode()) {
				return false;
			}

			setUnlockDelay(LOCK_DELAY);

			return true;
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			return postRedPacketCode(root, (CavanRedPacketAlipay) packet);
		}

		@Override
		public void onProgress(String name) {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.addPostTimes();
				setMaybeInvalid(packet);
			}
		}

		@Override
		public void onAndroidWidget(String name) {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.setInvalid();
			}
		}

		@Override
		public void onViewTextChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
			String text = CavanAccessibilityHelper.getEventText(event);
			setInputCode(text);
		}

		@Override
		public boolean isHomePage() {
			return true;
		}

		@Override
		public int getEventTypes(CavanAccessibilityPackage pkg) {
			return super.getEventTypes(pkg) | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED;
		}
	}

	public class CouponDetailActivity extends BaseWindow {

		public CouponDetailActivity(String name) {
			super(name);
			setBackViewId("com.alipay.android.phone.discovery.envelope:id/coupon_chai_close");
		}

		@Override
		public void onEnter(AccessibilityNodeInfo root) {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.addCommitTimes(false);
			}

			setMaybeInvalid(null);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.android.phone.discovery.envelope:id/action_chai") > 0) {
				setUnlockDelay(LOCK_DELAY);
				return true;
			}

			if (performActionBack(root)) {
				setUnlockDelay(LOCK_DELAY);
				setPacketCompleted();
				return true;
			}

			return false;
		}
	}

	public class ReceivedDetailActivity extends BaseWindow {

		public ReceivedDetailActivity(String name) {
			super(name);
			setBackViewId("com.alipay.mobile.ui:id/title_bar_back_button");
		}

		@Override
		public void onEnter(AccessibilityNodeInfo root) {
			setMaybeInvalid(null);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (performActionBack(root)) {
				setUnlockDelay(LOCK_DELAY);
				setPacketCompleted();
				return true;
			}

			return false;
		}
	}

	public class H5Activity extends BaseWindow {

		public H5Activity(String name) {
			super(name);
			setBackViewId("com.alipay.mobile.nebula:id/h5_tv_nav_back");
		}

		@Override
		public void onEnter(AccessibilityNodeInfo root) {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.addCommitTimes(true);
			}

			setMaybeInvalid(null);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (times < 3) {
				return true;
			}

			if (performActionBack(root)) {
				setUnlockDelay(LOCK_DELAY);
				return true;
			}

			return false;
		}
	}

	public class XiuxiuActivity extends BaseWindow {

		public XiuxiuActivity(String name) {
			super(name);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByViewId(root, "com.alipay.mobile.xiuxiu:id/button1");
			if (node == null) {
				return false;
			}

			return CavanAccessibilityHelper.performClickAndRecycle(node);
		}
	}

	public class UserLoginActivity extends BaseWindow {

		public UserLoginActivity(String name) {
			super(name);
		}
	}

	public class UpdateCommonDialog extends BaseWindow {

		public UpdateCommonDialog(String name) {
			super(name);
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByViewId(root, "com.alipay.mobile.accountauthbiz:id/update_cancel_tv");
			if (node != null) {
				return CavanAccessibilityHelper.performClickAndRecycle(node);
			}

			node = CavanAccessibilityHelper.findNodeByText(root, "稍后再说");
			if (node != null) {
				return CavanAccessibilityHelper.performClickAndRecycle(node);
			}

			return false;
		}
	}

	public CavanAccessibilityAlipay(CavanAccessibilityService service) {
		super(service, CavanPackageName.ALIPAY);
	}

	public synchronized void setInputCode(String code) {
		CavanAndroid.dLog("setInputCode: " + code);
		mInputCode = code;
	}

	public synchronized void setInputPacket(CavanRedPacketAlipay packet) {
		setInputCode(packet.getCode());
		mInputPacket = packet;
	}

	public synchronized CavanRedPacketAlipay getInputPacket(boolean create) {
		if (mInputPacket != null) {
			if (mInputPacket.getCode().equals(mInputCode)) {
				return mInputPacket;
			}

			mInputPacket = null;
		}

		if (mInputCode == null || mInputCode.isEmpty()) {
			return null;
		}

		if (mInputCode.equals("输口令 领红包")) {
			mInputCode = null;
			return null;
		}

		mInputPacket = CavanRedPacketAlipay.get(mInputCode, true);

		return mInputPacket;
	}

	public synchronized CavanRedPacketAlipay getMaybeInvalid() {
		return mMaybeInvalid;
	}

	public synchronized void setMaybeInvalid(CavanRedPacketAlipay packet) {
		mMaybeInvalid = packet;
	}

	public synchronized void setPacketCompleted() {
		CavanRedPacketAlipay packet = getInputPacket(false);
		if (packet != null) {
			packet.setCompleted();
		}
	}

	public boolean addPacket(String code) {
		return addPacket(CavanRedPacketAlipay.get(code, true, false));
	}

	@Override
	protected void initWindows() {
		addProgressWindow("com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog");
		addWindow(new LoginActivity("com.eg.android.AlipayGphone.AlipayLogin"));
		addWindow(new HomeActivity("com.alipay.android.phone.discovery.envelope.HomeActivity"));
		addWindow(new CouponDetailActivity("com.alipay.android.phone.discovery.envelope.get.GetRedEnvelopeActivity"));
		addWindow(new CouponDetailActivity("com.alipay.android.phone.discovery.envelope.get.SnsCouponDetailActivity"));
		addWindow(new ReceivedDetailActivity("com.alipay.android.phone.discovery.envelope.crowd.CrowdHostActivity"));
		addWindow(new ReceivedDetailActivity("com.alipay.android.phone.discovery.envelope.received.ReceivedDetailActivity"));
		addWindow(new H5Activity("com.alipay.mobile.nebulacore.ui.H5Activity"));
		addWindow(new H5Activity("com.alipay.mobile.nebulacore.ui.H5LoadingDialog"));
		addWindow(new UserLoginActivity("com.ali.user.mobile.login.ui.AliuserGuideActivity"));
		addWindow(new UserLoginActivity("com.alipay.mobile.commonui.widget.APNoticePopDialog"));
		addWindow(new UserLoginActivity("com.alipay.mobile.security.login.ui.AlipayUserLoginActivity"));
		addWindow(new XiuxiuActivity("com.alipay.mobile.xiuxiu.ui.RedPacketSettingsActivity"));
		addWindow(new UpdateCommonDialog("com.alipay.mobile.about.widget.UpdateCommonDialog"));
	}

	@Override
	public String[] getBackViewIds() {
		return BACK_VIEW_IDS;
	}

	@Override
	protected void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	protected void onDestroy() {
		instance = null;
		super.onDestroy();
	}
}
