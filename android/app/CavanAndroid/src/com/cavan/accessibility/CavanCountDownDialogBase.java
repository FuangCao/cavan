package com.cavan.accessibility;


public abstract class CavanCountDownDialogBase {

	public CavanRedPacket mPacket;

	public synchronized CavanRedPacket getPacket() {
		return mPacket;
	}

	public synchronized void setPacket(CavanRedPacket packet) {
		mPacket = packet;
	}

	public synchronized long update() {
		CavanRedPacket packet = mPacket;
		if (packet != null && packet.isPending()) {
			long remain = packet.getUnpackDelay(0);
			if (remain > 0) {
				onUpdated(remain);

				if (remain > 1000) {
					remain = 1000;
				}

				return remain;
			}

			onCompleted();
		} else {
			dismiss();
		}

		return 0;
	}

	public synchronized void show(CavanRedPacket packet) {
		mPacket = packet;
	}

	public synchronized boolean isShowing() {
		return (mPacket != null);
	}

	public synchronized void dismiss() {
		mPacket = null;
	}

	public void onCanceled() {
		CavanAccessibilityPackage pkg = mPacket.getPackage();
		pkg.setForceUnpackEnable(false);
		pkg.removePacket(mPacket);
		pkg.setPending(false);
		mPacket.setGotoIdle(false);
		mPacket.setCompleted();
		dismiss();
	}

	public void onCompleted() {
		mPacket.setGotoIdle(false);
		mPacket.setUnpackTime(0);
		dismiss();
	}

	protected void onUpdated(long remain) {}
}
