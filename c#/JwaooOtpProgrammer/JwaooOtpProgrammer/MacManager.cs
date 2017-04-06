using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public partial class MacManager : Form {

        private int mMacButtonCount;
        private MacAddressButton mCurrMacButton;
        private LinkedList<MacAddressButton> mMacButtonList = new LinkedList<MacAddressButton>();

        public MacManager() {
            InitializeComponent();

            MacAddressButton button = new MacAddressButton(this, 1000);
            button.Location = new Point(0, 0);
            button.Size = panelAddresses.Size;
            addMacAddressButton(button);
        }

        public ContextMenuStrip getContextMenuStripAlloc() {
            return contextMenuStripAlloc;
        }

        public ContextMenuStrip getContextMenuStripFree() {
            return contextMenuStripFree;
        }

        public void setFocusMacAddressButton(MacAddressButton button) {
            if (mCurrMacButton != null) {
                mCurrMacButton.BackColor = Color.White;
                mCurrMacButton.ForeColor = Color.Black;
                mCurrMacButton.FlatAppearance.BorderColor = Color.Black;
            }

            mCurrMacButton = button;
            mCurrMacButton.BackColor = Color.Blue;
            mCurrMacButton.ForeColor = Color.White;
            mCurrMacButton.FlatAppearance.BorderColor = Color.Red;

            listViewAddresses.Items[button.Index].Selected = true;
        }

        public void addMacAddressButton(MacAddressButton button) {
            button.GotFocus += buttonMacAddress_GotFocus;
            button.MouseEnter += buttonMacAddress_MouseEnter;
            panelAddresses.Controls.Add(button);

            button.Index = mMacButtonCount++;

            ListViewItem item = new ListViewItem("11:22:33:44:55:66");
            item.SubItems.Add("11:22:33:44:55:66");
            item.SubItems.Add("100");
            item.SubItems.Add("已分配");
            listViewAddresses.Items.Add(item);
        }

        private void buttonMacAddress_MouseEnter(object sender, EventArgs e) {
            MacAddressButton button = (MacAddressButton)sender;
            button.Focus();
        }

        private void buttonMacAddress_GotFocus(object sender, EventArgs e) {
            setFocusMacAddressButton((MacAddressButton) sender);
        }

        private void freeMacAddress() {
            if (mCurrMacButton != null) {
                mCurrMacButton.free();
            }
        }

        private void allocMacAddress() {
            if (mCurrMacButton != null) {
                mCurrMacButton.alloc(100);
            }
        }

        private int getAddressCount() {
            try {
                return int.Parse(textBoxAddressCount.Text);
            } catch {
                return 0;
            }
        }

        private void buttonAdd_Click(object sender, EventArgs e) {
            /* Rectangle bounds = buttonAddressFree.Bounds;
            int width = bounds.Width / 4;

            Button button = new Button();
            button.FlatStyle = FlatStyle.Flat;
            button.FlatAppearance.BorderColor = Color.Black;
            bounds.Width = width;
            button.Bounds = bounds;
            buttonAddressFree.Parent.Controls.Add(button);

            bounds = buttonAddressFree.Bounds;
            bounds.Width -= width + 1;
            bounds.X += width + 1;
            buttonAddressFree.Bounds = bounds; */
        }

        private void textBoxAddressStart_TextChanged(object sender, EventArgs e) {

        }

        private void textBoxAddressCount_TextChanged(object sender, EventArgs e) {

        }

        private void textBoxAddressEnd_TextChanged(object sender, EventArgs e) {

        }

        private void toolStripMenuItemFree_Click(object sender, EventArgs e) {
            freeMacAddress();
        }

        private void toolStripMenuItemAlloc_Click(object sender, EventArgs e) {
            allocMacAddress();
        }
    }

    public class MacAddressButton : Button {

        private int mIndex;
        private int mCount;
        private bool mUsed;
        private MacManager mForm;

        public MacAddressButton(MacManager form, int count) {
            mForm = form;
            FlatStyle = FlatStyle.Flat;
            FlatAppearance.BorderColor = Color.Black;

            Count = count;
            Used = false;
        }

        public int Index {
            get {
                return mIndex;
            }

            set {
                mIndex = value;
            }
        }

        public int Count {
            get {
                return mCount;
            }

            set {
                mCount = value;
            }
        }

        public bool Used {
            get {
                return mUsed;
            }

            set {
                mUsed = value;

                if (mUsed) {
                    ContextMenuStrip = mForm.getContextMenuStripFree();
                    Text = "已分配(" + mCount + ")";
                } else {
                    ContextMenuStrip = mForm.getContextMenuStripAlloc();
                    Text = "空闲(" + mCount + ")";
                }
            }
        }

        public bool alloc(int count) {
            if (mCount < count || count <= 0) {
                return false;
            }

            if (count < mCount) {
                Rectangle bounds = Bounds;
                int remain = mCount - count;
                int widthNew = remain * bounds.Width / mCount;
                int widthOld = bounds.Width - widthNew;

                bounds.Width = widthOld - 1;
                Bounds = bounds;
                Count = count;

                MacAddressButton button = new MacAddressButton(mForm, remain);
                bounds.X += widthOld;
                bounds.Width = widthNew;
                button.Bounds = bounds;
                mForm.addMacAddressButton(button);
            }

            Used = true;

            return true;
        }

        public void free() {
            Used = false;
        }
    }
}
