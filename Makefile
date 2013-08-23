##############################################
# OpenWrt Makefile for Doodle3D USB Connection tester package
##############################################
include $(TOPDIR)/rules.mk

PKG_NAME := usbconnectiontester
PKG_VERSION := 0.1.0
PKG_RELEASE := 1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/usbconnectiontester
	SECTION:=mods
	CATEGORY:=Doodle3D
	TITLE:=USB Connection tester package
endef
define Package/usbconnectiontester/description
	USB Connection tester package
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/usbconnectiontester/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usbconnectiontester $(1)/bin/

endef

$(eval $(call BuildPackage,usbconnectiontester))
