#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := app

include $(IDF_PATH)/make/project.mk

# deploy file to esp-ota-server
deploy: all
	cp build/$(PROJECT_NAME).bin /srv/esp-ota-server/data
