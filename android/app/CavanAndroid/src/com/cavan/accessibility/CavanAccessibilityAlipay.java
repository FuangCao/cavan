package com.cavan.accessibility;

import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanPackageName;

public class CavanAccessibilityAlipay extends CavanAccessibilityPackage {

	private static final String[] BACK_VIEW_IDS = {
		"com.alipay.mobile.ui:id/title_bar_back_button",
		"com.alipay.android.phone.discovery.envelope:id/coupon_chai_close",
		"com.alipay.mobile.nebula:id/h5_tv_nav_back",
	};

	private static final String[] OVER_VIEW_IDS = {
		"com.alipay.android.phone.discovery.envelope:id/coupon_action",
		"com.alipay.android.phone.discovery.envelope:id/sns_coupon_detail_action",
	};

	public static CavanAccessibilityAlipay instance;

	private CavanRedPacketAlipay mPacket;

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
		}

		public boolean inputRedPacketCode(AccessibilityNodeInfo input, String code) {
			String text = CavanAccessibilityHelper.setNodeText(mService, input, code);
			if (text != null) {
				return true;
			}

			CavanInputMethodService ime = CavanInputMethodService.instance;
			if (ime == null) {
				return false;
			}

			return ime.inputAlipayCode(code);
		}

		public boolean commitRedPacketCode() {
			return false;
		}

		public boolean postRedPacketCode(AccessibilityNodeInfo root, CavanRedPacketAlipay packet) {
			AccessibilityNodeInfo input = CavanAccessibilityHelper.findNodeByViewId(root, "com.alipay.android.phone.discovery.envelope:id/solitaire_edit");
			if (input == null) {
				return false;
			}

			addRecycleNode(input);

			if (mPacket != null) {
				mPacket.setPostPending(false);
			}

			if (!inputRedPacketCode(input, packet.getCode())) {
				return false;
			}

			packet.setPostPending(true);
			mPacket = packet;

			return true;
		}

		@Override
		public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
			if (mPacket != null && mPacket.isMaybeInvalid()) {
				if (getTimeConsume() > 1000) {
					setPacketInvalid(mPacket);
				}

				return true;
			}

			return postRedPacketCode(root, (CavanRedPacketAlipay) packet);
		}
	}

	public class ProgressDialog extends BaseWindow {

		public ProgressDialog(String name) {
			super(name);
		}
	}

	public class CouponDetailActivity extends BaseWindow {

		public CouponDetailActivity(String name) {
			super(name);
		}
	}

	public class ReceivedDetailActivity extends BaseWindow {

		public ReceivedDetailActivity(String name) {
			super(name);
		}
	}

	public class H5Activity extends BaseWindow {

		public H5Activity(String name) {
			super(name);
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

	public boolean addPacket(String code) {
		return addPacket(new CavanRedPacketAlipay(code));
	}

	public synchronized void setPacketInvalid(CavanRedPacketAlipay packet) {
		packet.setInvalid(true);
		removePacket(packet);
		mPacket = null;
	}

	public synchronized void setPacketCompleted(CavanRedPacketAlipay packet) {
		packet.setCompleted(true);
		removePacket(packet);
		mPacket = null;
	}

	@Override
	protected void initWindows() {
		addWindow(new LoginActivity("com.eg.android.AlipayGphone.AlipayLogin"));
		addWindow(new HomeActivity("com.alipay.android.phone.discovery.envelope.HomeActivity"));
		addWindow(new ProgressDialog("com.alipay.mobile.framework.app.ui.DialogHelper$APGenericProgressDialog"));
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
