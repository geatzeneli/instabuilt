// InstaBuiltVersion.h — Version tracking
// ARCHITECTURE.md Section 11.8: Versioning
// PIPELINE.md Section 8: Patching tiers

#pragma once

#define INSTABUILT_VERSION_MAJOR 0
#define INSTABUILT_VERSION_MINOR 1
#define INSTABUILT_VERSION_PATCH 0
#define INSTABUILT_VERSION_BUILD 1

#define INSTABUILT_VERSION_STRING "0.1.0.1"
#define INSTABUILT_VERSION_TEXT TEXT("0.1.0-prototype")
#define INSTABUILT_VERSION TEXT(INSTABUILT_VERSION_STRING)

/** Save file schema version. Increment when save format changes. */
#define INSTABUILT_SAVE_VERSION 1

/** Minimum compatible save version. Reject saves older than this. */
#define INSTABUILT_MIN_SAVE_VERSION 1
