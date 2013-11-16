LOCAL_PATH = $(PWD)
OUT_PATH = $(LOCAL_PATH)/out
OUT_BIN = $(OUT_PATH)/bin
OUT_GEN = $(OUT_PATH)/gen
RES_PATH = $(LOCAL_PATH)/res
SRC_PATH = $(LOCAL_PATH)/src

FILE_MANIFEST = $(LOCAL_PATH)/AndroidManifest.xml
FILE_ANDROID_JAR = $(SDK_HOME)/platforms/android-8/android.jar
TARGET_APK = $(OUT_BIN)/cavan.apk
TARGET_UNSIGNED_APK = $(OUT_BIN)/cavan_unsigned.apk
TARGET_DEX = $(OUT_BIN)/classes.dex
TARGET_RES = $(OUT_BIN)/resource
TARGET_KEYSTORE = $(OUT_BIN)/cavan.keystore
KEYSTORE_PASS = CFA8888

PACKAGE_NAME = $(shell cat $(FILE_MANIFEST) | grep package='".*"' | sed s/.*package='"\(.*\)"'/\\1/g)
PACKAGE_PATH = $(subst .,/,$(PACKAGE_NAME))
TARGET_R_JAVA = $(OUT_GEN)/$(PACKAGE_PATH)/R.java

CLASSPATH := $(CLASSPATH):$(OUT_BIN)

AIDL_SOURCE = $(shell find $(SRC_PATH) -name I*.aidl)
AIDL_TARGET = $(patsubst $(SRC_PATH)/%,$(OUT_GEN)/%.java,$(basename $(AIDL_SOURCE)))
JAVA_SOURCE = $(shell find $(SRC_PATH) -name *.java)
JAVA_TARGET = $(patsubst $(SRC_PATH)/%,$(OUT_BIN)/%.class,$(basename $(JAVA_SOURCE)))
JAVA_TARGET := $(patsubst $(OUT_GEN)/%,$(OUT_BIN)/%.class,$(basename $(TARGET_R_JAVA) $(AIDL_TARGET))) $(JAVA_TARGET)
JAVA_SOURCE := $(TARGET_R_JAVA) $(AIDL_TARGET) $(JAVA_SOURCE)

all: $(TARGET_APK)

$(TARGET_APK): $(TARGET_UNSIGNED_APK) $(TARGET_KEYSTORE)
	$(Q)jarsigner -verbose -storepass $(KEYSTORE_PASS) -keystore $(TARGET_KEYSTORE) -signedjar $@ $^

$(TARGET_KEYSTORE):
	$(Q)keytool -genkey -storepass $(KEYSTORE_PASS) -alias $(TARGET_KEYSTORE) -keyalg RSA -keystore $(TARGET_KEYSTORE)

$(TARGET_UNSIGNED_APK): $(TARGET_DEX) | $(OUT_GEN) $(OUT_BIN)
	$(Q)aapt package -fM $(FILE_MANIFEST) -S $(RES_PATH) -I $(FILE_ANDROID_JAR) -F $(TARGET_RES)
	$(Q)apkbuilder $@ -v -u -z $(TARGET_RES) -f $(TARGET_DEX) -rf $(SRC_PATH)

$(OUT_BIN)/%.class: $(SRC_PATH)/%.java | $(OUT_BIN)
	$(Q)javac -encoding GB18030 -target 1.5 -sourcepath $(SRC_PATH) -bootclasspath $(FILE_ANDROID_JAR) -d $(OUT_BIN) $^

$(OUT_BIN)/%.class: $(OUT_GEN)/%.java | $(OUT_BIN)
	$(Q)javac -encoding GB18030 -target 1.5 -sourcepath $(SRC_PATH) -bootclasspath $(FILE_ANDROID_JAR) -d $(OUT_BIN) $^

$(TARGET_DEX): $(JAVA_TARGET)
	$(Q)dx --dex --output=$@ $(@D)

$(TARGET_R_JAVA): $(OUT_GEN)
	$(Q)aapt package -fmJ $(OUT_GEN) -M $(FILE_MANIFEST) -S $(RES_PATH) -I $(FILE_ANDROID_JAR)

$(OUT_GEN)/%.java: $(SRC_PATH)/%.aidl | $(OUT_GEN)
	$(Q)aidl -o$(OUT_GEN) -I$(SRC_PATH) $<

$(OUT_BIN) $(OUT_GEN):
	$(Q)mkdir $@ -pv

.PRECIOUS: $(TARGET_R_JAVA) $(TARGET_DEX) $(TARGET_RES) $(AIDL_TARGET) $(JAVA_TARGET)
