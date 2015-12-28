#HLSDK server Android port
#Copyright (c) nicknekit

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := server

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libserver_hardfp
endif

LOCAL_CFLAGS += -D_LINUX -DCLIENT_WEAPONS -D_DEBUG \
	-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf \
	-fno-exceptions -fpermissive -Wno-write-strings -Wno-invalid-offsetof

LOCAL_CPPFLAGS := $(LOCAL_CFLAGS) -frtti

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
		    $(LOCAL_PATH)/. \
		    $(LOCAL_PATH)/wpn_shared \
		    $(LOCAL_PATH)/../common \
		    $(LOCAL_PATH)/../engine/common \
		    $(LOCAL_PATH)/../engine \
		    $(LOCAL_PATH)/../public \
		    $(LOCAL_PATH)/../pm_shared \
		    $(LOCAL_PATH)/../game_shared

LOCAL_SRC_FILES := airtank.cpp \
           animating.cpp \
           animation.cpp \
           armoury_entity.cpp \
           bmodels.cpp \
           buttons.cpp \
           cbase.cpp \
           client.cpp \
           combat.cpp \
           deadplayer_entity.cpp \
           defaultai.cpp \
           destroy_gamerules.cpp \
           doors.cpp \
           effects.cpp \
           explode.cpp \
           flyingmonster.cpp \
           func_break.cpp \
           func_headq.cpp \
           func_tank.cpp \
           game.cpp \
           gamerules.cpp \
           ggrenade.cpp \
           globals.cpp \
           headquarters_gamerules.cpp \
           healthkit.cpp \
           h_ai.cpp \
           h_cine.cpp \
           h_cycler.cpp \
           h_export.cpp \
           items.cpp \
           lights.cpp \
           maprules.cpp \
           monstermaker.cpp \
           monsters.cpp \
           monsterstate.cpp \
           mortar.cpp \
           multiplay_gamerules.cpp \
           nodes.cpp \
           observer.cpp \
           osprey.cpp \
           pathcorner.cpp \
           plane.cpp \
           plats.cpp \
           player.cpp \
           rat.cpp \
           roach.cpp \
           schedule.cpp \
           scripted.cpp \
           sound.cpp \
           soundent.cpp \
           spectator.cpp \
           squadmonster.cpp \
           subs.cpp \
           talkmonster.cpp \
           teamplay_gamerules.cpp \
           team_main.cpp \
           tentacle.cpp \
           triggers.cpp \
           util.cpp \
           weapons.cpp \
           world.cpp \
           wpn_ak47.cpp \
           wpn_aug.cpp \
           wpn_awp.cpp \
           wpn_c4.cpp \
           wpn_deagle.cpp \
           wpn_famas.cpp \
           wpn_flashbang.cpp \
           wpn_galil.cpp \
           wpn_glock.cpp \
           wpn_hegrenade.cpp \
           wpn_knife.cpp \
           wpn_m3.cpp \
           wpn_m4a1.cpp \
           wpn_mp5.cpp \
           wpn_sg552.cpp \
           wpn_smokegrenade.cpp \
           wpn_usp.cpp \
           xen.cpp \
	   ../pm_shared/pm_debug.c \
	   ../pm_shared/pm_math.c \
	   ../pm_shared/pm_shared.c \
	   ../game_shared/voice_gamemgr.cpp

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
