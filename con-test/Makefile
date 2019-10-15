include $(TOPDIR)/rules.mk

PKG_NAME:=con-test
PKG_VERSION:=0.2
PKG_RELEASE:=1
PKG_MAINTAINER:=Stefan Venz <stefan.venz@protonmail.com>
PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/con-test
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Wireless connection testing framework
	DEPENDS:=+bash +getopt +tcpdump
	MAINTAINER:=Stefan Venz <stefan.venz@protonmail.com>
	PKGARCH:=all
endef

define Package/con-test/description
	conTest eases the creation of network connection tests for wireless
	algorithm testing
endef

define Build/Compile
endef

define Package/con-test/install
		$(INSTALL_DIR) $(1)/usr/sbin/con-test
		$(INSTALL_BIN) ./con-test $(1)/usr/sbin/con-test/
		$(INSTALL_DATA) ./commons $(1)/usr/sbin/con-test/
		$(INSTALL_DATA) ./wireless_monitor $(1)/usr/sbin/con-test/
		$(INSTALL_DIR) $(1)/etc/config
		$(INSTALL_DATA) ./con-test.conf $(1)/etc/config/
endef

$(eval $(call BuildPackage,con-test))
