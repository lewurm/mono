/**
 * \file
 * Copyright 2018 Microsoft
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#ifndef __MONO_SGEN_MONO_H__
#define __MONO_SGEN_MONO_H__

#define MONO_SGEN_MONO_CALLBACKS_VERSION 1

typedef struct {
	int version;
	void (*emit_nursery_check) (MonoMethodBuilder *mb);
} MonoSgenMonoCallbacks;

void
mono_install_sgen_mono_callbacks (MonoSgenMonoCallbacks *cb);

#endif
