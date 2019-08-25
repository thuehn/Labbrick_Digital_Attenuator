include $(TOPDIR)/rules.mk

PKG_NAME:=digital_attenuator
PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_MAINTAINER:=Stefan Venz <stefan.venz@protonmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/digital_attenuator
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+kmod-usb-acm +libusb-1.0 +libusb-compat
	MAINTAINER:=$(PKG_MAINTAINER)
	TITLE:=Control software for labbrick digital attenuator
endef

define Package/digital_attenuator/description
	Control Vaunix programmable attenuator with config files or by command
	line.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ./src/. $(PKG_BUILD_DIR)/.
endef

define Build/Compile
	$(call Build/Compile/Default,all)
endef

define Package/digital_attenuator/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/attenuator_lab_brick $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,digital_attenuator))
