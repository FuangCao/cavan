package com.cavan.accessibility;

import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class CavanAccessibilityAlipay extends CavanAccessibilityPackage {

	public static CavanAccessibilityAlipay instance;

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

		private boolean mMaybeInvalid;

		public HomeActivity(String name) {
			super(name);
		}

		public boolean inputRedPacketCode(AccessibilityNodeInfo input, String code) {
			String text = CavanAccessibilityHelper.setNodeText(mService, input, code);
			if (text != null) {
				return true;
			}

			CavanInputMethodService ime = CavanInputMethodService.instance;
			if (ime == null) {
				CavanAndroid.eLog("ime == null");
				return false;
			}

			return ime.inputAlipayCode(code);
		}

		public boolean commitRedPacketCode() {
			CavanInputMethodService ime = CavanInputMethodService.instance;
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

				CavanInputMethodService ime = CavanInputMethodService.instance;
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

			if (mMaybeInvalid) {
				CavanRedPacketAlipay node = getInputPacket(input);
				if (node == packet) {
					if (getTimeConsume() > 1000) {
						node.setInvalid();
					}

					return true;
				}

				mMaybeInvalid = false;
			}

			if (!packet.isPending()) {
				if (packet.isInvalid()) {
					return inputRedPacketCode(input, CavanString.EMPTY_STRING);
				}

				return true;
			}

			if (!inputRedPacketCode(input, packet.getCode())) {
				return false;
			}

			setInputCode(packet.getCode());

			if (packet.getUnpackDelay(0) > 0) {
				showCountDownView(packet);
				return true;
			}

			dismissCountDownView();

			if (!commitRedPacketCode()) {
				return false;
			}

			return true;
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			return postRedPacketCode(root, (CavanRedPacketAlipay) packet);
		}

		@Override
		public void onEnter() {
			setInputCode(null);
		}

		@Override
		public void onProgress(String name) {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				mMaybeInvalid = true;
				packet.addPostTimes();
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
		public void onViewTextChanged(AccessibilityEvent event) {
			try {
				mInputCode = event.getText().get(0).toString();
				CavanAndroid.dLog("mInputCode = " + mInputCode);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public boolean isMainActivity() {
			return true;
		}
	}

	public class CouponDetailActivity extends BaseWindow {

		public CouponDetailActivity(String name) {
			super(name);
		}

		@Override
		public void onEnter() {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.addCommitTimes(false);
			}
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.android.phone.discovery.envelope:id/action_chai") > 0) {
				return true;
			}

			if (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.android.phone.discovery.envelope:id/coupon_chai_close") > 0) {
				setPacketCompleted();
				return true;
			}

			return false;
		}

		@Override
		public boolean back(CavanAccessibilityPackage pkg, AccessibilityNodeInfo root) {
			if (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.android.phone.discovery.envelope:id/coupon_chai_close") > 0) {
				return true;
			}

			return super.back(pkg, root);
		}
	}

	public class ReceivedDetailActivity extends BaseWindow {

		public ReceivedDetailActivity(String name) {
			super(name);
		}

		@Override
		public void onEnter() {
			CavanRedPacketAlipay packet = getInputPacket(false);
			if (packet != null) {
				packet.setCompleted();
			}
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			return (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.mobile.ui:id/title_bar_back_button") > 0);
		}
	}

	public class H5Activity extends BaseWindow {

		public H5Activity(String name) {
			super(name);
		}

		@Override
		public void onEnter() {
			CavanRedPacketAlipay packet = getInputPacket(true);
			if (packet != null) {
				packet.addCommitTimes(true);
			}
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (times < 3) {
				return true;
			}

			return (CavanAccessibilityHelper.performClickByViewIds(root, "com.alipay.mobile.nebula:id/h5_tv_nav_back") > 0);
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
		instance = this;
	}

	public synchronized String getInputCode() {
		return mInputCode;
	}

	public synchronized void setInputCode(String code) {
		mInputCode = code;
	}

	public CavanRedPacketAlipay getInputPacket(AccessibilityNodeInfo input) {
		String text = CavanAccessibilityHelper.getNodeText(input);
		if (text == null || text.isEmpty()) {
			return null;
		}

		return CavanRedPacketAlipay.get(text, false);
	}

	public synchronized CavanRedPacketAlipay getInputPacket(boolean create) {
		if (mInputPacket == null && mInputCode != null && mInputCode.length() > 0) {
			mInputPacket = CavanRedPacketAlipay.get(mInputCode, create);
		}

		return mInputPacket;
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
}
