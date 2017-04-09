using System;
using System.IO;
using System.Diagnostics;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public partial class CavanMacAddressManager : Form {

        private CavanMacAddressAddDialog mAddressAllocDialog;
        private CavanMacAddressButton mButtonCurrent;
        private CavanMacAddressRange mAddressRange;
        private object mSenderLocked;

        public CavanMacAddressManager(CavanMacAddress address, UInt32 count) {
            InitializeComponent();
            saveFileDialogExport.InitialDirectory = Application.StartupPath;

            mAddressRange = new CavanMacAddressRange(new CavanMacAddress(address), count);
            addMacAddressItemEmpty();

            AddressStart = address;
            AddressCount = count;
            AddressEnd = address.getAddressEnd(count);
            AddressNext = address.getAddressNext(count);
        }

        public ContextMenuStrip getContextMenuStripAlloc() {
            return contextMenuStripAlloc;
        }

        public ContextMenuStrip getContextMenuStripFree() {
            return contextMenuStripFree;
        }

        public void removeMacAddressItem(CavanMacAddressItem item) {
            panelAddresses.Controls.Remove(item.AddressButton);
            listViewAddresses.Items.Remove(item);
        }

        public void joinMacAddressItems(ListView.ListViewItemCollection items, int index, int count) {
            int width = 0;
            UInt32 addresses = 0;

            while (true) {
                CavanMacAddressItem item = (CavanMacAddressItem)listViewAddresses.Items[index--];
                CavanMacAddressButton button = item.AddressButton;

                if (count-- > 1) {
                    width += button.Width + 1;
                    addresses += item.AddressCount;
                    removeMacAddressItem(item);
                } else {
                    button.Width += width;
                    item.AddressCount += addresses;
                    break;
                }
            }
        }

        public void joinMacAddressItems() {
            ListView.ListViewItemCollection items = listViewAddresses.Items;
            int count = 0;

            foreach (CavanMacAddressItem item in items) {
                if (item.AddressUsed) {
                    if (count > 1) {
                        joinMacAddressItems(items, item.Index - 1, count);
                    }

                    count = 0;
                } else {
                    count++;
                }
            }

            if (count > 1) {
                joinMacAddressItems(items, items.Count - 1, count);
            }
        }

        public void setCurrentMacAddressButton(CavanMacAddressButton button) {
            if (button != mButtonCurrent) {
                if (mButtonCurrent != null) {
                    mButtonCurrent.setHighLight(false);
                }

                mButtonCurrent = button;

                if (button != null) {
                    button.setHighLight(true);
                }

                updateContextMenuStrip();
            }
        }

        public void updateContextMenuStrip() {
            if (mButtonCurrent == null || mButtonCurrent.AddressCount == 0) {
                listViewAddresses.ContextMenuStrip = null;
            } else if (mButtonCurrent.AddressUsed) {
                listViewAddresses.ContextMenuStrip = contextMenuStripFree;
            } else {
                listViewAddresses.ContextMenuStrip = contextMenuStripAlloc;
            }
        }

        public void addMacAddressItem(int index, CavanMacAddressItem item) {
            CavanMacAddressButton button = item.AddressButton;
            button.GotFocus += buttonMacAddress_GotFocus;
            button.LostFocus += buttonMacAddress_LostFocus;
            button.MouseCaptureChanged += buttonMacAddress_MouseCaptureChanged;
            panelAddresses.Controls.Add(button);

            listViewAddresses.Items.Insert(index, item);
        }

        private void addMacAddressItemEmpty() {
            Rectangle bounds = panelAddresses.Bounds;
            bounds.X = bounds.Y = 0;

            UInt32 count = mAddressRange.AddressCount;
            CavanMacAddress address = new CavanMacAddress(mAddressRange.AddressStart);
            CavanMacAddressItem item = new CavanMacAddressItem(this, bounds, address, count);
            addMacAddressItem(0, item);
        }

        private void buttonMacAddress_GotFocus(object sender, EventArgs e) {
            setCurrentMacAddressButton((CavanMacAddressButton)sender);
        }

        private void buttonMacAddress_MouseCaptureChanged(object sender, EventArgs e) {
            CavanMacAddressButton button = (CavanMacAddressButton)sender;
            button.Focus();
        }

        private void buttonMacAddress_LostFocus(object sender, EventArgs e) {
            if (sender == mButtonCurrent) {
                setCurrentMacAddressButton(null);
            }
        }

        private void listViewAddresses_SelectedIndexChanged(object sender, EventArgs e) {
            ListView.SelectedListViewItemCollection items = listViewAddresses.SelectedItems;
            if (items != null && items.Count > 0) {
                CavanMacAddressItem item = (CavanMacAddressItem)items[0];
                setCurrentMacAddressButton(item.AddressButton);
            } else {
                setCurrentMacAddressButton(null);
            }
        }

        private void freeMacAddress() {
            if (mButtonCurrent != null) {
                mButtonCurrent.free();
            }
        }

        private void allocMacAddress() {
            if (mButtonCurrent != null) {
                UInt32 count = mButtonCurrent.AddressCount;
                if (count > 0) {
                    if (mAddressAllocDialog == null) {
                        mAddressAllocDialog = new CavanMacAddressAddDialog();
                    }

                    mAddressAllocDialog.AddressCountMax = count;
                    if (mAddressAllocDialog.ShowDialog(this) == DialogResult.OK) {
                        mButtonCurrent.alloc(mAddressAllocDialog.AddressCount);
                    }
                }
            }
        }

        private void clearMacAddressItems() {
            panelAddresses.Controls.Clear();
            listViewAddresses.Items.Clear();
        }

        private void allocMacAddress(UInt32 count) {
            clearMacAddressItems();

            CavanMacAddressItem lastItem = null;

            for (UInt32 total = mAddressRange.AddressCount; total > 0; total -= count) {
                if (total < count) {
                    count = total;
                }

                int index;
                Rectangle bounds;
                CavanMacAddress address;

                if (lastItem == null) {
                    index = 0;
                    bounds = panelAddresses.Bounds;
                    bounds.X = bounds.Y = 0;
                    address = new CavanMacAddress(mAddressRange.AddressStart);
                } else {
                    index = lastItem.Index + 1;
                    bounds = lastItem.ButtonBounds;
                    bounds.X += bounds.Width + 1;
                    address = lastItem.AddressNext;
                }

                bounds.Width = (int)(count * panelAddresses.Width / mAddressRange.AddressCount - 1);

                CavanMacAddressItem item = new CavanMacAddressItem(this, bounds, address, count);
                addMacAddressItem(index, item);
                item.AddressUsed = true;
                lastItem = item;
            }

            if (lastItem == null) {
                addMacAddressItemEmpty();
            } else {
                Rectangle bounds = lastItem.ButtonBounds;
                bounds.Width = panelAddresses.Width - bounds.X;
                lastItem.ButtonBounds = bounds;
            }
        }

        public void setAddressStart(CavanMacAddress address) {
            if (mAddressRange.AddressStart.compareTo(address) == 0) {
                return;
            }

            mAddressRange.AddressStart = address;

            foreach (CavanMacAddressItem item in listViewAddresses.Items) {
                item.AddressStart = address;
                address = item.AddressNext;
            }
        }

        public void setAddressCount(UInt32 count) {
            if (mAddressRange.AddressCount == count) {
                return;
            }

            mAddressRange.AddressCount = count;

            if (count > 0) {
                UInt32 total = 0;
                CavanMacAddressItem lastItem = null;

                foreach (CavanMacAddressItem item in listViewAddresses.Items) {
                    if (total < count) {
                        CavanMacAddressButton button = item.AddressButton;

                        if (total + item.AddressCount > count) {
                            item.AddressCount = count - total;
                            total = count;
                        } else {
                            total += item.AddressCount;
                        }

                        int width = (int)(panelAddresses.Width * item.AddressCount / count - 1);

                        if (lastItem == null) {
                            button.Width = width;
                        } else {
                            Rectangle bounds = lastItem.AddressButton.Bounds;
                            bounds.X += bounds.Width + 1;
                            bounds.Width = width;
                            button.Bounds = bounds;
                        }

                        lastItem = item;
                    } else {
                        removeMacAddressItem(item);
                    }
                }

                if (total < count) {
                    if (lastItem == null) {
                        addMacAddressItemEmpty();
                    } else if (lastItem.AddressUsed) {
                        Rectangle bounds = lastItem.ButtonBounds;
                        bounds.X += bounds.Width + 1;
                        bounds.Width = panelAddresses.Width - bounds.X;

                        CavanMacAddressItem item = new CavanMacAddressItem(this, bounds, lastItem.AddressNext, count - total);
                        addMacAddressItem(lastItem.Index + 1, item);
                        lastItem = null;
                    } else {
                        lastItem.AddressCount += count - total;
                    }
                }

                if (lastItem != null) {
                    Rectangle bounds = lastItem.ButtonBounds;
                    bounds.Width = panelAddresses.Width - bounds.X;
                    lastItem.ButtonBounds = bounds;
                }
            }
        }

        public CavanMacAddress AddressStart {
            get {
                return new CavanMacAddress().fromString(textBoxAddressStart.Text);
            }

            set {
                textBoxAddressStart.Text = value.ToString();
            }
        }

        public UInt32 AddressCount {
            get {
                try {
                    return Convert.ToUInt32(textBoxAddressCount.Text);
                } catch {
                    return 0;
                }
            }

            set {
                textBoxAddressCount.Text = Convert.ToString(value);
            }
        }

        public CavanMacAddress AddressEnd {
            get {
                return new CavanMacAddress().fromString(textBoxAddressEnd.Text);
            }

            set {
                textBoxAddressEnd.Text = value.ToString();
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return new CavanMacAddress().fromString(textBoxAddressNext.Text);
            }

            set {
                textBoxAddressNext.Text = value.ToString();
            }
        }

        private void textBoxAddressStart_TextChanged(object sender, EventArgs e) {
            if (acquireEventLock(sender)) {
                UInt32 count = AddressCount;
                CavanMacAddress address = AddressStart;

                setAddressStart(new CavanMacAddress(address));
                AddressEnd = address.getAddressEnd(count);
                AddressNext = address.getAddressNext(count);
            }
        }

        private void textBoxAddressCount_TextChanged(object sender, EventArgs e) {
            textBoxAddressStart_TextChanged(sender, e);
        }

        private void textBoxAddressEnd_TextChanged(object sender, EventArgs e) {
            if (acquireEventLock(sender)) {
                CavanMacAddress address = AddressEnd;

                address.increase();
                AddressNext = address;

                if (address.sub(AddressStart) == 0) {
                    AddressCount = (UInt32)address.toLong();
                } else {
                    AddressCount = 0;
                }
            }
        }

        private void textBoxAddressNext_TextChanged(object sender, EventArgs e) {
            if (acquireEventLock(sender)) {
                CavanMacAddress address = AddressNext;
                CavanMacAddress end = new CavanMacAddress(address);

                end.decrease();
                AddressEnd = end;

                if (address.sub(AddressStart) == 0) {
                    AddressCount = (UInt32)address.toLong();
                } else {
                    AddressCount = 0;
                }
            }
        }

        private void textBoxAddressStart_Leave(object sender, EventArgs e) {
            CavanMacAddress address = AddressStart;
            AddressStart = address;
            setAddressStart(address);
        }

        private void textBoxAddressEnd_Leave(object sender, EventArgs e) {
            AddressEnd = AddressEnd;
            setAddressCount(AddressCount);
        }

        private void textBoxAddressCount_Leave(object sender, EventArgs e) {
            UInt32 count = AddressCount;
            AddressCount = count;
            setAddressCount(count);
        }

        private void textBoxAddressNext_Leave(object sender, EventArgs e) {
            AddressNext = AddressNext;
            setAddressCount(AddressCount);
        }

        private void toolStripMenuItemFree_Click(object sender, EventArgs e) {
            freeMacAddress();
        }

        private void toolStripMenuItemAlloc_Click(object sender, EventArgs e) {
            allocMacAddress();
        }

        public bool acquireEventLock(object sender) {
            if (mSenderLocked != sender) {
                if (mSenderLocked != null) {
                    return false;
                }

                mSenderLocked = sender;
            }

            timerEventLock.Stop();
            timerEventLock.Start();

            return true;
        }

        private void timerUpdateLock_Tick(object sender, EventArgs e) {
            timerEventLock.Stop();
            mSenderLocked = null;
        }

        private void buttonAllocAvg_Click(object sender, EventArgs e) {
            CavanMacAddressAllocAvgDialog dialog = new CavanMacAddressAllocAvgDialog(mAddressRange.AddressCount);
            if (dialog.ShowDialog() == DialogResult.OK) {
                allocMacAddress((mAddressRange.AddressCount + dialog.Count - 1) / dialog.Count);
            }
        }

        private void buttonAllocFixSize_Click(object sender, EventArgs e) {
            CavanMacAddressAllocFixSizeDialog dialog = new CavanMacAddressAllocFixSizeDialog(mAddressRange.AddressCount);
            if (dialog.ShowDialog() == DialogResult.OK) {
                allocMacAddress(dialog.Count);
            }
        }

        private void buttonClear_Click(object sender, EventArgs e) {
            clearMacAddressItems();
            addMacAddressItemEmpty();
        }

        private void buttonExport_Click(object sender, EventArgs e) {
            if (saveFileDialogExport.ShowDialog() == DialogResult.OK) {
                StreamWriter writer = File.CreateText(saveFileDialogExport.FileName);
                writer.WriteLine("起始地址：" + AddressStart);
                writer.WriteLine("结束地址：" + AddressEnd);
                writer.WriteLine("地址个数：" + AddressCount);
                writer.WriteLine();

                foreach (CavanMacAddressItem item in listViewAddresses.Items) {
                    writer.WriteLine(item);
                }

                writer.Close();

                Process.Start("NotePad.exe", saveFileDialogExport.FileName);
            }
        }

        private void listViewAddresses_Leave(object sender, EventArgs e) {
            setCurrentMacAddressButton(null);
        }
    }

    public class CavanMacAddressButton : Button {

        private bool mAddressHighLight;
        private CavanMacAddressItem mAddressItem;

        public CavanMacAddressButton(CavanMacAddressItem item, Rectangle bounds) {
            mAddressItem = item;
            Bounds = bounds;

            FlatStyle = FlatStyle.Flat;
            FlatAppearance.BorderColor = Color.Black;
        }

        public CavanMacAddressItem AddressItem {
            get {
                return mAddressItem;
            }
        }

        public CavanMacAddressManager AddressManager {
            get {
                return mAddressItem.AddressManager;
            }
        }

        public UInt32 AddressCount {
            get {
                return mAddressItem.AddressCount;
            }

            set {
                mAddressItem.AddressCount = value;
            }
        }

        public bool AddressUsed {
            get {
                return mAddressItem.AddressUsed;
            }

            set {
                mAddressItem.AddressUsed = value;
            }
        }

        public CavanMacAddress AddressEnd {
            get {
                return mAddressItem.AddressEnd;
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return mAddressItem.AddressNext;
            }
        }

        public void setHighLight(bool enable) {
            mAddressHighLight = enable;
            updateColor();

            mAddressItem.setHighLight(enable);
        }

        public void updateColor() {
            if (mAddressHighLight) {
                BackColor = Color.Blue;
                FlatAppearance.BorderColor = Color.Red;
            } else if (AddressUsed) {
                BackColor = Color.Green;
                FlatAppearance.BorderColor = Color.Black;
            } else {
                BackColor = Color.White;
                FlatAppearance.BorderColor = Color.Black;
            }

            if (AddressUsed) {
                ForeColor = Color.White;
            } else {
                ForeColor = Color.Red;
            }
        }

        public void updateUI() {
            UInt32 count = AddressCount;

            if (count == 0) {
                ContextMenuStrip = null;
                Text = "无可用地址";
            } else if (AddressUsed) {
                ContextMenuStrip = AddressManager.getContextMenuStripFree();
                Text = "已分配\n(" + count + ")";
            } else {
                ContextMenuStrip = AddressManager.getContextMenuStripAlloc();
                Text = "空闲\n(" + count + ")";
            }

            updateColor();
        }

        public bool alloc(UInt32 count) {
            return mAddressItem.alloc(count);
        }

        public void free() {
            mAddressItem.free();
        }
    }

    public class CavanMacAddressItem : ListViewItem {

        private bool mAddressUsed;
        private CavanMacAddressRange mAddressRange;

        private CavanMacAddressManager mAddressManager;
        private CavanMacAddressButton mAddressButton;

        public CavanMacAddressItem(CavanMacAddressManager manager, Rectangle bounds, CavanMacAddress address, UInt32 count) {
            mAddressManager = manager;
            mAddressUsed = false;

            mAddressRange = new CavanMacAddressRange(address, count);
            mAddressButton = new CavanMacAddressButton(this, bounds);

            SubItems.Add("-");
            SubItems.Add("-");
            SubItems.Add("-");
            updateUI();
        }

        public CavanMacAddress AddressStart {
            get {
                return mAddressRange.AddressStart;
            }

            set {
                mAddressRange.AddressStart = value;
                updateUI();
            }
        }

        public UInt32 AddressCount {
            get {
                return mAddressRange.AddressCount;
            }

            set {
                mAddressRange.AddressCount = value;
                updateUI();
            }
        }

        public bool AddressUsed {
            get {
                return mAddressUsed;
            }

            set {
                mAddressUsed = value;
                updateUI();
            }
        }

        public String AddressUsedText {
            get {
                if (AddressUsed) {
                    return "已分配";
                } else {
                    return "空闲";
                }
            }
        }

        public CavanMacAddress AddressEnd {
            get {
                return mAddressRange.AddressEnd;
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return mAddressRange.AddressNext;
            }
        }

        public CavanMacAddressManager AddressManager {
            get {
                return mAddressManager;
            }
        }

        public CavanMacAddressButton AddressButton {
            get {
                return mAddressButton;
            }
        }

        public Rectangle ButtonBounds {
            get {
                return mAddressButton.Bounds;
            }

            set {
                mAddressButton.Bounds = value;
            }
        }

        public int ButtonWidth {
            get {
                return mAddressButton.Width;
            }

            set {
                mAddressButton.Width = value;
            }
        }

        public void updateUI() {
            Text = AddressStart.ToString();
            SubItems[1].Text = AddressEnd.ToString();
            SubItems[2].Text = Convert.ToString(AddressCount);
            SubItems[3].Text = AddressUsedText;

            mAddressButton.updateUI();
        }

        public bool alloc(UInt32 count) {
            if (mAddressUsed || count == 0) {
                return false;
            }

            UInt32 max = mAddressRange.AddressCount;
            if (count > max) {
                return false;
            }

            if (count < max) {
                Rectangle bounds = mAddressButton.Bounds;
                UInt32 remain = max - count;
                int widthNew = (int)(remain * bounds.Width / max);
                int widthOld = bounds.Width - widthNew;

                bounds.Width = widthOld - 1;
                mAddressButton.Bounds = bounds;
                mAddressRange.AddressCount = count;

                bounds.X += widthOld;
                bounds.Width = widthNew;

                CavanMacAddressItem item = new CavanMacAddressItem(mAddressManager, bounds, AddressNext, remain);
                mAddressManager.addMacAddressItem(Index + 1, item);
            }

            AddressUsed = true;
            mAddressManager.updateContextMenuStrip();

            return true;
        }

        public void free() {
            AddressUsed = false;
            mAddressManager.joinMacAddressItems();
            mAddressManager.updateContextMenuStrip();
        }

        public void setHighLight(bool enable) {
            if (enable) {
                BackColor = Color.Blue;
                ForeColor = Color.White;
            } else {
                BackColor = Color.White;
                ForeColor = Color.Black;
            }
        }

        public override string ToString() {
            StringBuilder builder = new StringBuilder();

            builder.Append(AddressStart).Append(' ').Append(AddressCount).Append(" - ");
            builder.Append(AddressEnd).Append(' ').Append(AddressUsedText);

            return builder.ToString(); ;
        }
    }
}
