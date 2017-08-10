---
title: Out-of-tree modules
permalink: /docs/out-of-tree/
---

The OSS-7 buildsystem allows to compile modules which are not located in the main source tree. This can be useful if this part contains private IP or if this makes no sense to open source it. The system allows this for:
- platforms
- chips
- applications

For instance, let's say you have an internal platform which you cannot add to `stack/framework/hal/platforms` and push upstream. Setting the `PLATFORM_EXTRA_PLATFORMS_DIR` cmake variable to a directory which contains your custom platform(s) allows you to keep your proprietary platforms in a directory out of the OSS-7 source tree. In this way the platform's code can be managed (versioned controled) separately from OSS-7, and the OSS-7 source tree remains clean and in sync with upstream. Likewise, you can do the same for chips and applications using the `CHIP_EXTRA_CHIPS_DIR` and `APP_EXTRA_APPS_DIR` cmake variables respectively.
