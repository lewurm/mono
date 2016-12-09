#ifndef custom_dl_iterate_phdr_h
#define custom_dl_iterate_phdr_h

int custom_dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info, size_t size, void *data), void *data);

#endif
