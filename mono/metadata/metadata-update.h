/**
 * \file
 */

#ifndef __MONO_METADATA_UPDATE_H__
#define __MONO_METADATA_UPDATE_H__

#include "mono/utils/mono-forward.h"
#include "mono/metadata/loader-internals.h"
#include "mono/metadata/metadata-internals.h"

void
mono_metadata_update_init (void);

gboolean
mono_metadata_update_available (void);

gboolean
mono_metadata_wait_for_update (uint32_t timeout_ms);

uint32_t
mono_metadata_update_prepare (MonoDomain *domain);

void
mono_metadata_update_publish (MonoDomain *domain, MonoAssemblyLoadContext *alc, uint32_t generation);

void
mono_metadata_update_cancel (uint32_t generation);

void
mono_metadata_update_cleanup_on_close (MonoImage *base_image);

void
mono_dil_file_close (MonoDilFile *dil);

void
mono_dil_file_destroy (MonoDilFile *dil);


#endif /*__MONO_METADATA_UPDATE_H__*/
