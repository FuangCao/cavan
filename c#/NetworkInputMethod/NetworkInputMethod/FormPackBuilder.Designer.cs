namespace NetworkInputMethod
{
    partial class FormPackBuilder
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormPackBuilder));
            this.buttonAccount = new System.Windows.Forms.Button();
            this.buttonSave = new System.Windows.Forms.Button();
            this.treeView = new System.Windows.Forms.TreeView();
            this.imageList = new System.Windows.Forms.ImageList(this.components);
            this.buttonFile = new System.Windows.Forms.Button();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.buttonServer = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // buttonAccount
            // 
            this.buttonAccount.Location = new System.Drawing.Point(174, 472);
            this.buttonAccount.Name = "buttonAccount";
            this.buttonAccount.Size = new System.Drawing.Size(75, 23);
            this.buttonAccount.TabIndex = 0;
            this.buttonAccount.Text = "账号";
            this.buttonAccount.UseVisualStyleBackColor = true;
            this.buttonAccount.Click += new System.EventHandler(this.buttonAccount_Click);
            // 
            // buttonSave
            // 
            this.buttonSave.Location = new System.Drawing.Point(557, 472);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 1;
            this.buttonSave.Text = "保存";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // treeView
            // 
            this.treeView.FullRowSelect = true;
            this.treeView.HideSelection = false;
            this.treeView.ImageIndex = 0;
            this.treeView.ImageList = this.imageList;
            this.treeView.Location = new System.Drawing.Point(12, 12);
            this.treeView.Name = "treeView";
            this.treeView.SelectedImageIndex = 4;
            this.treeView.ShowLines = false;
            this.treeView.Size = new System.Drawing.Size(620, 454);
            this.treeView.TabIndex = 2;
            // 
            // imageList
            // 
            this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
            this.imageList.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList.Images.SetKeyName(0, "server");
            this.imageList.Images.SetKeyName(1, "file");
            this.imageList.Images.SetKeyName(2, "account");
            this.imageList.Images.SetKeyName(3, "item");
            this.imageList.Images.SetKeyName(4, "selected");
            // 
            // buttonFile
            // 
            this.buttonFile.Location = new System.Drawing.Point(93, 472);
            this.buttonFile.Name = "buttonFile";
            this.buttonFile.Size = new System.Drawing.Size(75, 23);
            this.buttonFile.TabIndex = 3;
            this.buttonFile.Text = "文件";
            this.buttonFile.UseVisualStyleBackColor = true;
            this.buttonFile.Click += new System.EventHandler(this.buttonFile_Click);
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "文本文件|game*.txt|所以文件|*.*";
            this.openFileDialog.Multiselect = true;
            // 
            // buttonServer
            // 
            this.buttonServer.Location = new System.Drawing.Point(12, 472);
            this.buttonServer.Name = "buttonServer";
            this.buttonServer.Size = new System.Drawing.Size(75, 23);
            this.buttonServer.TabIndex = 4;
            this.buttonServer.Text = "服务器";
            this.buttonServer.UseVisualStyleBackColor = true;
            this.buttonServer.Click += new System.EventHandler(this.buttonServer_Click);
            // 
            // FormPackBuilder
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(644, 507);
            this.Controls.Add(this.buttonServer);
            this.Controls.Add(this.buttonFile);
            this.Controls.Add(this.treeView);
            this.Controls.Add(this.buttonSave);
            this.Controls.Add(this.buttonAccount);
            this.Name = "FormPackBuilder";
            this.Text = "FormPackBuilder";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonAccount;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.TreeView treeView;
        private System.Windows.Forms.Button buttonFile;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.ImageList imageList;
        private System.Windows.Forms.Button buttonServer;
    }
}