MODULE := backends/platform/3ds

MODULE_OBJS := \
	main.o \
	allocHeapsOverride.o \
	shader.shbin.o \
	sprite.o \
	options.o \
	osystem.o \
	osystem-audio.o \
	osystem-events.o
#TEMPLATE:
#ifdef ENABLE_[ENGINE UPPERCASE]
#MODULE_OBJS += \
#	shaders/[ENGINE LOWERCASE]/[ENGINE LOWERCASE]_[SHADER SUBNAME].shbin.o \
#	shaders/[ENGINE LOWERCASE]/[ENGINE LOWERCASE]_[SHADER SUBNAME].shbin.o \
#	...
#endif
ifdef ENABLE_PLAYGROUND3D
MODULE_OBJS += \
	shaders/playground3d/playground3d_cube.shbin.o \
	shaders/playground3d/playground3d_offset.shbin.o \
	shaders/playground3d/playground3d_fade.shbin.o \
	shaders/playground3d/playground3d_bitmap.shbin.o \
	shaders/playground3d/playground3d_viewport.shbin.o
endif
MODULE_OBJS += \
	osystem-graphics.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
